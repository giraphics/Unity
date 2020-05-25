#pragma once

#include "QtUIVulkanExample.h"

#define VERTEX_BUFFER_BIND_ID 0
class VulkanExample : public QtUIVulkanExample
{
public:
    struct Vertex {
        float pos[3];
        float color[3];
    };

    vks::Buffer vertexBuffer;

    struct {
        VkPipelineVertexInputStateCreateInfo inputState;
        std::vector<VkVertexInputBindingDescription> bindingDescriptions;
        std::vector<VkVertexInputAttributeDescription> attributeDescriptions;
    } vertices;

    VulkanExample();
    virtual ~VulkanExample();

public:
    void prepareVertices();
    void preparePipelines();
    void setupVertexDescriptions();

    void paint(VkCommandBuffer commandBuffer);
    void draw();

    void buildCommandBuffers() override;
    void render() override;
    void prepare() override;

    virtual void viewChanged() override
    {
        // This function is called by the base example class each time the view is changed by user input
        // updateUniformBuffers(); // Update Projection and View
    }


#ifdef UNITY_BUILD
    virtual void ProcessDeviceEvent(UnityGfxDeviceEventType type, IUnityInterfaces* interfaces);
private:
    void handleUnityGfxDeviceEventInitialize(IUnityInterfaces* interfaces);
    void handleUnityGfxDeviceEventShutdown();
#endif

private:
    VkPipeline pipeline;
};
