#pragma once
#include "QtUIVulkanExample.h"

class VulkanExample : public QtUIVulkanExample
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

    VulkanExample();
    virtual ~VulkanExample();

public:
    void prepareVertices();
    void preparePipelines();

    VkShaderModule loadSPIRVShader(std::string filename);
    VkShaderModule loadSPIRVShader(const uint32_t* pCode, size_t codeSize);
    void paint(VkCommandBuffer commandBuffer);

    void buildCommandBuffers();
    virtual void render();
    void prepare();

    VkPipeline pipeline;
};
