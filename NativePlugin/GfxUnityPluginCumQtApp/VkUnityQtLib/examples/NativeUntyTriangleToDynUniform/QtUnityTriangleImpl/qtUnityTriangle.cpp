#include "qtUnityTriangle.h"

#include "../../VkUnityQtLib/data/shaders/triangle/triangleShaders.h"

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

VulkanExample::VulkanExample()
    : QtUIVulkanExample()
{
    title = "Test Triangle";
    camera.type = Camera::CameraType::lookat;
    camera.setPosition(glm::vec3(0.0f, 0.0f, -2.5f));
    camera.setRotation(glm::vec3(0.0f));
    camera.setPerspective(60.0f, (float)width / (float)height, 1.0f, 256.0f);
}

VulkanExample::~VulkanExample()
{
    vkDestroyPipeline(device, pipeline, nullptr);
    vkDestroyBuffer(device, vertexBuffer.buffer, nullptr);
    vkFreeMemory(device, vertexBuffer.memory, nullptr);
}

void VulkanExample::prepareVertices()
{
    // Setup vertices
    std::vector<Vertex> vertexCoords =
    {
        { {  1.0f,  1.0f, 0.0f }, { 1.0f, 0.0f, 0.0f } },
        { { -1.0f,  1.0f, 0.0f }, { 0.0f, 1.0f, 0.0f } },
        { {  0.0f, -1.0f, 0.0f }, { 1.0f, 0.0f, 1.0f } }
    };

    VK_CHECK_RESULT(vulkanDevice->createBuffer(
                        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                        &vertexBuffer,
                        vertexCoords.size() * sizeof(Vertex),
                        vertexCoords.data()));
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

    shaderStages[0] = loadShader("/media/parminder/Data/Dev/GiraphicsRepo/Unity/NativePlugin/GfxUnityPluginCumQtApp/VkUnityQtLib/data/shaders/triangle/triangle.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
    shaderStages[1] = loadShader("/media/parminder/Data/Dev/GiraphicsRepo/Unity/NativePlugin/GfxUnityPluginCumQtApp/VkUnityQtLib/data/shaders/triangle/triangle.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);

    VkGraphicsPipelineCreateInfo pipelineCreateInfo =
            vks::initializers::pipelineCreateInfo(
                VK_NULL_HANDLE,
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

void VulkanExample::paint(VkCommandBuffer commandBuffer)
{
    const VkDeviceSize offset = 0;
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, &vertexBuffer.buffer, &offset);
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
    vkCmdDraw(commandBuffer, 3, 1, 0, 0);
}

void VulkanExample::prepare()
{
#ifdef UNITY_BUILD
    createPipelineCache(); // Pipeline cache object for unity
    prepareVertices();
    setupVertexDescriptions();
    preparePipelines();
#else
    VulkanExampleBase::prepare();
    prepareVertices();
    setupVertexDescriptions();
    preparePipelines();
    buildCommandBuffers();
#endif
    prepared = true;
}

void VulkanExample::buildCommandBuffers()
{
    VkCommandBufferBeginInfo cmdBufInfo = {};
    cmdBufInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    cmdBufInfo.pNext = nullptr;

    // Set clear values for all framebuffer attachments with loadOp set to clear
    // We use two attachments (color and depth) that are cleared at the start of the subpass and as such we need to set clear values for both
    VkClearValue clearValues[2];
    clearValues[0].color = { { 0.0f, 0.0f, 0.2f, 1.0f } };
    clearValues[1].depthStencil = { 1.0f, 0 };

    VkRenderPassBeginInfo renderPassBeginInfo = {};
    renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassBeginInfo.pNext = nullptr;
    renderPassBeginInfo.renderPass = renderPass;
    renderPassBeginInfo.renderArea.offset.x = 0;
    renderPassBeginInfo.renderArea.offset.y = 0;
    renderPassBeginInfo.renderArea.extent.width = width;
    renderPassBeginInfo.renderArea.extent.height = height;
    renderPassBeginInfo.clearValueCount = 2;
    renderPassBeginInfo.pClearValues = clearValues;

    for (int32_t i = 0; i < drawCmdBuffers.size(); ++i)
    {
        // Set target frame buffer
        renderPassBeginInfo.framebuffer = frameBuffers[i];

        VK_CHECK_RESULT(vkBeginCommandBuffer(drawCmdBuffers[i], &cmdBufInfo));

        // Start the first sub pass specified in our default render pass setup by the base class
        // This will clear the color and depth attachment
        //vkCmdBeginRenderPass(drawCmdBuffers[i], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
        Hook_vkCmdBeginRenderPass(drawCmdBuffers[i], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
        // Update dynamic viewport state
        VkViewport viewport = {};
        viewport.height = (float)height;
        viewport.width = (float)width;
        viewport.minDepth = (float) 0.0f;
        viewport.maxDepth = (float) 1.0f;
        vkCmdSetViewport(drawCmdBuffers[i], 0, 1, &viewport);

        // Update dynamic scissor state
        VkRect2D scissor = {};
        scissor.extent.width = width;
        scissor.extent.height = height;
        scissor.offset.x = 0;
        scissor.offset.y = 0;
        vkCmdSetScissor(drawCmdBuffers[i], 0, 1, &scissor);

        paint(drawCmdBuffers[i]);

        vkCmdEndRenderPass(drawCmdBuffers[i]);

        // Ending the render pass will add an implicit barrier transitioning the frame buffer color attachment to
        // VK_IMAGE_LAYOUT_PRESENT_SRC_KHR for presenting it to the windowing system

        VK_CHECK_RESULT(vkEndCommandBuffer(drawCmdBuffers[i]));
    }
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

        vkDestroyBuffer(m_Instance.device, vertexBuffer.buffer, NULL);
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
}
#endif

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
    buildCommandBuffers();
    draw(); // Swapchain scheduled for presentation
#endif
}
