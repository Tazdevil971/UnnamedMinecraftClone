#include "Context.hpp"

#include <cstring>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <vector>

using namespace render;

std::vector<const char *> getGlfwExtensions() {
    uint32_t count = 0;
    const char **extensions = glfwGetRequiredInstanceExtensions(&count);

    return {extensions, extensions + count};
}

std::unique_ptr<Context> Context::INSTANCE;

Context::Context(GLFWwindow *window) : window{window} {
    try {
        createInstance();
        createSurface();
        deviceInfo = pickPhysicalDevice();
        createDevice();
    } catch (...) {
        cleanup();
        throw;
    }
}

Context::~Context() { cleanup(); }

void Context::cleanup() {
    if (device != VK_NULL_HANDLE) {
        vkDestroyDevice(device, nullptr);
        device = VK_NULL_HANDLE;
    }

    if (surface != VK_NULL_HANDLE) {
        vkDestroySurfaceKHR(instance, surface, nullptr);
        surface = VK_NULL_HANDLE;
    }

    if (instance != VK_NULL_HANDLE) {
        vkDestroyInstance(instance, nullptr);
        instance = VK_NULL_HANDLE;
    }
}

void Context::createInstance() {
    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "UnnamedMinecraftClone";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;

    auto requiredExtensions = getGlfwExtensions();
    auto instanceExtensions = getInstanceExtensionSupport();
    if (instanceExtensions.hasKHRPortabilityEnumeration) {
        requiredExtensions.push_back(
            VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
        createInfo.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
    }

    createInfo.enabledExtensionCount = requiredExtensions.size();
    createInfo.ppEnabledExtensionNames = requiredExtensions.data();

    auto instanceLayers = getInstanceLayerSupport();
    const char *layerNames[] = {"VK_LAYER_KHRONOS_validation"};
    if (instanceLayers.hasKhronos) {
        std::cout << "[INFO] Enabling VK_LAYER_KHRONOS_validation" << std::endl;
        createInfo.enabledLayerCount = 1;
        createInfo.ppEnabledLayerNames = layerNames;
    } else {
        createInfo.enabledLayerCount = 0;
        createInfo.ppEnabledLayerNames = nullptr;
    }

    if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS)
        throw std::runtime_error{"failed to create instance!"};
}

void Context::createSurface() {
    if (glfwCreateWindowSurface(instance, window, nullptr, &surface) !=
        VK_SUCCESS)
        throw std::runtime_error{"failed to create window surface!"};
}

Context::DeviceInfo Context::pickPhysicalDevice() {
    uint32_t count = 0;
    vkEnumeratePhysicalDevices(instance, &count, nullptr);

    std::vector<VkPhysicalDevice> devices{count};
    vkEnumeratePhysicalDevices(instance, &count, devices.data());

    for (auto &device : devices) {
        auto queues = getDeviceQueueFamilies(device);
        if (!queues.isComplete()) continue;

        auto support = getDeviceExtensionSupport(device);
        if (!support.hasKHRSwapchain) continue;

        auto surfaceFormat = chooseSurfaceFormat(device);
        if (!surfaceFormat.has_value()) continue;

        auto presentMode = choosePresentMode(device);
        if (!presentMode.has_value()) continue;

        auto depthFormat = findFirstSupportedFormat(
            device,
            {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT,
             VK_FORMAT_D24_UNORM_S8_UINT},
            VK_IMAGE_TILING_OPTIMAL,
            VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
        if (!depthFormat.has_value()) continue;

        VkPhysicalDeviceFeatures supportedFeatures{};
        vkGetPhysicalDeviceFeatures(device, &supportedFeatures);

        VkPhysicalDeviceProperties props{};
        vkGetPhysicalDeviceProperties(device, &props);

        bool hasFilterAnisotropy =
            supportedFeatures.samplerAnisotropy == VK_TRUE;

        return DeviceInfo{queues,
                          device,
                          surfaceFormat.value(),
                          presentMode.value(),
                          depthFormat.value(),
                          hasFilterAnisotropy,
                          support.hasKHRDedicatedAllocation,
                          props.limits.maxSamplerAnisotropy};
    }

    throw std::runtime_error{"no suitable device found!"};
}

void Context::createDevice() {
    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos{};

    const float queuePriorities[] = {1.0f};

    VkDeviceQueueCreateInfo queueCreateInfo{};
    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo.queueCount = 1;
    queueCreateInfo.pQueuePriorities = queuePriorities;

    queueCreateInfo.queueFamilyIndex = deviceInfo.queues.graphics.value();
    queueCreateInfos.push_back(queueCreateInfo);

    if (deviceInfo.queues.hasDedicatedPresentQueue()) {
        std::cout << "[INFO] Device has dedicated present queue" << std::endl;
        queueCreateInfo.queueFamilyIndex = deviceInfo.queues.present.value();
        queueCreateInfos.push_back(queueCreateInfo);
    }

    VkPhysicalDeviceFeatures deviceFeatures{};
    deviceFeatures.samplerAnisotropy =
        deviceInfo.hasFilterAnisotropy ? VK_TRUE : VK_FALSE;

    VkDeviceCreateInfo deviceCreateInfo{};
    deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
    deviceCreateInfo.queueCreateInfoCount = queueCreateInfos.size();

    std::vector<const char *> requiredExtensions{
        VK_KHR_SWAPCHAIN_EXTENSION_NAME};
    if (deviceInfo.hasKHRDedicatedAllocation) {
        std::cout << "[INFO] Enabling VK_KHR_dedicated_allocation" << std::endl;
        requiredExtensions.push_back(
            VK_KHR_DEDICATED_ALLOCATION_EXTENSION_NAME);
        requiredExtensions.push_back(
            VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME);
    }

    deviceCreateInfo.enabledExtensionCount = requiredExtensions.size();
    deviceCreateInfo.ppEnabledExtensionNames = requiredExtensions.data();

    deviceCreateInfo.pEnabledFeatures = &deviceFeatures;

    if (vkCreateDevice(deviceInfo.device, &deviceCreateInfo, nullptr,
                       &device) != VK_SUCCESS)
        throw std::runtime_error{"failed to create device!"};

    vkGetDeviceQueue(device, deviceInfo.queues.graphics.value(), 0,
                     &graphicsQueue);
    vkGetDeviceQueue(device, deviceInfo.queues.present.value(), 0,
                     &presentQueue);
}

Context::InstanceExtensions Context::getInstanceExtensionSupport() {
    uint32_t count = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &count, nullptr);

    std::vector<VkExtensionProperties> properties{count};
    vkEnumerateInstanceExtensionProperties(nullptr, &count, properties.data());

    InstanceExtensions support;

    for (auto &extension : properties) {
        if (std::strcmp(extension.extensionName,
                        VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME) == 0)
            support.hasKHRPortabilityEnumeration = true;
    }

    return support;
}

Context::InstanceLayers Context::getInstanceLayerSupport() {
    uint32_t count = 0;
    vkEnumerateInstanceLayerProperties(&count, nullptr);

    std::vector<VkLayerProperties> properties{count};
    vkEnumerateInstanceLayerProperties(&count, properties.data());

    InstanceLayers support;

    for (auto &layer : properties) {
        if (std::strcmp(layer.layerName, "VK_LAYER_KHRONOS_validation") == 0)
            support.hasKhronos = true;
    }

    return support;
}

Context::DeviceExtensions Context::getDeviceExtensionSupport(
    VkPhysicalDevice device) {
    uint32_t count = 0;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &count, nullptr);

    std::vector<VkExtensionProperties> properties{count};
    vkEnumerateDeviceExtensionProperties(device, nullptr, &count,
                                         properties.data());

    DeviceExtensions support;

    for (auto &extension : properties) {
        if (std::strcmp(extension.extensionName,
                        VK_KHR_SWAPCHAIN_EXTENSION_NAME) == 0)
            support.hasKHRSwapchain = true;

        if (std::strcmp(extension.extensionName,
                        VK_KHR_DEDICATED_ALLOCATION_EXTENSION_NAME) == 0)
            support.hasKHRDedicatedAllocation = true;
    }

    return support;
}

Context::QueueFamilies Context::getDeviceQueueFamilies(
    VkPhysicalDevice device) {
    uint32_t count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &count, nullptr);

    std::vector<VkQueueFamilyProperties> properties{count};
    vkGetPhysicalDeviceQueueFamilyProperties(device, &count, properties.data());

    QueueFamilies indices;

    uint32_t i = 0;
    for (auto &queue : properties) {
        if (queue.queueFlags & VK_QUEUE_GRAPHICS_BIT) indices.graphics = i;

        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface,
                                             &presentSupport);
        if (presentSupport) indices.present = i;

        if (indices.isComplete()) break;

        i++;
    }

    return indices;
}

std::optional<VkSurfaceFormatKHR> Context::chooseSurfaceFormat(
    VkPhysicalDevice device) {
    uint32_t count = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &count, nullptr);

    if (count == 0) return {};

    std::vector<VkSurfaceFormatKHR> formats{count};
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &count,
                                         formats.data());

    for (auto &format : formats) {
        if (format.format == VK_FORMAT_R8G8B8A8_SRGB &&
            format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
            return format;
    }

    return formats[0];
}

std::optional<VkPresentModeKHR> Context::choosePresentMode(
    VkPhysicalDevice device) {
    uint32_t count = 0;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &count, nullptr);

    if (count == 0) return {};

    std::vector<VkPresentModeKHR> modes{count};
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &count,
                                              modes.data());

    for (auto mode : modes) {
        if (mode == VK_PRESENT_MODE_MAILBOX_KHR)
            return VK_PRESENT_MODE_MAILBOX_KHR;
    }

    return VK_PRESENT_MODE_FIFO_KHR;
}

std::optional<VkFormat> Context::findFirstSupportedFormat(
    VkPhysicalDevice device, const std::vector<VkFormat> &formats,
    VkImageTiling tiling, VkFormatFeatureFlags features) const {
    for (const auto &format : formats) {
        VkFormatProperties props;
        vkGetPhysicalDeviceFormatProperties(device, format, &props);

        if (tiling == VK_IMAGE_TILING_LINEAR &&
            (props.linearTilingFeatures & features) == features) {
            return format;
        } else if (tiling == VK_IMAGE_TILING_OPTIMAL &&
                   (props.optimalTilingFeatures & features) == features) {
            return format;
        }
    }

    return {};
}

VkShaderModule Context::loadShaderModule(const std::string &path) const {
    std::ifstream is(path, std::ios::binary | std::ios::ate);
    if (!is) throw std::runtime_error{"failed to load shader module"};

    std::vector<uint32_t> code;
    code.resize(is.tellg());

    is.seekg(0);
    is.read(reinterpret_cast<char *>(code.data()), code.size());
    is.close();

    return loadShaderModule(code.data(), code.size());
}

VkShaderModule Context::loadShaderModule(const uint32_t *code,
                                         size_t size) const {
    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = size;
    createInfo.pCode = code;

    VkShaderModule shaderModule;
    if (vkCreateShaderModule(getDevice(), &createInfo, nullptr,
                             &shaderModule) != VK_SUCCESS)
        throw std::runtime_error{"failed to create shader module"};

    return shaderModule;
}

void Context::waitDeviceIdle() { vkDeviceWaitIdle(getDevice()); }
