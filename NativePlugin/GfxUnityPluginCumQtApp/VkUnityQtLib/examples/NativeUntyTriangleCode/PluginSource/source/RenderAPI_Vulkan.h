#pragma once
#include "RenderAPI.h"

#include "../../Common/QtUIVulkanExample.h"

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

public:
    void prepareVertices();
    void preparePipelines();

    VkShaderModule loadSPIRVShader(std::string filename);
    VkShaderModule loadSPIRVShader(const uint32_t* pCode, size_t codeSize);
    void paint(VkCommandBuffer commandBuffer);

    void render() override;
    void prepare() override;

    virtual void ProcessDeviceEvent(UnityGfxDeviceEventType type, IUnityInterfaces* interfaces);
    virtual void DrawTriangle();

private:
    VkPipeline pipeline;
};
