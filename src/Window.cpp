#include "Window.hpp"

#include <stdexcept>
#include <iostream>
#include <cstring>
#include <limits>
#include <algorithm>
#include <fstream>

static std::vector<Window::Vertex> vertices = {
    {{0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}},
    {{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
    {{-0.5f, 0.5f}, {0.0f, 1.0f, 1.0f}}
};

Window::Window() {}

void Window::init() {
    initWindow();
    initVulkan();
}

void Window::initWindow() {
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    window = glfwCreateWindow(1280, 720, "UnnamedMinecraftClone", nullptr, nullptr);

    glfwSetWindowUserPointer(window, this);
    glfwSetFramebufferSizeCallback(window, Window::glfwOnResizeCallback);
}

void Window::initVulkan() {
    createVkInstance();
    createSurface();
    physicalDevice = pickPhysicalDevice();
    createDevice();
    createSwapChain();
    createGraphicsPipeline();
    createFramebuffers();
    createCommandPool();
    createVertexBuffer();
    createCommandBuffer();
    createSyncObjects();
    std::cout << "Initialization complete" << std::endl;
}

void Window::createVkInstance() {
    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "UnnamedMinecraftClone";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;

    auto requiredExtensions = getGlfwRequiredInstanceExtensions();
    auto extensionSupport = getVkInstanceExtensionSupport();
    if (extensionSupport.hasKHRPortabilityEnumeration) {
        std::cout << "Enabling KHR portability enumeration" << std::endl;
        requiredExtensions.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
        createInfo.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
    }

    createInfo.enabledExtensionCount = requiredExtensions.size();
    createInfo.ppEnabledExtensionNames = requiredExtensions.data();

    auto layerSupport = getVkInstanceLayerSupport();
    if (layerSupport.hasKhronos) {
        std::cout << "Enabling KHRONOS validation layer" << std::endl;
        
        static const char* LAYERS[] = {"VK_LAYER_KHRONOS_validation"};
        createInfo.enabledLayerCount = 1;
        createInfo.ppEnabledLayerNames = LAYERS;
    } else {
        std::cout << "Validation layer unsupported!" << std::endl;
        createInfo.enabledLayerCount = 0;
        createInfo.ppEnabledLayerNames = nullptr;
    }

    if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS)
        throw std::runtime_error{"failed to create instance!"};
}

void Window::createSurface() {
    if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS)
        throw std::runtime_error{"failed to create window surface!"};
}

void Window::createDevice() {
    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos{};

    for (auto queueIndex : physicalDevice.queues.indices()) {
        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueIndex;
        queueCreateInfo.queueCount = 1;
        
        static float PRIORITIES[] = {1.0f};
        queueCreateInfo.pQueuePriorities = PRIORITIES;

        queueCreateInfos.push_back(queueCreateInfo);
    }

    VkPhysicalDeviceFeatures deviceFeatures{};
    
    VkDeviceCreateInfo deviceCreateInfo{};
    deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
    deviceCreateInfo.queueCreateInfoCount = queueCreateInfos.size();
    
    static const char* EXTENSIONS[] = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
    deviceCreateInfo.enabledExtensionCount = 1;
    deviceCreateInfo.ppEnabledExtensionNames = EXTENSIONS;
    
    deviceCreateInfo.pEnabledFeatures = &deviceFeatures;

    if (vkCreateDevice(physicalDevice.handle, &deviceCreateInfo, nullptr, &device) != VK_SUCCESS)
        throw std::runtime_error{"failed to create device!"};

    vkGetDeviceQueue(device, physicalDevice.queues.graphics.value(), 0, &graphicsQueue);
    vkGetDeviceQueue(device, physicalDevice.queues.present.value(), 0, &presentQueue);
}

void Window::createSwapChain() {
    VkSurfaceCapabilitiesKHR capabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice.handle, surface, &capabilities);

    uint32_t minImageCount = capabilities.minImageCount + 1;
    if (capabilities.maxImageCount > 0 && minImageCount > capabilities.maxImageCount)
        minImageCount = capabilities.maxImageCount;

    VkSwapchainCreateInfoKHR swapchainCreateInfo{};
    swapchainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchainCreateInfo.surface = surface;
    swapchainCreateInfo.minImageCount = minImageCount;
    swapchainCreateInfo.imageFormat = physicalDevice.surfaceFormat.format;
    swapchainCreateInfo.imageColorSpace = physicalDevice.surfaceFormat.colorSpace;
    swapchainCreateInfo.imageArrayLayers = 1;
    swapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
        swapchainCreateInfo.imageExtent = capabilities.currentExtent;
    } else {
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);

        swapchainCreateInfo.imageExtent.width = std::clamp(
            static_cast<uint32_t>(width),
            capabilities.minImageExtent.width,
            capabilities.maxImageExtent.width
        );
        swapchainCreateInfo.imageExtent.height = std::clamp(
            static_cast<uint32_t>(height),
            capabilities.minImageExtent.height,
            capabilities.maxImageExtent.height
        );
    }

    std::vector<uint32_t> queueIndices = physicalDevice.queues.indices();

    if (queueIndices.size() > 1) {
        // The swapchain will be shared between submit queues
        swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        swapchainCreateInfo.queueFamilyIndexCount = queueIndices.size();
        swapchainCreateInfo.pQueueFamilyIndices = queueIndices.data();
    } else {
        // Present and graphics are the same submit queue, enable exclusive mode
        swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        swapchainCreateInfo.queueFamilyIndexCount = 0;
        swapchainCreateInfo.pQueueFamilyIndices = nullptr;
    }

    swapchainCreateInfo.preTransform = capabilities.currentTransform;
    swapchainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapchainCreateInfo.presentMode = physicalDevice.presentMode;
    swapchainCreateInfo.clipped = VK_TRUE;
    swapchainCreateInfo.oldSwapchain = VK_NULL_HANDLE;

    if (vkCreateSwapchainKHR(device, &swapchainCreateInfo, nullptr, &swapchain) != VK_SUCCESS)
        throw std::runtime_error{"failed to create swapchain!"};

    uint32_t imageCount = 0;
    vkGetSwapchainImagesKHR(device, swapchain, &imageCount, nullptr);

    swapchainImages.resize(imageCount, VK_NULL_HANDLE);
    vkGetSwapchainImagesKHR(device, swapchain, &imageCount, swapchainImages.data());

    swapchainExtent = swapchainCreateInfo.imageExtent;
    swapchainFormat = swapchainCreateInfo.imageFormat;

    swapchainImageViews.resize(imageCount, VK_NULL_HANDLE);
    for (uint32_t i = 0; i < imageCount; i++) {
        VkImageViewCreateInfo imageCreateInfo{};
        imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        imageCreateInfo.image = swapchainImages[i];
        imageCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        imageCreateInfo.format = swapchainFormat;
        imageCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imageCreateInfo.subresourceRange.baseMipLevel = 0;
        imageCreateInfo.subresourceRange.levelCount = 1;
        imageCreateInfo.subresourceRange.baseArrayLayer = 0;
        imageCreateInfo.subresourceRange.layerCount = 1;

        if (vkCreateImageView(device, &imageCreateInfo, nullptr, &swapchainImageViews[i]) != VK_SUCCESS)
            throw std::runtime_error{"failed to create swapchain image view"};
    }
}

void Window::createGraphicsPipeline() {
    VkAttachmentDescription colorAttachment{};
    colorAttachment.format = swapchainFormat;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;

    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    VkRenderPassCreateInfo renderPassCreateInfo{};
    renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassCreateInfo.attachmentCount = 1;
    renderPassCreateInfo.pAttachments = &colorAttachment;
    renderPassCreateInfo.subpassCount = 1;
    renderPassCreateInfo.pSubpasses = &subpass;
    renderPassCreateInfo.dependencyCount = 1;
    renderPassCreateInfo.pDependencies = &dependency;

    if (vkCreateRenderPass(device, &renderPassCreateInfo, nullptr, &renderPass) != VK_SUCCESS)
        throw std::runtime_error{"failed to create render pass!"};

    VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{};
    pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutCreateInfo.setLayoutCount = 0;
    pipelineLayoutCreateInfo.pSetLayouts = nullptr;
    pipelineLayoutCreateInfo.pushConstantRangeCount = 0;
    pipelineLayoutCreateInfo.pPushConstantRanges = nullptr;

    if (vkCreatePipelineLayout(device, &pipelineLayoutCreateInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
        throw std::runtime_error{"failed to create pipeline layout!"};

    auto vertShaderModule = loadShaderModule("SimpleVert.vert.spv");
    auto fragShaderModule = loadShaderModule("SimpleFrag.frag.spv");

    VkPipelineShaderStageCreateInfo vertStageInfo{};
    vertStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertStageInfo.module = vertShaderModule;
    vertStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo fragStageInfo{};
    fragStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragStageInfo.module = fragShaderModule;
    fragStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo stageInfos[] = {vertStageInfo, fragStageInfo};

    VkPipelineDynamicStateCreateInfo dynamicStateInfo{};
    dynamicStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;

    static VkDynamicState DYNAMIC_STATES[] = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR
    };
    dynamicStateInfo.dynamicStateCount = 2;
    dynamicStateInfo.pDynamicStates = DYNAMIC_STATES;

    auto bindingDescription = Vertex::getBindingDescription();
    auto attributeDescriptions = Vertex::getAttributeDescriptions();

    VkPipelineVertexInputStateCreateInfo vertexInputStageInfo{};
    vertexInputStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputStageInfo.vertexBindingDescriptionCount = 1;
    vertexInputStageInfo.pVertexBindingDescriptions = &bindingDescription;
    vertexInputStageInfo.vertexAttributeDescriptionCount = attributeDescriptions.size();
    vertexInputStageInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

    VkPipelineInputAssemblyStateCreateInfo inputAssemblyStageInfo{};
    inputAssemblyStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssemblyStageInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssemblyStageInfo.primitiveRestartEnable = VK_FALSE;

    VkPipelineViewportStateCreateInfo viewportStateInfo{};
    viewportStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportStateInfo.viewportCount = 1;
    viewportStateInfo.scissorCount = 1;

    VkPipelineRasterizationStateCreateInfo rasterizerStateInfo{};
    rasterizerStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizerStateInfo.depthClampEnable = VK_FALSE;
    rasterizerStateInfo.rasterizerDiscardEnable = VK_FALSE;
    rasterizerStateInfo.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizerStateInfo.lineWidth = 1.0f;
    rasterizerStateInfo.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizerStateInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rasterizerStateInfo.depthBiasEnable = VK_FALSE;
    rasterizerStateInfo.depthBiasConstantFactor = 0.0f;
    rasterizerStateInfo.depthBiasClamp = 0.0f;
    rasterizerStateInfo.depthBiasSlopeFactor = 0.0f;

    VkPipelineMultisampleStateCreateInfo multisamplingStateInfo{};
    multisamplingStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisamplingStateInfo.sampleShadingEnable = VK_FALSE;
    multisamplingStateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisamplingStateInfo.minSampleShading = 1.0f;
    multisamplingStateInfo.pSampleMask = nullptr;
    multisamplingStateInfo.alphaToCoverageEnable = VK_FALSE;
    multisamplingStateInfo.alphaToOneEnable = VK_FALSE;

    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_FALSE;
    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

    VkPipelineColorBlendStateCreateInfo colorBlendStateInfo{};
    colorBlendStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlendStateInfo.logicOpEnable = VK_FALSE;
    colorBlendStateInfo.logicOp = VK_LOGIC_OP_COPY;
    colorBlendStateInfo.attachmentCount = 1;
    colorBlendStateInfo.pAttachments = &colorBlendAttachment;
    colorBlendStateInfo.blendConstants[0] = 0.0f;
    colorBlendStateInfo.blendConstants[1] = 0.0f;
    colorBlendStateInfo.blendConstants[2] = 0.0f;
    colorBlendStateInfo.blendConstants[3] = 0.0f;

    VkGraphicsPipelineCreateInfo pipelineCreateInfo{};
    pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineCreateInfo.stageCount = 2;
    pipelineCreateInfo.pStages = stageInfos;
    pipelineCreateInfo.pVertexInputState = &vertexInputStageInfo;
    pipelineCreateInfo.pInputAssemblyState = &inputAssemblyStageInfo;
    pipelineCreateInfo.pViewportState = &viewportStateInfo;
    pipelineCreateInfo.pRasterizationState = &rasterizerStateInfo;
    pipelineCreateInfo.pMultisampleState = &multisamplingStateInfo;
    pipelineCreateInfo.pDepthStencilState = nullptr;
    pipelineCreateInfo.pColorBlendState = &colorBlendStateInfo;
    pipelineCreateInfo.pDynamicState = &dynamicStateInfo;
    pipelineCreateInfo.layout = pipelineLayout;
    pipelineCreateInfo.renderPass = renderPass;
    pipelineCreateInfo.subpass = 0;
    pipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;
    pipelineCreateInfo.basePipelineIndex = -1;

    if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineCreateInfo, nullptr, &pipeline) != VK_SUCCESS)
        throw std::runtime_error{"failed to create graphics pipeline"};

    vkDestroyShaderModule(device, vertShaderModule, nullptr);
    vkDestroyShaderModule(device, fragShaderModule, nullptr);
}

void Window::createFramebuffers() {
    uint32_t imageCount = swapchainImages.size();

    swapchainFramebuffers.resize(imageCount, VK_NULL_HANDLE);
    for (uint32_t i = 0; i < imageCount; i++) {
        VkFramebufferCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        createInfo.renderPass = renderPass;
        createInfo.attachmentCount = 1;
        createInfo.pAttachments = &swapchainImageViews[i];
        createInfo.width = swapchainExtent.width;
        createInfo.height = swapchainExtent.height;
        createInfo.layers = 1;

        if (vkCreateFramebuffer(device, &createInfo, nullptr, &swapchainFramebuffers[i]) != VK_SUCCESS)
            throw std::runtime_error{"failed to create framebuffer!"};
    }
}

void Window::createCommandPool() {
    VkCommandPoolCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    createInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    createInfo.queueFamilyIndex = physicalDevice.queues.graphics.value();

    if (vkCreateCommandPool(device, &createInfo, nullptr, &commandPool) != VK_SUCCESS)
        throw std::runtime_error{"failed to create command pool!"};
}

void Window::createVertexBuffer() {
    VkDeviceSize bufferSize = sizeof(Vertex) * vertices.size();
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    createBuffer(
        bufferSize,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        stagingBuffer,
        stagingBufferMemory);

    void *data;
    vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);

    memcpy(data, vertices.data(), bufferSize);

    vkUnmapMemory(device, stagingBufferMemory);
    createBuffer(
        bufferSize,
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        vertexBuffer,
        vertexBufferMemory);
    copyBuffer(stagingBuffer, vertexBuffer, bufferSize);
    vkDestroyBuffer(device, stagingBuffer, nullptr);
    vkFreeMemory(device, stagingBufferMemory, nullptr);
}

void Window::createCommandBuffer() {
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = MAX_FRAMES_IN_FLIGHT;

    commandBuffers.resize(MAX_FRAMES_IN_FLIGHT, VK_NULL_HANDLE);
    if (vkAllocateCommandBuffers(device, &allocInfo, commandBuffers.data()) != VK_SUCCESS)
        throw std::runtime_error{"failed to create command buffer!"};
}

void Window::createSyncObjects() {
    VkSemaphoreCreateInfo semaphoreCreateInfo{};
    semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceCreateInfo{};
    fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT, VK_NULL_HANDLE);
    for(uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
        if (vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS)
            throw std::runtime_error{"failed to create sync objects!"};

    renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT, VK_NULL_HANDLE);
    for(uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
        if (vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS)
            throw std::runtime_error{"failed to create sync objects!"};

    inFlightFences.resize(MAX_FRAMES_IN_FLIGHT, VK_NULL_HANDLE);
    for(uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
        if (vkCreateFence(device, &fenceCreateInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS)
            throw std::runtime_error{"failed to create sync objects!"};
}

void Window::drawFrame() {
    vkWaitForFences(device, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

    uint32_t imageIndex;
    VkResult result = vkAcquireNextImageKHR(device, swapchain, UINT64_MAX, imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);

    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        recreateSwapchain();
        return;
    } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        throw std::runtime_error{"failed to acquire swapchain image!"};
    }

    vkResetFences(device, 1, &inFlightFences[currentFrame]);

    vkResetCommandBuffer(commandBuffers[currentFrame], 0);
    recordCommandBuffer(commandBuffers[currentFrame], imageIndex);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkPipelineStageFlags WAIT_STAGES[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = &imageAvailableSemaphores[currentFrame];
    submitInfo.pWaitDstStageMask = WAIT_STAGES;

    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffers[currentFrame];

    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = &renderFinishedSemaphores[currentFrame];

    if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFences[currentFrame]) != VK_SUCCESS)
        throw std::runtime_error{"failed to submit draw command buffer!"};

    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = &renderFinishedSemaphores[currentFrame];
    presentInfo.pResults = nullptr;
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &swapchain;
    presentInfo.pImageIndices = &imageIndex;

    vkQueuePresentKHR(presentQueue, &presentInfo);
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || windowResized) {
        windowResized = false;
        recreateSwapchain();
    } else if (result != VK_SUCCESS) {
        throw std::runtime_error{"failed to present swapchain image!"};
    }

    currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void Window::recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex) {
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = 0;
    beginInfo.pInheritanceInfo = nullptr;

    if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS)
        throw std::runtime_error{"failed to begin recording command buffer!"};

    VkRenderPassBeginInfo renderPassBeginInfo{};
    renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassBeginInfo.renderPass = renderPass;
    renderPassBeginInfo.framebuffer = swapchainFramebuffers[imageIndex];
    renderPassBeginInfo.renderArea.offset = {0, 0};
    renderPassBeginInfo.renderArea.extent = swapchainExtent;

    VkClearValue clearColor = {{{0.0f, 0.0f, 0.1f, 1.0f}}};
    renderPassBeginInfo.clearValueCount = 1;
    renderPassBeginInfo.pClearValues = &clearColor;

    vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, &vertexBuffer, offsets);

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(swapchainExtent.width);
    viewport.height = static_cast<float>(swapchainExtent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = swapchainExtent;
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

    vkCmdDraw(commandBuffer, vertices.size(), 1, 0, 0);
    
    vkCmdEndRenderPass(commandBuffer);

    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
        throw std::runtime_error{"failed to record command buffer!"};
}

void Window::cleanupSwapchain() {
    for (auto framebuffer : swapchainFramebuffers)
        if (framebuffer != VK_NULL_HANDLE)
            vkDestroyFramebuffer(device, framebuffer, nullptr);
    
    for (auto imageView : swapchainImageViews)
        if (imageView != VK_NULL_HANDLE)
            vkDestroyImageView(device, imageView, nullptr);

    if (swapchain != VK_NULL_HANDLE)
        vkDestroySwapchainKHR(device, swapchain, nullptr);
}

void Window::recreateSwapchain() {
    int width = 0, height = 0;
    glfwGetFramebufferSize(window, &width, &height);
    while (width == 0 || height == 0) {
        glfwGetFramebufferSize(window, &width, &height);
        glfwWaitEvents();
    }

    vkDeviceWaitIdle(device);

    cleanupSwapchain();

    createSwapChain();
    createFramebuffers();
}

void Window::mainLoop() {
    while(!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        drawFrame();
    }

    vkDeviceWaitIdle(device);
}

void Window::cleanup() {

    for (auto semaphore : imageAvailableSemaphores)
        if (semaphore != VK_NULL_HANDLE)
            vkDestroySemaphore(device, semaphore, nullptr);
    
    for (auto semaphore : renderFinishedSemaphores)
        if (semaphore != VK_NULL_HANDLE)
            vkDestroySemaphore(device, semaphore, nullptr);
    
    for (auto fence : inFlightFences)
        if (fence != VK_NULL_HANDLE)
            vkDestroyFence(device, fence, nullptr);

    cleanupSwapchain();

    if (vertexBuffer != VK_NULL_HANDLE)
        vkDestroyBuffer(device, vertexBuffer, nullptr);

    if (vertexBufferMemory != VK_NULL_HANDLE)
        vkFreeMemory(device, vertexBufferMemory, nullptr);

    if (commandPool != VK_NULL_HANDLE)
        vkDestroyCommandPool(device, commandPool, nullptr);

    if (pipeline != VK_NULL_HANDLE)
        vkDestroyPipeline(device, pipeline, nullptr);

    if (pipelineLayout != VK_NULL_HANDLE)
        vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
    
    if (renderPass != VK_NULL_HANDLE)
        vkDestroyRenderPass(device, renderPass, nullptr);

    if (device != VK_NULL_HANDLE)
        vkDestroyDevice(device, nullptr);

    if (surface != VK_NULL_HANDLE)
        vkDestroySurfaceKHR(instance, surface, nullptr);
    
    if (instance != VK_NULL_HANDLE)
        vkDestroyInstance(instance, nullptr);
    
    if (window)
        glfwDestroyWindow(window);
    
    glfwTerminate();
}

std::vector<const char*> Window::getGlfwRequiredInstanceExtensions() {
    uint32_t count = 0;
    const char** extensions = glfwGetRequiredInstanceExtensions(&count);

    return {extensions, extensions + count};
}

Window::VkInstanceExtensionSupport Window::getVkInstanceExtensionSupport() {
    uint32_t count = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &count, nullptr);

    std::vector<VkExtensionProperties> properties{count};
    vkEnumerateInstanceExtensionProperties(nullptr, &count, properties.data());

    VkInstanceExtensionSupport support;

    for (auto &extension : properties) {
        if (std::strcmp(extension.extensionName, VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME) == 0)
            support.hasKHRPortabilityEnumeration = true;
    }

    return support;
}

Window::VkInstanceLayerSupport Window::getVkInstanceLayerSupport() {
    uint32_t count = 0;
    vkEnumerateInstanceLayerProperties(&count, nullptr);

    std::vector<VkLayerProperties> properties{count};
    vkEnumerateInstanceLayerProperties(&count, properties.data());

    VkInstanceLayerSupport support;

    for (auto &layer : properties) {
        if (std::strcmp(layer.layerName, "VK_LAYER_KHRONOS_validation") == 0)
            support.hasKhronos = true;
    }

    return support;
}

Window::VkDeviceExtensionSupport Window::getVkDeviceExtensionSupport(VkPhysicalDevice device) {
    uint32_t count = 0;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &count, nullptr);

    std::vector<VkExtensionProperties> properties{count};
    vkEnumerateDeviceExtensionProperties(device, nullptr, &count, properties.data());

    VkDeviceExtensionSupport support;

    for (auto &extension : properties) {
        if (std::strcmp(extension.extensionName, VK_KHR_SWAPCHAIN_EXTENSION_NAME) == 0)
            support.hasKHRSwapchain = true;
    }

    return support;
}

Window::QueueFamilies Window::getVkDeviceQueueFamilies(VkPhysicalDevice device) {
    uint32_t count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &count, nullptr);

    std::vector<VkQueueFamilyProperties> properties{count};
    vkGetPhysicalDeviceQueueFamilyProperties(device, &count, properties.data());

    QueueFamilies indices;

    uint32_t i = 0;
    for (auto &queue : properties) {
        if (queue.queueFlags & VK_QUEUE_GRAPHICS_BIT)
            indices.graphics = i;

        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);
        if (presentSupport)
            indices.present = i;

        if (indices.isComplete())
            break;

        i++;
    }

    return indices;
}

std::optional<VkSurfaceFormatKHR> Window::chooseSurfaceFormat(VkPhysicalDevice device) {
    uint32_t count = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &count, nullptr);

    if (count == 0)
        return {};

    std::vector<VkSurfaceFormatKHR> formats{count};
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &count, formats.data());

    for (auto &format : formats) {
        if (format.format == VK_FORMAT_R8G8B8A8_SRGB && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
            return format;
    }

    return formats[0];
}

std::optional<VkPresentModeKHR> Window::choosePresentMode(VkPhysicalDevice device) {
    uint32_t count = 0;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &count, nullptr);

    if (count == 0)
        return {};

    std::vector<VkPresentModeKHR> modes{count};
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &count, modes.data());

    for (auto mode : modes) {
        if (mode == VK_PRESENT_MODE_MAILBOX_KHR)
            return VK_PRESENT_MODE_MAILBOX_KHR;
    }

    return VK_PRESENT_MODE_FIFO_KHR;
}

Window::PhysicalDevice Window::pickPhysicalDevice() {
    uint32_t count = 0;
    vkEnumeratePhysicalDevices(instance, &count, nullptr);

    std::vector<VkPhysicalDevice> devices{count};
    vkEnumeratePhysicalDevices(instance, &count, devices.data());

    for (auto &device : devices) {
        auto queues = getVkDeviceQueueFamilies(device);
        if (!queues.isComplete())
            continue;

        auto support = getVkDeviceExtensionSupport(device);
        if (!support.hasKHRSwapchain)
            continue;

        auto surfaceFormat = chooseSurfaceFormat(device);
        if (!surfaceFormat.has_value())
            continue;

        auto presentMode = choosePresentMode(device);
        if (!presentMode.has_value())
            continue;

        return PhysicalDevice {
            device,
            queues,
            surfaceFormat.value(),
            presentMode.value()
        };
    }

    throw std::runtime_error{"no suitable device found!"};
}

void Window::createBuffer(
    VkDeviceSize size,
    VkBufferUsageFlags usage,
    VkMemoryPropertyFlags properties,
    VkBuffer& buffer,
    VkDeviceMemory& deviceMemory){
    VkBufferCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    createInfo.size = size;
    createInfo.usage = usage;
    createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(device, &createInfo, nullptr, &buffer) != VK_SUCCESS)
        throw std::runtime_error{ "failed to create vertex buffer!" };

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(device, buffer, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

    if (vkAllocateMemory(device, &allocInfo, nullptr, &deviceMemory) != VK_SUCCESS)
        throw std::runtime_error("failed to allocate vertex buffer memory!");

    vkBindBufferMemory(device, buffer, deviceMemory, 0);
}

void Window::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    if (vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer) != VK_SUCCESS)
        throw std::runtime_error{ "failed to create command buffer!" };

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &beginInfo);
    VkBufferCopy copyRegion{};
    copyRegion.srcOffset = 0;
    copyRegion.dstOffset = 0;
    copyRegion.size = size;
    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);
    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(graphicsQueue);
    vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);

}

uint32_t Window::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice.handle, &memProperties);

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }

    throw std::runtime_error("failed to find suitable memory type!");
}

VkShaderModule Window::loadShaderModule(const std::string &path) {
    std::ifstream is(path, std::ios::binary | std::ios::ate);
    if (!is)
        throw std::runtime_error{"failed to load shader module"};

    std::vector<uint32_t> code;
    code.resize(is.tellg());

    is.seekg(0);
    is.read(reinterpret_cast<char*>(code.data()), code.size());
    is.close();

    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size();
    createInfo.pCode = code.data();

    VkShaderModule shaderModule;
    if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
        throw std::runtime_error{"failed to create shader module"};

    return shaderModule;
}

void Window::onResize(int width, int height) {
    windowResized = true;
}

void Window::glfwOnResizeCallback(GLFWwindow *window, int width, int height) {
    auto app = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
    app->onResize(width, height);
}