#pragma once

#include <cstring>

//#define USE_VULKAN_HEADERS

#ifdef USE_VULKAN_HEADERS
#include "vulkan/vulkan.h"
#include "Unity/IUnityGraphicsVulkan.h"

static PFN_vkGetInstanceProcAddr UNITY_INTERFACE_API InterceptVulkanInitialization(PFN_vkGetInstanceProcAddr getInstanceProcAddr, void*)
{
    return &vkGetInstanceProcAddr;
}

static void LoadVulkanAPI(PFN_vkGetInstanceProcAddr getInstanceProcAddr, VkInstance instance)
{
}

static VKAPI_ATTR void VKAPI_CALL Hook_vkCmdBeginRenderPass(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo* pRenderPassBegin, VkSubpassContents contents)
{
    // Change this to 'true' to override the clear color with green
    const bool allowOverrideClearColor = true; // Parminder: changed to true
    if (pRenderPassBegin->clearValueCount <= 16 && pRenderPassBegin->clearValueCount > 0 && allowOverrideClearColor)
    {
        VkClearValue clearValues[16] = {};
        memcpy(clearValues, pRenderPassBegin->pClearValues, pRenderPassBegin->clearValueCount * sizeof(VkClearValue));

        VkRenderPassBeginInfo patchedBeginInfo = *pRenderPassBegin;
        patchedBeginInfo.pClearValues = clearValues;
        for (unsigned int i = 0; i < pRenderPassBegin->clearValueCount - 1; ++i)
        {
            clearValues[i].color.float32[0] = 0.0;
            clearValues[i].color.float32[1] = 1.0;
            clearValues[i].color.float32[2] = 1.0;
            clearValues[i].color.float32[3] = 1.0;
        }
        vkCmdBeginRenderPass(commandBuffer, &patchedBeginInfo, contents);
    }
    else
    {
        vkCmdBeginRenderPass(commandBuffer, pRenderPassBegin, contents);
    }
}

#else

// This plugin does not link to the Vulkan loader, easier to support multiple APIs and systems that don't have Vulkan support
//#define VK_NO_PROTOTYPES
#include "Unity/IUnityGraphicsVulkan.h"
#include <vulkan/vulkan.h>
#include <iostream>
#include <assert.h>

//#define UNITY_USED_VULKAN_API_FUNCTIONS(apply) \
//    apply(vkCreateInstance); \
//    apply(vkCmdBeginRenderPass); \
//    apply(vkCreateBuffer); \
//    apply(vkGetPhysicalDeviceMemoryProperties); \
//    apply(vkGetBufferMemoryRequirements); \
//    apply(vkMapMemory); \
//    apply(vkBindBufferMemory); \
//    apply(vkAllocateMemory); \
//    apply(vkDestroyBuffer); \
//    apply(vkFreeMemory); \
//    apply(vkUnmapMemory); \
//    apply(vkQueueWaitIdle); \
//    apply(vkDeviceWaitIdle); \
//    apply(vkCmdCopyBufferToImage); \
//    apply(vkFlushMappedMemoryRanges); \
//    apply(vkCreatePipelineLayout); \
//    apply(vkCreateShaderModule); \
//    apply(vkDestroyShaderModule); \
//    apply(vkCreateGraphicsPipelines); \
//    apply(vkCmdBindPipeline); \
//    apply(vkCmdDraw); \
//    apply(vkCmdPushConstants); \
//    apply(vkCmdBindVertexBuffers); \
//    apply(vkDestroyPipeline); \
//    apply(vkDestroyPipelineLayout); \
//    apply(vkCreatePipelineCache);

//#define VULKAN_DEFINE_API_FUNCPTR(func) static PFN_##func func
//VULKAN_DEFINE_API_FUNCPTR(vkGetInstanceProcAddr);
//UNITY_USED_VULKAN_API_FUNCTIONS(VULKAN_DEFINE_API_FUNCPTR);
//#undef VULKAN_DEFINE_API_FUNCPTR

//#define LOAD_VULKAN_FUNC(fn) if (!fn) fn = (PFN_##fn)vkGetInstanceProcAddr(instance, #fn)
//#undef LOAD_VULKAN_FUNC
//static PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr;
#define VK_CHECK_RESULT(f)																				\
{																										\
    VkResult res = (f);																					\
    if (res != VK_SUCCESS)																				\
    {																									\
        std::cout << "Fatal : VkResult is \"" << res << "\" in " << __FILE__ << " at line " << __LINE__ << std::endl; \
        assert(res == VK_SUCCESS);																		\
    }																									\
}

//static void LoadVulkanAPI(PFN_vkGetInstanceProcAddr getInstanceProcAddr, VkInstance instance)
//{
//    if (!vkGetInstanceProcAddr && getInstanceProcAddr)
//        vkGetInstanceProcAddr = getInstanceProcAddr;

////    if (!vkCreateInstance)
////        vkCreateInstance = (PFN_vkCreateInstance)vkGetInstanceProcAddr(VK_NULL_HANDLE, "vkCreateInstance");

//UNITY_USED_VULKAN_API_FUNCTIONS(LOAD_VULKAN_FUNC);
//}

static VKAPI_ATTR void VKAPI_CALL Hook_vkCmdBeginRenderPass(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo* pRenderPassBegin, VkSubpassContents contents)
{
    // Change this to 'true' to override the clear color with green
    const bool allowOverrideClearColor = true; // Parminder: changed to true
    if (pRenderPassBegin->clearValueCount <= 16 && pRenderPassBegin->clearValueCount > 0 && allowOverrideClearColor)
    {
        VkClearValue clearValues[16] = {};
        memcpy(clearValues, pRenderPassBegin->pClearValues, pRenderPassBegin->clearValueCount * sizeof(VkClearValue));

        VkRenderPassBeginInfo patchedBeginInfo = *pRenderPassBegin;
        patchedBeginInfo.pClearValues = clearValues;
        for (unsigned int i = 0; i < pRenderPassBegin->clearValueCount - 1; ++i)
        {
            clearValues[i].color.float32[0] = 0.0f;
            clearValues[i].color.float32[1] = 0.0f;
            clearValues[i].color.float32[2] = 0.2f;
            clearValues[i].color.float32[3] = 1.0f;
        }
        vkCmdBeginRenderPass(commandBuffer, &patchedBeginInfo, contents);
    }
    else
    {
        vkCmdBeginRenderPass(commandBuffer, pRenderPassBegin, contents);
    }
}

static VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL Hook_vkGetInstanceProcAddr(VkInstance device, const char* funcName)
{
    if (!funcName)
        return NULL;

//#define INTERCEPT(fn) if (strcmp(funcName, #fn) == 0) return (PFN_vkVoidFunction)&Hook_##fn
//    INTERCEPT(vkCreateInstance);
//#undef INTERCEPT

    return NULL;
}

//static PFN_vkGetInstanceProcAddr UNITY_INTERFACE_API InterceptVulkanInitialization(PFN_vkGetInstanceProcAddr getInstanceProcAddr, void*)
//{
//    vkGetInstanceProcAddr = getInstanceProcAddr;
//    return Hook_vkGetInstanceProcAddr;
//}

#endif // #USE_VULKAN_HEADERS
