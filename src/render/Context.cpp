#include "Context.hpp"

#include <cstring>
#include <stdexcept>
#include <vector>

using namespace render;

std::vector<const char *> getGlfwExtensions() {
    uint32_t count = 0;
    const char **extensions = glfwGetRequiredInstanceExtensions(&count);

    return {extensions, extensions + count};
}

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

void Context::cleanup() {
    if (device != VK_NULL_HANDLE) vkDestroyDevice(device, nullptr);

    if (surface != VK_NULL_HANDLE)
        vkDestroySurfaceKHR(instance, surface, nullptr);

    if (instance != VK_NULL_HANDLE) vkDestroyInstance(instance, nullptr);
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
    auto extensionSupport = getInstanceExtensionSupport();
    if (extensionSupport.hasKHRPortabilityEnumeration) {
        requiredExtensions.push_back(
            VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
        createInfo.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
    }

    createInfo.enabledExtensionCount = requiredExtensions.size();
    createInfo.ppEnabledExtensionNames = requiredExtensions.data();

    auto layerSupport = getInstanceLayerSupport();
    const char *layerNames[] = {"VK_LAYER_KHRONOS_validation"};
    if (layerSupport.hasKhronos) {
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

        VkPhysicalDeviceFeatures supportedFeatures;
        vkGetPhysicalDeviceFeatures(device, &supportedFeatures);

        bool hasFilterAnisotropy =
            supportedFeatures.samplerAnisotropy == VK_TRUE;

        return DeviceInfo{queues, device, surfaceFormat.value(),
                          presentMode.value(), hasFilterAnisotropy};
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
        queueCreateInfo.queueFamilyIndex = deviceInfo.queues.present.value();
        queueCreateInfos.push_back(queueCreateInfo);
    }

    if (deviceInfo.queues.hasDedicatedTransferQueue()) {
        queueCreateInfo.queueFamilyIndex = deviceInfo.queues.transfer.value();
        queueCreateInfos.push_back(queueCreateInfo);
    }

    VkPhysicalDeviceFeatures deviceFeatures{};
    deviceFeatures.samplerAnisotropy =
        deviceInfo.hasFilterAnisotropy ? VK_TRUE : VK_FALSE;

    VkDeviceCreateInfo deviceCreateInfo{};
    deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
    deviceCreateInfo.queueCreateInfoCount = queueCreateInfos.size();

    const char *extensionNames[] = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
    deviceCreateInfo.enabledExtensionCount = 1;
    deviceCreateInfo.ppEnabledExtensionNames = extensionNames;

    deviceCreateInfo.pEnabledFeatures = &deviceFeatures;

    if (vkCreateDevice(deviceInfo.device, &deviceCreateInfo, nullptr,
                       &device) != VK_SUCCESS)
        throw std::runtime_error{"failed to create device!"};

    vkGetDeviceQueue(device, deviceInfo.queues.graphics.value(), 0,
                     &graphicsQueue);
    vkGetDeviceQueue(device, deviceInfo.queues.present.value(), 0,
                     &presentQueue);
    vkGetDeviceQueue(device, deviceInfo.queues.transfer.value(), 0,
                     &transferQueue);
}

Context::InstanceExtensionSupport Context::getInstanceExtensionSupport() {
    uint32_t count = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &count, nullptr);

    std::vector<VkExtensionProperties> properties{count};
    vkEnumerateInstanceExtensionProperties(nullptr, &count, properties.data());

    InstanceExtensionSupport support;

    for (auto &extension : properties) {
        if (std::strcmp(extension.extensionName,
                        VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME) == 0)
            support.hasKHRPortabilityEnumeration = true;
    }

    return support;
}

Context::InstanceLayerSupport Context::getInstanceLayerSupport() {
    uint32_t count = 0;
    vkEnumerateInstanceLayerProperties(&count, nullptr);

    std::vector<VkLayerProperties> properties{count};
    vkEnumerateInstanceLayerProperties(&count, properties.data());

    InstanceLayerSupport support;

    for (auto &layer : properties) {
        if (std::strcmp(layer.layerName, "VK_LAYER_KHRONOS_validation") == 0)
            support.hasKhronos = true;
    }

    return support;
}

Context::DeviceExtensionSupport Context::getDeviceExtensionSupport(
    VkPhysicalDevice device) {
    uint32_t count = 0;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &count, nullptr);

    std::vector<VkExtensionProperties> properties{count};
    vkEnumerateDeviceExtensionProperties(device, nullptr, &count,
                                         properties.data());

    DeviceExtensionSupport support;

    for (auto &extension : properties) {
        if (std::strcmp(extension.extensionName,
                        VK_KHR_SWAPCHAIN_EXTENSION_NAME) == 0)
            support.hasKHRSwapchain = true;
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
        if (queue.queueFlags & VK_QUEUE_GRAPHICS_BIT)
            indices.graphics = i;
        else if (queue.queueFlags & VK_QUEUE_TRANSFER_BIT)
            indices.transfer = i;

        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface,
                                             &presentSupport);
        if (presentSupport) indices.present = i;

        if (indices.isComplete()) break;

        i++;
    }

    // If we did not find a transfer queue, use the graphics one
    if (!indices.transfer.has_value()) indices.transfer = indices.graphics;

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