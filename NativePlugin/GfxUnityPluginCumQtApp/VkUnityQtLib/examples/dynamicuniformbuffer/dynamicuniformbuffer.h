#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <vector>
#include <array>
#include <random>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <vulkan/vulkan.h>
#include "vulkanexamplebase.h"
#include "VulkanDevice.hpp"
#include "VulkanBuffer.hpp"

#define VERTEX_BUFFER_BIND_ID 0
#define ENABLE_VALIDATION false
#define OBJECT_INSTANCES 125

// Vertex layout for this example
struct Vertex {
	float pos[3];
	float color[3];
};

class VulkanExample : public VulkanExampleBase
{
public:
	struct {
		VkPipelineVertexInputStateCreateInfo inputState;
		std::vector<VkVertexInputBindingDescription> bindingDescriptions;
		std::vector<VkVertexInputAttributeDescription> attributeDescriptions;
	} vertices;

	vks::Buffer vertexBuffer;
	vks::Buffer indexBuffer;
	uint32_t indexCount;

	struct {
		vks::Buffer view;
		vks::Buffer dynamic;
	} uniformBuffers;

	struct {
		glm::mat4 projection;
		glm::mat4 view;
	} uboVS;

	// Store random per-object rotations
	glm::vec3 rotations[OBJECT_INSTANCES];
	glm::vec3 rotationSpeeds[OBJECT_INSTANCES];

	// One big uniform buffer that contains all matrices
	// Note that we need to manually allocate the data to cope for GPU-specific uniform buffer offset alignments
	struct UboDataDynamic {
		glm::mat4 *model = nullptr;
	} uboDataDynamic;

	VkPipeline pipeline;
	VkPipelineLayout pipelineLayout;
	VkDescriptorSet descriptorSet;
	VkDescriptorSetLayout descriptorSetLayout;

	float animationTimer = 0.0f;

	size_t dynamicAlignment;

    VulkanExample();
    ~VulkanExample();

    void buildCommandBuffers();

    void draw();

    void generateCube();

    void setupVertexDescriptions();

    void setupDescriptorPool();

    void setupDescriptorSetLayout();

    void setupDescriptorSet();

    void preparePipelines();

    // Prepare and initialize uniform buffer containing shader uniforms
    void prepareUniformBuffers();

    void updateUniformBuffers();

    void updateDynamicUniformBuffer(bool force = false);

    void prepare();

    virtual void render();

    virtual void viewChanged();

#ifdef UNITY_BUILD
    virtual void ProcessDeviceEvent(UnityGfxDeviceEventType type, IUnityInterfaces* interfaces);
private:
    void handleUnityGfxDeviceEventInitialize(IUnityInterfaces* interfaces);
    void handleUnityGfxDeviceEventShutdown();
#endif

};
