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
#define ENABLE_VALIDATION false
class QtUIVulkanExample : public VulkanExampleBase
{
public:
    QtUIVulkanExample() : VulkanExampleBase(ENABLE_VALIDATION)
    {
    }

    virtual ~QtUIVulkanExample()
    {
    }


#ifdef UNITY_BUILD
    bool initVulkanUnity()
    {
        VkResult err;

        // Vulkan instance
        instance = m_Instance.instance;
        physicalDevice = m_Instance.physicalDevice;
        vkGetPhysicalDeviceMemoryProperties(physicalDevice, &deviceMemoryProperties);
        vulkanDevice = new vks::VulkanDevice(physicalDevice);
        vulkanDevice->logicalDevice = m_Instance.device;
        device = vulkanDevice->logicalDevice;
        queue = m_Instance.graphicsQueue;


        return true;
    }
#endif

    virtual void mapExternalObjectToGraphicsSubSystem()
    {
#ifdef UNITY_BUILD
        initVulkanUnity();

        XXUnityVulkanRecordingState recordingState;
        if (!m_UnityVulkan->CommandRecordingState(&recordingState, XXkUnityVulkanGraphicsQueueAccess_DontCare))
            return;

        // Unity does not destroy render passes, so this is safe regarding ABA-problem
        if (recordingState.renderPass != renderPass)
        {
            renderPass = recordingState.renderPass;
        }
#else
        // Todo: Non-Unity Gfx mapping
#endif
    }

#ifdef UNITY_BUILD
    virtual void ProcessDeviceEvent(UnityGfxDeviceEventType type, IUnityInterfaces* interfaces) {}

    IUnityGraphicsVulkan* m_UnityVulkan = NULL;
    XXUnityVulkanInstance m_Instance;
#endif
};
