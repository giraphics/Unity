#pragma once
#include "RenderAPI.h"

#include <string.h>
#include <map>
#include <vector>
#include <math.h>

#include "VkQtUnityCommon.h"
#include "triangleShaders.h"

#include "../../QtUIVulkanExample.h"

class RenderAPI_VulkanNew : public RenderAPI, public QtUIVulkanExample
{
public:
    struct Vertex {
        float position[3];
        float color[3];
    };

    struct {
        VkDeviceMemory memory; // Handle to the device memory for this buffer
        VkBuffer buffer;       // Handle to the Vulkan buffer object that the memory is bound to
    } vertices;

    RenderAPI_VulkanNew();
    virtual ~RenderAPI_VulkanNew();

    virtual void ProcessDeviceEvent(UnityGfxDeviceEventType type, IUnityInterfaces* interfaces);
    virtual void DrawTriangle();

    // BASE XXXXXXXXXXXXXXXXX
    uint32_t getMemoryTypeIndex(uint32_t typeBits, VkMemoryPropertyFlags properties); // Move to base class
    void render() override {}
    // BASE XXXXXXXXXXXXXXXXX

public:
    void prepareVertices();
    void preparePipelines();

    VkShaderModule loadSPIRVShader(std::string filename);
    VkShaderModule loadSPIRVShader(const uint32_t* pCode, size_t codeSize);
    void paint(VkCommandBuffer commandBuffer);

    // BASE XXXXXXXXXXXXXXXXX
    void mapUnityToQtVkObjects();
    void createPipelineCache(); // Should be in base class?
    // BASE XXXXXXXXXXXXXXXXX

private:
    // BASE XXXXXXXXXXXXXXXXX
    IUnityGraphicsVulkan* m_UnityVulkan = NULL;
    XXUnityVulkanInstance m_Instance;
    // BASE XXXXXXXXXXXXXXXXX

    VkPhysicalDeviceMemoryProperties deviceMemoryProperties;
    VkPipeline pipeline = VK_NULL_HANDLE;

    VkPipelineCache pipelineCache = VK_NULL_HANDLE;
    VkDevice device = VK_NULL_HANDLE;
    VkRenderPass renderPass = VK_NULL_HANDLE;
};
