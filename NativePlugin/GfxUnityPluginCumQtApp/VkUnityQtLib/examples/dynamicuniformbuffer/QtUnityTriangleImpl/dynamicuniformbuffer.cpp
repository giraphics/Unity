#include "dynamicuniformbuffer.h"


// Wrapper functions for aligned memory allocation
// There is currently no standard for this in C++ that works across all platforms and vendors, so we abstract this
void* alignedAlloc(size_t size, size_t alignment)
{
    void *data = nullptr;
#if defined(_MSC_VER) || defined(__MINGW32__)
    data = _aligned_malloc(size, alignment);
#else
    int res = posix_memalign(&data, alignment, size);
    if (res != 0)
        data = nullptr;
#endif
    return data;
}

void alignedFree(void* data)
{
#if	defined(_MSC_VER) || defined(__MINGW32__)
    _aligned_free(data);
#else
    free(data);
#endif
}

VulkanExample::VulkanExample() : VulkanExampleBase(ENABLE_VALIDATION)
{
    title = "Dynamic uniform buffers";
    camera.type = Camera::CameraType::lookat;
    camera.setPosition(glm::vec3(0.0f, 0.0f, -30.0f));
    camera.setRotation(glm::vec3(0.0f));
    camera.setPerspective(60.0f, (float)width / (float)height, 0.1f, 256.0f);
    settings.overlay = true;
}

VulkanExample::~VulkanExample()
{
    if (uboDataDynamic.model) {
        alignedFree(uboDataDynamic.model);
    }

    // Clean up used Vulkan resources
    // Note : Inherited destructor cleans up resources stored in base class
    vkDestroyPipeline(device, pipeline, nullptr);

    vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
    vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);

    vertexBuffer.destroy();
    indexBuffer.destroy();

    uniformBuffers.view.destroy();
    uniformBuffers.dynamic.destroy();
}

void VulkanExample::draw()
{
    VulkanExampleBase::prepareFrame();

    // Command buffer to be sumitted to the queue
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &drawCmdBuffers[currentBuffer];

    // Submit to queue
    VK_CHECK_RESULT(vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE));

    VulkanExampleBase::submitFrame();
}

void VulkanExample::generateCube()
{
    // Setup vertices indices for a colored cube
    std::vector<Vertex> vertices = {
        { { -1.0f, -1.0f,  1.0f },{ 1.0f, 0.0f, 0.0f } },
        { {  1.0f, -1.0f,  1.0f },{ 0.0f, 1.0f, 0.0f } },
        { {  1.0f,  1.0f,  1.0f },{ 0.0f, 0.0f, 1.0f } },
        { { -1.0f,  1.0f,  1.0f },{ 0.0f, 0.0f, 0.0f } },
        { { -1.0f, -1.0f, -1.0f },{ 1.0f, 0.0f, 0.0f } },
        { {  1.0f, -1.0f, -1.0f },{ 0.0f, 1.0f, 0.0f } },
        { {  1.0f,  1.0f, -1.0f },{ 0.0f, 0.0f, 1.0f } },
        { { -1.0f,  1.0f, -1.0f },{ 0.0f, 0.0f, 0.0f } },
    };

    std::vector<uint32_t> indices = {
        0,1,2, 2,3,0, 1,5,6, 6,2,1, 7,6,5, 5,4,7, 4,0,3, 3,7,4, 4,5,1, 1,0,4, 3,2,6, 6,7,3,
    };

    indexCount = static_cast<uint32_t>(indices.size());

    // Create buffers
    // For the sake of simplicity we won't stage the vertex data to the gpu memory
    // Vertex buffer
    VK_CHECK_RESULT(vulkanDevice->createBuffer(
                        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                        &vertexBuffer,
                        vertices.size() * sizeof(Vertex),
                        vertices.data()));
    // Index buffer
    VK_CHECK_RESULT(vulkanDevice->createBuffer(
                        VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                        &indexBuffer,
                        indices.size() * sizeof(uint32_t),
                        indices.data()));
}

void VulkanExample::setupVertexDescriptions()
{
    // Binding description
    vertices.bindingDescriptions = {
        vks::initializers::vertexInputBindingDescription(VERTEX_BUFFER_BIND_ID, sizeof(Vertex), VK_VERTEX_INPUT_RATE_VERTEX),
    };

    // Attribute descriptions
    vertices.attributeDescriptions = {
        vks::initializers::vertexInputAttributeDescription(VERTEX_BUFFER_BIND_ID, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, pos)),	// Location 0 : Position
        vks::initializers::vertexInputAttributeDescription(VERTEX_BUFFER_BIND_ID, 1, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, color)),	// Location 1 : Color
    };

    vertices.inputState = vks::initializers::pipelineVertexInputStateCreateInfo();
    vertices.inputState.vertexBindingDescriptionCount = static_cast<uint32_t>(vertices.bindingDescriptions.size());
    vertices.inputState.pVertexBindingDescriptions = vertices.bindingDescriptions.data();
    vertices.inputState.vertexAttributeDescriptionCount = static_cast<uint32_t>(vertices.attributeDescriptions.size());
    vertices.inputState.pVertexAttributeDescriptions = vertices.attributeDescriptions.data();
}

void VulkanExample::setupDescriptorPool()
{
    // Example uses one ubo and one image sampler
    std::vector<VkDescriptorPoolSize> poolSizes =
    {
        vks::initializers::descriptorPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1),
        vks::initializers::descriptorPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1),
        vks::initializers::descriptorPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1)
    };

    VkDescriptorPoolCreateInfo descriptorPoolInfo =
            vks::initializers::descriptorPoolCreateInfo(
                static_cast<uint32_t>(poolSizes.size()),
                poolSizes.data(),
                2);

    VK_CHECK_RESULT(vkCreateDescriptorPool(device, &descriptorPoolInfo, nullptr, &descriptorPool));
}

void VulkanExample::setupDescriptorSetLayout()
{
    std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings =
    {
        vks::initializers::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, 0),
        vks::initializers::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, VK_SHADER_STAGE_VERTEX_BIT, 1),
        vks::initializers::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 2)
    };

    VkDescriptorSetLayoutCreateInfo descriptorLayout =
            vks::initializers::descriptorSetLayoutCreateInfo(
                setLayoutBindings.data(),
                static_cast<uint32_t>(setLayoutBindings.size()));

    VK_CHECK_RESULT(vkCreateDescriptorSetLayout(device, &descriptorLayout, nullptr, &descriptorSetLayout));

    VkPipelineLayoutCreateInfo pPipelineLayoutCreateInfo =
            vks::initializers::pipelineLayoutCreateInfo(
                &descriptorSetLayout,
                1);

    VK_CHECK_RESULT(vkCreatePipelineLayout(device, &pPipelineLayoutCreateInfo, nullptr, &pipelineLayout));
}

void VulkanExample::setupDescriptorSet()
{
    VkDescriptorSetAllocateInfo allocInfo =
            vks::initializers::descriptorSetAllocateInfo(
                descriptorPool,
                &descriptorSetLayout,
                1);

    VK_CHECK_RESULT(vkAllocateDescriptorSets(device, &allocInfo, &descriptorSet));

    std::vector<VkWriteDescriptorSet> writeDescriptorSets = {
        // Binding 0 : Projection/View matrix uniform buffer
        vks::initializers::writeDescriptorSet(descriptorSet, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 0, &uniformBuffers.view.descriptor),
        // Binding 1 : Instance matrix as dynamic uniform buffer
        vks::initializers::writeDescriptorSet(descriptorSet, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1, &uniformBuffers.dynamic.descriptor),
    };

    vkUpdateDescriptorSets(device, static_cast<uint32_t>(writeDescriptorSets.size()), writeDescriptorSets.data(), 0, NULL);
}

void VulkanExample::prepareUniformBuffers()
{
    // Allocate data for the dynamic uniform buffer object
    // We allocate this manually as the alignment of the offset differs between GPUs

    // Calculate required alignment based on minimum device offset alignment
    size_t minUboAlignment = vulkanDevice->properties.limits.minUniformBufferOffsetAlignment;
    dynamicAlignment = sizeof(glm::mat4);
    if (minUboAlignment > 0) {
        dynamicAlignment = (dynamicAlignment + minUboAlignment - 1) & ~(minUboAlignment - 1);
    }

    size_t bufferSize = OBJECT_INSTANCES * dynamicAlignment;

    uboDataDynamic.model = (glm::mat4*)alignedAlloc(bufferSize, dynamicAlignment);
    assert(uboDataDynamic.model);

    std::cout << "minUniformBufferOffsetAlignment = " << minUboAlignment << std::endl;
    std::cout << "dynamicAlignment = " << dynamicAlignment << std::endl;

    // Vertex shader uniform buffer block

    // Static shared uniform buffer object with projection and view matrix
    VK_CHECK_RESULT(vulkanDevice->createBuffer(
                        VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                        &uniformBuffers.view,
                        sizeof(uboVS)));

    // Uniform buffer object with per-object matrices
    VK_CHECK_RESULT(vulkanDevice->createBuffer(
                        VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
                        &uniformBuffers.dynamic,
                        bufferSize));

    // Map persistent
    VK_CHECK_RESULT(uniformBuffers.view.map());
    VK_CHECK_RESULT(uniformBuffers.dynamic.map());

    // Prepare per-object matrices with offsets and random rotations
    std::default_random_engine rndEngine(benchmark.active ? 0 : (unsigned)time(nullptr));
    std::normal_distribution<float> rndDist(-1.0f, 1.0f);
    for (uint32_t i = 0; i < OBJECT_INSTANCES; i++) {
        rotations[i] = glm::vec3(rndDist(rndEngine), rndDist(rndEngine), rndDist(rndEngine)) * 2.0f * (float)M_PI;
        rotationSpeeds[i] = glm::vec3(rndDist(rndEngine), rndDist(rndEngine), rndDist(rndEngine));
    }

    updateUniformBuffers();
    updateDynamicUniformBuffer(true);
}

void VulkanExample::updateUniformBuffers()
{
    // Fixed ubo with projection and view matrices
    uboVS.projection = camera.matrices.perspective;
    uboVS.view = camera.matrices.view;

    memcpy(uniformBuffers.view.mapped, &uboVS, sizeof(uboVS));
}

void VulkanExample::updateDynamicUniformBuffer(bool force)
{
    // Update at max. 60 fps
    animationTimer += frameTimer;
    if ((animationTimer <= 1.0f / 60.0f) && (!force)) {
        return;
    }

    // Dynamic ubo with per-object model matrices indexed by offsets in the command buffer
    uint32_t dim = static_cast<uint32_t>(pow(OBJECT_INSTANCES, (1.0f / 3.0f)));
    glm::vec3 offset(5.0f);

    for (uint32_t x = 0; x < dim; x++)
    {
        for (uint32_t y = 0; y < dim; y++)
        {
            for (uint32_t z = 0; z < dim; z++)
            {
                uint32_t index = x * dim * dim + y * dim + z;

                // Aligned offset
                glm::mat4* modelMat = (glm::mat4*)(((uint64_t)uboDataDynamic.model + (index * dynamicAlignment)));

                // Update rotations
                rotations[index] += animationTimer * rotationSpeeds[index];

                // Update matrices
                glm::vec3 pos = glm::vec3(-((dim * offset.x) / 2.0f) + offset.x / 2.0f + x * offset.x, -((dim * offset.y) / 2.0f) + offset.y / 2.0f + y * offset.y, -((dim * offset.z) / 2.0f) + offset.z / 2.0f + z * offset.z);
                *modelMat = glm::translate(glm::mat4(1.0f), pos);
                *modelMat = glm::rotate(*modelMat, rotations[index].x, glm::vec3(1.0f, 1.0f, 0.0f));
                *modelMat = glm::rotate(*modelMat, rotations[index].y, glm::vec3(0.0f, 1.0f, 0.0f));
                *modelMat = glm::rotate(*modelMat, rotations[index].z, glm::vec3(0.0f, 0.0f, 1.0f));
            }
        }
    }

    animationTimer = 0.0f;

    memcpy(uniformBuffers.dynamic.mapped, uboDataDynamic.model, uniformBuffers.dynamic.size);
    // Flush to make changes visible to the host
    VkMappedMemoryRange memoryRange = vks::initializers::mappedMemoryRange();
    memoryRange.memory = uniformBuffers.dynamic.memory;
    memoryRange.size = uniformBuffers.dynamic.size;
    vkFlushMappedMemoryRanges(device, 1, &memoryRange);
}

void VulkanExample::viewChanged()
{
    updateUniformBuffers();
}

void VulkanExample::paint(VkCommandBuffer commandBuffer)
{
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

    VkDeviceSize offsets[1] = { 0 };
    vkCmdBindVertexBuffers(commandBuffer, VERTEX_BUFFER_BIND_ID, 1, &vertexBuffer.buffer, offsets);
    vkCmdBindIndexBuffer(commandBuffer, indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);

    // Render multiple objects using different model matrices by dynamically offsetting into one uniform buffer
    for (uint32_t j = 0; j < OBJECT_INSTANCES; j++)
    {
        // One dynamic offset per dynamic descriptor to offset into the ubo containing all model matrices
        uint32_t dynamicOffset = j * static_cast<uint32_t>(dynamicAlignment);
        // Bind the descriptor set for rendering a mesh using the dynamic offset
        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSet, 1, &dynamicOffset);

        vkCmdDrawIndexed(commandBuffer, indexCount, 1, 0, 0, 0);
    }

    drawUI(commandBuffer);
}

void VulkanExample::render()
{
    if (!prepared)
        return;

#ifdef UNITY_BUILD
    XXUnityVulkanRecordingState recordingState;
    if ((pipeline == VK_NULL_HANDLE) || !m_UnityVulkan->CommandRecordingState(&recordingState, XXkUnityVulkanGraphicsQueueAccess_DontCare))
        return;

    paint(recordingState.commandBuffer);
#else
    draw();
#endif
    if (!paused)
        updateDynamicUniformBuffer();
}

void VulkanExample::prepare()
{
#ifdef UNITY_BUILD
#else
     VulkanExampleBase::prepare();
    generateCube();
    setupVertexDescriptions();
    prepareUniformBuffers();
    setupDescriptorSetLayout();
    preparePipelines();
    setupDescriptorPool();
    setupDescriptorSet();
    buildCommandBuffers();
#endif
    prepared = true;
}

void VulkanExample::buildCommandBuffers()
{
    VkCommandBufferBeginInfo cmdBufInfo = vks::initializers::commandBufferBeginInfo();

    VkClearValue clearValues[2];
    clearValues[0].color = defaultClearColor;
    clearValues[1].depthStencil = { 1.0f, 0 };

    VkRenderPassBeginInfo renderPassBeginInfo = vks::initializers::renderPassBeginInfo();
    renderPassBeginInfo.renderPass = renderPass;
    renderPassBeginInfo.renderArea.offset.x = 0;
    renderPassBeginInfo.renderArea.offset.y = 0;
    renderPassBeginInfo.renderArea.extent.width = width;
    renderPassBeginInfo.renderArea.extent.height = height;
    renderPassBeginInfo.clearValueCount = 2;
    renderPassBeginInfo.pClearValues = clearValues;

    for (int32_t i = 0; i < drawCmdBuffers.size(); ++i)
    {
        renderPassBeginInfo.framebuffer = frameBuffers[i];

        VK_CHECK_RESULT(vkBeginCommandBuffer(drawCmdBuffers[i], &cmdBufInfo));

        vkCmdBeginRenderPass(drawCmdBuffers[i], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

        VkViewport viewport = vks::initializers::viewport((float)width, (float)height, 0.0f, 1.0f);
        vkCmdSetViewport(drawCmdBuffers[i], 0, 1, &viewport);

        VkRect2D scissor = vks::initializers::rect2D(width, height, 0, 0);
        vkCmdSetScissor(drawCmdBuffers[i], 0, 1, &scissor);

        paint(drawCmdBuffers[i]);

        vkCmdEndRenderPass(drawCmdBuffers[i]);

        VK_CHECK_RESULT(vkEndCommandBuffer(drawCmdBuffers[i]));
    }
}

void VulkanExample::preparePipelines()
{
    VkPipelineInputAssemblyStateCreateInfo inputAssemblyState =
            vks::initializers::pipelineInputAssemblyStateCreateInfo(
                VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
                0,
                VK_FALSE);

    VkPipelineRasterizationStateCreateInfo rasterizationState =
            vks::initializers::pipelineRasterizationStateCreateInfo(
                VK_POLYGON_MODE_FILL,
                VK_CULL_MODE_NONE,
                VK_FRONT_FACE_COUNTER_CLOCKWISE,
                0);

    VkPipelineColorBlendAttachmentState blendAttachmentState =
            vks::initializers::pipelineColorBlendAttachmentState(
                0xf,
                VK_FALSE);

    VkPipelineColorBlendStateCreateInfo colorBlendState =
            vks::initializers::pipelineColorBlendStateCreateInfo(
                1,
                &blendAttachmentState);

    VkPipelineDepthStencilStateCreateInfo depthStencilState =
            vks::initializers::pipelineDepthStencilStateCreateInfo(
                VK_TRUE,
                VK_TRUE,
                VK_COMPARE_OP_LESS_OR_EQUAL);

    VkPipelineViewportStateCreateInfo viewportState =
            vks::initializers::pipelineViewportStateCreateInfo(1, 1, 0);

    VkPipelineMultisampleStateCreateInfo multisampleState =
            vks::initializers::pipelineMultisampleStateCreateInfo(
                VK_SAMPLE_COUNT_1_BIT,
                0);

    std::vector<VkDynamicState> dynamicStateEnables = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR
    };
    VkPipelineDynamicStateCreateInfo dynamicState =
            vks::initializers::pipelineDynamicStateCreateInfo(
                dynamicStateEnables.data(),
                static_cast<uint32_t>(dynamicStateEnables.size()),
                0);

    // Load shaders
    std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages;

    shaderStages[0] = loadShader(getAssetPath() + "shaders/dynamicuniformbuffer/base.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
    shaderStages[1] = loadShader(getAssetPath() + "shaders/dynamicuniformbuffer/base.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);

    VkGraphicsPipelineCreateInfo pipelineCreateInfo =
            vks::initializers::pipelineCreateInfo(
                pipelineLayout,
                renderPass,
                0);

    pipelineCreateInfo.pVertexInputState = &vertices.inputState;
    pipelineCreateInfo.pInputAssemblyState = &inputAssemblyState;
    pipelineCreateInfo.pRasterizationState = &rasterizationState;
    pipelineCreateInfo.pColorBlendState = &colorBlendState;
    pipelineCreateInfo.pMultisampleState = &multisampleState;
    pipelineCreateInfo.pViewportState = &viewportState;
    pipelineCreateInfo.pDepthStencilState = &depthStencilState;
    pipelineCreateInfo.pDynamicState = &dynamicState;
    pipelineCreateInfo.stageCount = static_cast<uint32_t>(shaderStages.size());
    pipelineCreateInfo.pStages = shaderStages.data();

    VK_CHECK_RESULT(vkCreateGraphicsPipelines(device, pipelineCache, 1, &pipelineCreateInfo, nullptr, &pipeline));
}

#ifdef UNITY_BUILD
void VulkanExample::ProcessDeviceEvent(UnityGfxDeviceEventType type, IUnityInterfaces* interfaces)
{
    switch (type)
    {
    case kUnityGfxDeviceEventInitialize:
        handleUnityGfxDeviceEventInitialize(interfaces);
        break;

    case kUnityGfxDeviceEventShutdown:
        handleUnityGfxDeviceEventShutdown();
        break;

    case kUnityGfxDeviceEventBeforeReset:
        break;

    case kUnityGfxDeviceEventAfterReset:
        break;
    }
}

void VulkanExample::handleUnityGfxDeviceEventShutdown()
{
    if (m_Instance.device != VK_NULL_HANDLE)
    {
        //GarbageCollect(true);
        if (pipeline != VK_NULL_HANDLE)
        {
            vkDestroyPipeline(m_Instance.device, pipeline, NULL);
            pipeline = VK_NULL_HANDLE;
        }

        vkDestroyBuffer(m_Instance.device, vertices.buffer, NULL);
    }

    m_UnityVulkan = NULL;
    renderPass = VK_NULL_HANDLE;
    m_Instance = XXUnityVulkanInstance();
}

void VulkanExample::handleUnityGfxDeviceEventInitialize(IUnityInterfaces* interfaces)
{
    m_UnityVulkan = interfaces->Get<IUnityGraphicsVulkan>();
    m_Instance = m_UnityVulkan->Instance();

    // Make sure Vulkan API functions are loaded
    //LoadVulkanAPI(m_Instance.getInstanceProcAddr, m_Instance.instance);

    XXUnityVulkanPluginEventConfig config_1;
    config_1.graphicsQueueAccess = XXkUnityVulkanGraphicsQueueAccess_DontCare;
    config_1.renderPassPrecondition = XXkUnityVulkanRenderPass_EnsureInside;
    config_1.flags = XXkUnityVulkanEventConfigFlag_EnsurePreviousFrameSubmission | XXkUnityVulkanEventConfigFlag_ModifiesCommandBuffersState;
    m_UnityVulkan->ConfigureEvent(1, &config_1);

    // alternative way to intercept API
    m_UnityVulkan->InterceptVulkanAPI("vkCmdBeginRenderPass", (PFN_vkVoidFunction)Hook_vkCmdBeginRenderPass);

    /**********************************************************/
    /*            Graphics intialization goes here            */
    /**********************************************************/
    // 1. Map external GPU objects to our graphics sub-system
    mapExternalObjectToGraphicsSubSystem();

    // Do the preperation
    prepare();

    // Prepare the vertices
    prepareVertices();
    preparePipelines();
}
#endif
