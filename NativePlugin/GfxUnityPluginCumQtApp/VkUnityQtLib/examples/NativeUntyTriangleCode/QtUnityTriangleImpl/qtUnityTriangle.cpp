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
    vkDestroyBuffer(device, vertices.buffer, nullptr);
    vkFreeMemory(device, vertices.memory, nullptr);
}

void VulkanExample::prepareVertices()
{
    // Setup vertices
    std::vector<Vertex> vertexBuffer =
    {
        { {  1.0f,  1.0f, 0.0f }, { 1.0f, 0.0f, 0.0f } },
        { { -1.0f,  1.0f, 0.0f }, { 1.0f, 0.0f, 0.0f } },
        { {  0.0f, -1.0f, 0.0f }, { 0.0f, 0.0f, 1.0f } }
    };
    uint32_t vertexBufferSize = static_cast<uint32_t>(vertexBuffer.size()) * sizeof(Vertex);

    VkMemoryAllocateInfo memAlloc = {};
    memAlloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    VkMemoryRequirements memReqs;

    void *data;
    VkBufferCreateInfo vertexBufferInfo = {};
    vertexBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    vertexBufferInfo.size = vertexBufferSize;
    vertexBufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;

    // Copy vertex data to a buffer visible to the host
    VK_CHECK_RESULT(vkCreateBuffer(device, &vertexBufferInfo, nullptr, &vertices.buffer));
    vkGetBufferMemoryRequirements(device, vertices.buffer, &memReqs);
    memAlloc.allocationSize = memReqs.size;
    // VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT is host visible memory, and VK_MEMORY_PROPERTY_HOST_COHERENT_BIT makes sure writes are directly visible
    memAlloc.memoryTypeIndex = getMemoryTypeIndex(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    VK_CHECK_RESULT(vkAllocateMemory(device, &memAlloc, nullptr, &vertices.memory));
    VK_CHECK_RESULT(vkMapMemory(device, vertices.memory, 0, memAlloc.allocationSize, 0, &data));
    memcpy(data, vertexBuffer.data(), vertexBufferSize);
    vkUnmapMemory(device, vertices.memory);
    VK_CHECK_RESULT(vkBindBufferMemory(device, vertices.buffer, vertices.memory, 0));
}

VkShaderModule VulkanExample::loadSPIRVShader(std::string filename)
{
    size_t shaderSize;
    char* shaderCode = NULL;

    std::ifstream is(filename, std::ios::binary | std::ios::in | std::ios::ate);

    if (is.is_open())
    {
        shaderSize = is.tellg();
        is.seekg(0, std::ios::beg);
        // Copy file contents into a buffer
        shaderCode = new char[shaderSize];
        is.read(shaderCode, shaderSize);
        is.close();
        assert(shaderSize > 0);
    }

    if (shaderCode)
    {
        // Create a new shader module that will be used for pipeline creation
        VkShaderModuleCreateInfo moduleCreateInfo{};
        moduleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        moduleCreateInfo.codeSize = shaderSize;
        moduleCreateInfo.pCode = (uint32_t*)shaderCode;

        VkShaderModule shaderModule;
        VK_CHECK_RESULT(vkCreateShaderModule(device, &moduleCreateInfo, NULL, &shaderModule));

        delete[] shaderCode;

        return shaderModule;
    }
    else
    {
        std::cerr << "Error: Could not open shader file \"" << filename << "\"" << std::endl;
        return VK_NULL_HANDLE;
    }
}

VkShaderModule VulkanExample::loadSPIRVShader(const uint32_t *pCode, size_t codeSize)
{
    VkShaderModuleCreateInfo moduleCreateInfo{};
    moduleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    moduleCreateInfo.codeSize = codeSize;
    moduleCreateInfo.pCode = pCode;

    VkShaderModule shaderModule;
    VK_CHECK_RESULT(vkCreateShaderModule(device, &moduleCreateInfo, NULL, &shaderModule));

    return shaderModule;
}

void VulkanExample::preparePipelines()
{
    // Create the graphics pipeline used in this example
    // Vulkan uses the concept of rendering pipelines to encapsulate fixed states, replacing OpenGL's complex state machine
    // A pipeline is then stored and hashed on the GPU making pipeline changes very fast
    // Note: There are still a few dynamic states that are not directly part of the pipeline (but the info that they are used is)

    VkGraphicsPipelineCreateInfo pipelineCreateInfo = {};
    pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    // The layout used for this pipeline (can be shared among multiple pipelines using the same layout)
    // Renderpass this pipeline is attached to
    pipelineCreateInfo.renderPass = renderPass;

    // Construct the differnent states making up the pipeline

    // Input assembly state describes how primitives are assembled
    // This pipeline will assemble vertex data as a triangle lists (though we only use one triangle)
    VkPipelineInputAssemblyStateCreateInfo inputAssemblyState = {};
    inputAssemblyState.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssemblyState.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

    // Rasterization state
    VkPipelineRasterizationStateCreateInfo rasterizationState = {};
    rasterizationState.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizationState.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizationState.cullMode = VK_CULL_MODE_NONE;
    rasterizationState.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizationState.depthClampEnable = VK_FALSE;
    rasterizationState.rasterizerDiscardEnable = VK_FALSE;
    rasterizationState.depthBiasEnable = VK_FALSE;
    rasterizationState.lineWidth = 1.0f;

    // Color blend state describes how blend factors are calculated (if used)
    // We need one blend attachment state per color attachment (even if blending is not used
    VkPipelineColorBlendAttachmentState blendAttachmentState[1] = {};
    blendAttachmentState[0].colorWriteMask = 0xf;
    blendAttachmentState[0].blendEnable = VK_FALSE;
    VkPipelineColorBlendStateCreateInfo colorBlendState = {};
    colorBlendState.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlendState.attachmentCount = 1;
    colorBlendState.pAttachments = blendAttachmentState;

    // Viewport state sets the number of viewports and scissor used in this pipeline
    // Note: This is actually overriden by the dynamic states (see below)
    VkPipelineViewportStateCreateInfo viewportState = {};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.scissorCount = 1;

    // Enable dynamic states
    // Most states are baked into the pipeline, but there are still a few dynamic states that can be changed within a command buffer
    // To be able to change these we need do specify which dynamic states will be changed using this pipeline. Their actual states are set later on in the command buffer.
    // For this example we will set the viewport and scissor using dynamic states
    std::vector<VkDynamicState> dynamicStateEnables;
    dynamicStateEnables.push_back(VK_DYNAMIC_STATE_VIEWPORT);
    dynamicStateEnables.push_back(VK_DYNAMIC_STATE_SCISSOR);
    VkPipelineDynamicStateCreateInfo dynamicState = {};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.pDynamicStates = dynamicStateEnables.data();
    dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStateEnables.size());

    // Depth and stencil state containing depth and stencil compare and test operations
    // We only use depth tests and want depth tests and writes to be enabled and compare with less or equal
    VkPipelineDepthStencilStateCreateInfo depthStencilState = {};
    depthStencilState.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencilState.depthTestEnable = VK_TRUE;
    depthStencilState.depthWriteEnable = VK_TRUE;
    depthStencilState.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
    depthStencilState.depthBoundsTestEnable = VK_FALSE;
    depthStencilState.back.failOp = VK_STENCIL_OP_KEEP;
    depthStencilState.back.passOp = VK_STENCIL_OP_KEEP;
    depthStencilState.back.compareOp = VK_COMPARE_OP_ALWAYS;
    depthStencilState.stencilTestEnable = VK_FALSE;
    depthStencilState.front = depthStencilState.back;

    // Multi sampling state
    // This example does not make use fo multi sampling (for anti-aliasing), the state must still be set and passed to the pipeline
    VkPipelineMultisampleStateCreateInfo multisampleState = {};
    multisampleState.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampleState.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisampleState.pSampleMask = nullptr;

    // Vertex input descriptions
    // Specifies the vertex input parameters for a pipeline

    // Vertex input binding
    // This example uses a single vertex input binding at binding point 0 (see vkCmdBindVertexBuffers)
    VkVertexInputBindingDescription vertexInputBinding = {};
    vertexInputBinding.binding = 0;
    vertexInputBinding.stride = sizeof(Vertex);
    vertexInputBinding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    // Inpute attribute bindings describe shader attribute locations and memory layouts
    std::array<VkVertexInputAttributeDescription, 2> vertexInputAttributs;
    // These match the following shader layout (see triangle.vert):
    //	layout (location = 0) in vec3 inPos;
    //	layout (location = 1) in vec3 inColor;
    // Attribute location 0: Position
    vertexInputAttributs[0].binding = 0;
    vertexInputAttributs[0].location = 0;
    // Position attribute is three 32 bit signed (SFLOAT) floats (R32 G32 B32)
    vertexInputAttributs[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    vertexInputAttributs[0].offset = offsetof(Vertex, position);
    // Attribute location 1: Color
    vertexInputAttributs[1].binding = 0;
    vertexInputAttributs[1].location = 1;
    // Color attribute is three 32 bit signed (SFLOAT) floats (R32 G32 B32)
    vertexInputAttributs[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    vertexInputAttributs[1].offset = offsetof(Vertex, color);

    // Vertex input state used for pipeline creation
    VkPipelineVertexInputStateCreateInfo vertexInputState = {};
    vertexInputState.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputState.vertexBindingDescriptionCount = 1;
    vertexInputState.pVertexBindingDescriptions = &vertexInputBinding;
    vertexInputState.vertexAttributeDescriptionCount = 2;
    vertexInputState.pVertexAttributeDescriptions = vertexInputAttributs.data();

    // Shaders
    std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages{};

    // Vertex shader
    shaderStages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    // Set pipeline stage for this shader
    shaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
    // Load binary SPIR-V shader
    //shaderStages[0].module = loadSPIRVShader(getAssetPath() + "shaders/triangle/triangle.vert.spv");
    shaderStages[0].module = loadSPIRVShader(vertexShaderSpirv, sizeof(vertexShaderSpirv));
    // Main entry point for the shader
    shaderStages[0].pName = "main";
    assert(shaderStages[0].module != VK_NULL_HANDLE);

    // Fragment shader
    shaderStages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    // Set pipeline stage for this shader
    shaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    // Load binary SPIR-V shader
    //        shaderStages[1].module = loadSPIRVShader(getAssetPath() + "shaders/triangle/triangle.frag.spv");
    shaderStages[1].module = loadSPIRVShader(fragmentShaderSpirv, sizeof(fragmentShaderSpirv));
    // Main entry point for the shader
    shaderStages[1].pName = "main";
    assert(shaderStages[1].module != VK_NULL_HANDLE);

    // Set pipeline shader stage info
    pipelineCreateInfo.stageCount = static_cast<uint32_t>(shaderStages.size());
    pipelineCreateInfo.pStages = shaderStages.data();

    // Assign the pipeline states to the pipeline creation info structure
    pipelineCreateInfo.pVertexInputState = &vertexInputState;
    pipelineCreateInfo.pInputAssemblyState = &inputAssemblyState;
    pipelineCreateInfo.pRasterizationState = &rasterizationState;
    pipelineCreateInfo.pColorBlendState = &colorBlendState;
    pipelineCreateInfo.pMultisampleState = &multisampleState;
    pipelineCreateInfo.pViewportState = &viewportState;
    pipelineCreateInfo.pDepthStencilState = &depthStencilState;
    pipelineCreateInfo.renderPass = renderPass;
    pipelineCreateInfo.pDynamicState = &dynamicState;

    // Create rendering pipeline using the specified states
    VK_CHECK_RESULT(vkCreateGraphicsPipelines(device, pipelineCache, 1, &pipelineCreateInfo, nullptr, &pipeline));

    // Shader modules are no longer needed once the graphics pipeline has been created
    vkDestroyShaderModule(device, shaderStages[0].module, nullptr);
    vkDestroyShaderModule(device, shaderStages[1].module, nullptr);
}

void VulkanExample::paint(VkCommandBuffer commandBuffer)
{
    const VkDeviceSize offset = 0;
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, &vertices.buffer, &offset);
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
    vkCmdDraw(commandBuffer, 3, 1, 0, 0);
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

void VulkanExample::prepare()
{
#ifdef UNITY_BUILD
    createPipelineCache(); // Pipeline cache object for unity
#else
    VulkanExampleBase::prepare();
    prepareSynchronizationPrimitives();

    prepareVertices();
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
