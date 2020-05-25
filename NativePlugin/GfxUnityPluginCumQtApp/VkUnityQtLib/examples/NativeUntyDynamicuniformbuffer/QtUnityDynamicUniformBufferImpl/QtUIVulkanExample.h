#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <fstream>
#include <vector>
#include <exception>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <vulkan/vulkan.h>
#include "vulkanexamplebase.h"

#ifdef UNITY_BUILD
#include "../PluginSource/source/Unity/IUnityGraphicsVulkan.h"
#endif
#include "../PluginSource/source/Unity/IUnityGraphics.h"

// Set to "true" to enable Vulkan's validation layers (see vulkandebug.cpp for details)
#define ENABLE_VALIDATION true
class QtUIVulkanExample : public VulkanExampleBase
{
public:
    VkSemaphore presentCompleteSemaphore;
    VkSemaphore renderCompleteSemaphore;
    std::vector<VkFence> waitFences;

    QtUIVulkanExample() : VulkanExampleBase(ENABLE_VALIDATION)
    {
    }

    virtual ~QtUIVulkanExample()
    {
//        vkDestroySemaphore(device, presentCompleteSemaphore, nullptr);
//        vkDestroySemaphore(device, renderCompleteSemaphore, nullptr);

//        for (auto& fence : waitFences)
//        {
//            vkDestroyFence(device, fence, nullptr);
//        }
    }

//    void draw()
//    {
//        // Get next image in the swap chain (back/front buffer)
//        VK_CHECK_RESULT(swapChain.acquireNextImage(presentCompleteSemaphore, &currentBuffer));

//        // Use a fence to wait until the command buffer has finished execution before using it again
//        VK_CHECK_RESULT(vkWaitForFences(device, 1, &waitFences[currentBuffer], VK_TRUE, UINT64_MAX));
//        VK_CHECK_RESULT(vkResetFences(device, 1, &waitFences[currentBuffer]));

//        // Pipeline stage at which the queue submission will wait (via pWaitSemaphores)
//        VkPipelineStageFlags waitStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
//        // The submit info structure specifices a command buffer queue submission batch
//        VkSubmitInfo submitInfo = {};
//        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
//        submitInfo.pWaitDstStageMask = &waitStageMask;               // Pointer to the list of pipeline stages that the semaphore waits will occur at
//        submitInfo.pWaitSemaphores = &presentCompleteSemaphore;      // Semaphore(s) to wait upon before the submitted command buffer starts executing
//        submitInfo.waitSemaphoreCount = 1;                           // One wait semaphore
//        submitInfo.pSignalSemaphores = &renderCompleteSemaphore;     // Semaphore(s) to be signaled when command buffers have completed
//        submitInfo.signalSemaphoreCount = 1;                         // One signal semaphore
//        submitInfo.pCommandBuffers = &drawCmdBuffers[currentBuffer]; // Command buffers(s) to execute in this batch (submission)
//        submitInfo.commandBufferCount = 1;                           // One command buffer

//        // Submit to the graphics queue passing a wait fence
//        VK_CHECK_RESULT(vkQueueSubmit(queue, 1, &submitInfo, waitFences[currentBuffer]));

//        // Present the current buffer to the swap chain
//        // Pass the semaphore signaled by the command buffer submission from the submit info as the wait semaphore for swap chain presentation
//        // This ensures that the image is not presented to the windowing system until all commands have been submitted
//        VkResult present = swapChain.queuePresent(queue, currentBuffer, renderCompleteSemaphore);
//        if (!((present == VK_SUCCESS) || (present == VK_SUBOPTIMAL_KHR))) {
//            VK_CHECK_RESULT(present);
//        }
//    }

#ifdef UNITY_BUILD
    bool initVulkanUnity()
    {
        VkResult err;

        // Vulkan instance
        instance = m_Instance.instance;

//        err = createInstance(settings.validation);
//        if (err) {
//            vks::tools::exitFatal("Could not create Vulkan instance : \n" + vks::tools::errorString(err), err);
//            return false;
//        }

//        // If requested, we enable the default validation layers for debugging
//        if (settings.validation)
//        {
//            // The report flags determine what type of messages for the layers will be displayed
//            // For validating (debugging) an appplication the error and warning bits should suffice
//            VkDebugReportFlagsEXT debugReportFlags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;
//            // Additional flags include performance info, loader and layer debug messages, etc.
//            vks::debug::setupDebugging(instance, debugReportFlags, VK_NULL_HANDLE);
//        }

        // Physical device
//        uint32_t gpuCount = 0;
//        // Get number of available physical devices
//        VK_CHECK_RESULT(vkEnumeratePhysicalDevices(instance, &gpuCount, nullptr));
//        assert(gpuCount > 0);
//        // Enumerate devices
//        std::vector<VkPhysicalDevice> physicalDevices(gpuCount);
//        err = vkEnumeratePhysicalDevices(instance, &gpuCount, physicalDevices.data());
//        if (err) {
//            vks::tools::exitFatal("Could not enumerate physical devices : \n" + vks::tools::errorString(err), err);
//            return false;
//        }

//        // GPU selection

//        // Select physical device to be used for the Vulkan example
//        // Defaults to the first device unless specified by command line
//        uint32_t selectedDevice = 0;

//    #if !defined(VK_USE_PLATFORM_ANDROID_KHR)
//        // GPU selection via command line argument
//        for (size_t i = 0; i < args.size(); i++)
//        {
//            // Select GPU
//            if ((args[i] == std::string("-g")) || (args[i] == std::string("-gpu")))
//            {
//                char* endptr;
//                uint32_t index = strtol(args[i + 1], &endptr, 10);
//                if (endptr != args[i + 1])
//                {
//                    if (index > gpuCount - 1)
//                    {
//                        std::cerr << "Selected device index " << index << " is out of range, reverting to device 0 (use -listgpus to show available Vulkan devices)" << std::endl;
//                    }
//                    else
//                    {
//                        std::cout << "Selected Vulkan device " << index << std::endl;
//                        selectedDevice = index;
//                    }
//                };
//                break;
//            }
//            // List available GPUs
//            if (args[i] == std::string("-listgpus"))
//            {
//                uint32_t gpuCount = 0;
//                VK_CHECK_RESULT(vkEnumeratePhysicalDevices(instance, &gpuCount, nullptr));
//                if (gpuCount == 0)
//                {
//                    std::cerr << "No Vulkan devices found!" << std::endl;
//                }
//                else
//                {
//                    // Enumerate devices
//                    std::cout << "Available Vulkan devices" << std::endl;
//                    std::vector<VkPhysicalDevice> devices(gpuCount);
//                    VK_CHECK_RESULT(vkEnumeratePhysicalDevices(instance, &gpuCount, devices.data()));
//                    for (uint32_t i = 0; i < gpuCount; i++) {
//                        VkPhysicalDeviceProperties deviceProperties;
//                        vkGetPhysicalDeviceProperties(devices[i], &deviceProperties);
//                        std::cout << "Device [" << i << "] : " << deviceProperties.deviceName << std::endl;
//                        std::cout << " Type: " << vks::tools::physicalDeviceTypeString(deviceProperties.deviceType) << std::endl;
//                        std::cout << " API: " << (deviceProperties.apiVersion >> 22) << "." << ((deviceProperties.apiVersion >> 12) & 0x3ff) << "." << (deviceProperties.apiVersion & 0xfff) << std::endl;
//                    }
//                }
//            }
//        }
//    #endif

//        physicalDevice = physicalDevices[selectedDevice];
        physicalDevice = m_Instance.physicalDevice;

        // Store properties (including limits), features and memory properties of the phyiscal device (so that examples can check against them)
//        vkGetPhysicalDeviceProperties(physicalDevice, &deviceProperties);
//        vkGetPhysicalDeviceFeatures(physicalDevice, &deviceFeatures);
        //vkGetPhysicalDeviceMemoryProperties(physicalDevice, &deviceMemoryProperties);

        // Derived examples can override this to set actual features (based on above readings) to enable for logical device creation
        //getEnabledFeatures();

        // Vulkan device creation
        // This is handled by a separate class that gets a logical device representation
        // and encapsulates functions related to a device
        vulkanDevice = new vks::VulkanDevice(physicalDevice);
//        VkResult res = vulkanDevice->createLogicalDevice(enabledFeatures, enabledDeviceExtensions, deviceCreatepNextChain);
//        if (res != VK_SUCCESS) {
//            vks::tools::exitFatal("Could not create Vulkan device: \n" + vks::tools::errorString(res), res);
//            return false;
//        }
        vulkanDevice->logicalDevice = m_Instance.device;
        device = vulkanDevice->logicalDevice;

        // Get a graphics queue from the device
//        vkGetDeviceQueue(device, vulkanDevice->queueFamilyIndices.graphics, 0, &queue);
        queue = m_Instance.graphicsQueue;

//        // Find a suitable depth format
//        VkBool32 validDepthFormat = vks::tools::getSupportedDepthFormat(physicalDevice, &depthFormat);
//        assert(validDepthFormat);

//        swapChain.connect(instance, physicalDevice, device);

//        // Create synchronization objects
//        VkSemaphoreCreateInfo semaphoreCreateInfo = vks::initializers::semaphoreCreateInfo();
//        // Create a semaphore used to synchronize image presentation
//        // Ensures that the image is displayed before we start submitting new commands to the queu
//        VK_CHECK_RESULT(vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &semaphores.presentComplete));
//        // Create a semaphore used to synchronize command submission
//        // Ensures that the image is not presented until all commands have been sumbitted and executed
//        VK_CHECK_RESULT(vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &semaphores.renderComplete));

//        // Set up submit info structure
//        // Semaphores will stay the same during application lifetime
//        // Command buffer submission info is set by each example
//        submitInfo = vks::initializers::submitInfo();
//        submitInfo.pWaitDstStageMask = &submitPipelineStages;
//        submitInfo.waitSemaphoreCount = 1;
//        submitInfo.pWaitSemaphores = &semaphores.presentComplete;
//        submitInfo.signalSemaphoreCount = 1;
//        submitInfo.pSignalSemaphores = &semaphores.renderComplete;

        return true;
    }
#endif

    virtual void mapExternalObjectToGraphicsSubSystem()
    {
#ifdef UNITY_BUILD
        vkGetPhysicalDeviceMemoryProperties(m_Instance.physicalDevice, &deviceMemoryProperties);
        device = m_Instance.device;

        XXUnityVulkanRecordingState recordingState;
        if (!m_UnityVulkan->CommandRecordingState(&recordingState, XXkUnityVulkanGraphicsQueueAccess_DontCare))
            return;

        initVulkanUnity();
        // Unity does not destroy render passes, so this is safe regarding ABA-problem
        if (recordingState.renderPass != renderPass)
        {
            renderPass = recordingState.renderPass;
        }
#else
        // Todo: Non-Unity Gfx mapping
#endif
    }

    virtual void ProcessDeviceEvent(UnityGfxDeviceEventType type, IUnityInterfaces* interfaces) {}

#ifdef UNITY_BUILD
    IUnityGraphicsVulkan* m_UnityVulkan = NULL;
    XXUnityVulkanInstance m_Instance;
#endif
};
