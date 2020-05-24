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

#include "../../data/shaders/triangle/triangleShaders.h"
#ifdef UNITY_BUILD
#include "Unity/IUnityGraphicsVulkan.h"
#endif

// Set to "true" to enable Vulkan's validation layers (see vulkandebug.cpp for details)
#define ENABLE_VALIDATION false
class QtUIVulkanExample : public VulkanExampleBase
{
public:
    VkSemaphore presentCompleteSemaphore;
    VkSemaphore renderCompleteSemaphore;
    std::vector<VkFence> waitFences;

    QtUIVulkanExample() : VulkanExampleBase(ENABLE_VALIDATION)
    {
    }

    virtual ~QtUIVulkanExample()
    {
        vkDestroySemaphore(device, presentCompleteSemaphore, nullptr);
        vkDestroySemaphore(device, renderCompleteSemaphore, nullptr);

        for (auto& fence : waitFences)
        {
            vkDestroyFence(device, fence, nullptr);
        }
    }

    void draw()
    {
        // Get next image in the swap chain (back/front buffer)
        VK_CHECK_RESULT(swapChain.acquireNextImage(presentCompleteSemaphore, &currentBuffer));

        // Use a fence to wait until the command buffer has finished execution before using it again
        VK_CHECK_RESULT(vkWaitForFences(device, 1, &waitFences[currentBuffer], VK_TRUE, UINT64_MAX));
        VK_CHECK_RESULT(vkResetFences(device, 1, &waitFences[currentBuffer]));

        // Pipeline stage at which the queue submission will wait (via pWaitSemaphores)
        VkPipelineStageFlags waitStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        // The submit info structure specifices a command buffer queue submission batch
        VkSubmitInfo submitInfo = {};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.pWaitDstStageMask = &waitStageMask;               // Pointer to the list of pipeline stages that the semaphore waits will occur at
        submitInfo.pWaitSemaphores = &presentCompleteSemaphore;      // Semaphore(s) to wait upon before the submitted command buffer starts executing
        submitInfo.waitSemaphoreCount = 1;                           // One wait semaphore
        submitInfo.pSignalSemaphores = &renderCompleteSemaphore;     // Semaphore(s) to be signaled when command buffers have completed
        submitInfo.signalSemaphoreCount = 1;                         // One signal semaphore
        submitInfo.pCommandBuffers = &drawCmdBuffers[currentBuffer]; // Command buffers(s) to execute in this batch (submission)
        submitInfo.commandBufferCount = 1;                           // One command buffer

        // Submit to the graphics queue passing a wait fence
        VK_CHECK_RESULT(vkQueueSubmit(queue, 1, &submitInfo, waitFences[currentBuffer]));

        // Present the current buffer to the swap chain
        // Pass the semaphore signaled by the command buffer submission from the submit info as the wait semaphore for swap chain presentation
        // This ensures that the image is not presented to the windowing system until all commands have been submitted
        VkResult present = swapChain.queuePresent(queue, currentBuffer, renderCompleteSemaphore);
        if (!((present == VK_SUCCESS) || (present == VK_SUBOPTIMAL_KHR))) {
            VK_CHECK_RESULT(present);
        }
    }

    void prepareSynchronizationPrimitives()
    {
        // Semaphores (Used for correct command ordering)
        VkSemaphoreCreateInfo semaphoreCreateInfo = {};
        semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        semaphoreCreateInfo.pNext = nullptr;

        // Semaphore used to ensures that image presentation is complete before starting to submit again
        VK_CHECK_RESULT(vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &presentCompleteSemaphore));

        // Semaphore used to ensures that all commands submitted have been finished before submitting the image to the queue
        VK_CHECK_RESULT(vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &renderCompleteSemaphore));

        // Fences (Used to check draw command buffer completion)
        VkFenceCreateInfo fenceCreateInfo = {};
        fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        // Create in signaled state so we don't wait on first render of each command buffer
        fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
        waitFences.resize(drawCmdBuffers.size());
        for (auto& fence : waitFences)
        {
            VK_CHECK_RESULT(vkCreateFence(device, &fenceCreateInfo, nullptr, &fence));
        }
    }

    VkCommandBuffer getCommandBuffer(bool begin)
    {
        VkCommandBuffer cmdBuffer;

        VkCommandBufferAllocateInfo cmdBufAllocateInfo = {};
        cmdBufAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        cmdBufAllocateInfo.commandPool = cmdPool;
        cmdBufAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        cmdBufAllocateInfo.commandBufferCount = 1;

        VK_CHECK_RESULT(vkAllocateCommandBuffers(device, &cmdBufAllocateInfo, &cmdBuffer));

        // If requested, also start the new command buffer
        if (begin)
        {
            VkCommandBufferBeginInfo cmdBufInfo = vks::initializers::commandBufferBeginInfo();
            VK_CHECK_RESULT(vkBeginCommandBuffer(cmdBuffer, &cmdBufInfo));
        }

        return cmdBuffer;
    }

    void flushCommandBuffer(VkCommandBuffer commandBuffer)
    {
        assert(commandBuffer != VK_NULL_HANDLE);

        VK_CHECK_RESULT(vkEndCommandBuffer(commandBuffer));

        VkSubmitInfo submitInfo = {};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffer;

        // Create fence to ensure that the command buffer has finished executing
        VkFenceCreateInfo fenceCreateInfo = {};
        fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceCreateInfo.flags = 0;
        VkFence fence;
        VK_CHECK_RESULT(vkCreateFence(device, &fenceCreateInfo, nullptr, &fence));

        // Submit to the queue
        VK_CHECK_RESULT(vkQueueSubmit(queue, 1, &submitInfo, fence));
        // Wait for the fence to signal that command buffer has finished executing
        VK_CHECK_RESULT(vkWaitForFences(device, 1, &fence, VK_TRUE, DEFAULT_FENCE_TIMEOUT));

        vkDestroyFence(device, fence, nullptr);
        vkFreeCommandBuffers(device, cmdPool, 1, &commandBuffer);
    }

    uint32_t getMemoryTypeIndex(uint32_t typeBits, VkMemoryPropertyFlags properties)
    {
        // Iterate over all memory types available for the device used in this example
        for (uint32_t i = 0; i < deviceMemoryProperties.memoryTypeCount; i++)
        {
            if ((typeBits & 1) == 1)
            {
                if ((deviceMemoryProperties.memoryTypes[i].propertyFlags & properties) == properties)
                {
                    return i;
                }
            }
            typeBits >>= 1;
        }

        throw "Could not find a suitable memory type!";
    }

    void setupDepthStencil() override
    {
        // Create an optimal image used as the depth stencil attachment
        VkImageCreateInfo image = {};
        image.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        image.imageType = VK_IMAGE_TYPE_2D;
        image.format = depthFormat;
        // Use example's height and width
        image.extent = { width, height, 1 };
        image.mipLevels = 1;
        image.arrayLayers = 1;
        image.samples = VK_SAMPLE_COUNT_1_BIT;
        image.tiling = VK_IMAGE_TILING_OPTIMAL;
        image.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
        image.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        VK_CHECK_RESULT(vkCreateImage(device, &image, nullptr, &depthStencil.image));

        // Allocate memory for the image (device local) and bind it to our image
        VkMemoryAllocateInfo memAlloc = {};
        memAlloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        VkMemoryRequirements memReqs;
        vkGetImageMemoryRequirements(device, depthStencil.image, &memReqs);
        memAlloc.allocationSize = memReqs.size;
        memAlloc.memoryTypeIndex = getMemoryTypeIndex(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        VK_CHECK_RESULT(vkAllocateMemory(device, &memAlloc, nullptr, &depthStencil.mem));
        VK_CHECK_RESULT(vkBindImageMemory(device, depthStencil.image, depthStencil.mem, 0));

        // Create a view for the depth stencil image
        // Images aren't directly accessed in Vulkan, but rather through views described by a subresource range
        // This allows for multiple views of one image with differing ranges (e.g. for different layers)
        VkImageViewCreateInfo depthStencilView = {};
        depthStencilView.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        depthStencilView.viewType = VK_IMAGE_VIEW_TYPE_2D;
        depthStencilView.format = depthFormat;
        depthStencilView.subresourceRange = {};
        depthStencilView.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
        depthStencilView.subresourceRange.baseMipLevel = 0;
        depthStencilView.subresourceRange.levelCount = 1;
        depthStencilView.subresourceRange.baseArrayLayer = 0;
        depthStencilView.subresourceRange.layerCount = 1;
        depthStencilView.image = depthStencil.image;
        VK_CHECK_RESULT(vkCreateImageView(device, &depthStencilView, nullptr, &depthStencil.view));
    }

    void setupFrameBuffer() override
    {
        // Create a frame buffer for every image in the swapchain
        frameBuffers.resize(swapChain.imageCount);
        for (size_t i = 0; i < frameBuffers.size(); i++)
        {
            std::array<VkImageView, 2> attachments;
            attachments[0] = swapChain.buffers[i].view; // Color attachment is the view of the swapchain image
            attachments[1] = depthStencil.view;         // Depth/Stencil attachment is the same for all frame buffers

            VkFramebufferCreateInfo frameBufferCreateInfo = {};
            frameBufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            // All frame buffers use the same renderpass setup
            frameBufferCreateInfo.renderPass = renderPass;
            frameBufferCreateInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
            frameBufferCreateInfo.pAttachments = attachments.data();
            frameBufferCreateInfo.width = width;
            frameBufferCreateInfo.height = height;
            frameBufferCreateInfo.layers = 1;
            // Create the framebuffer
            VK_CHECK_RESULT(vkCreateFramebuffer(device, &frameBufferCreateInfo, nullptr, &frameBuffers[i]));
        }
    }

    void setupRenderPass() override
    {
        // This example will use a single render pass with one subpass

        // Descriptors for the attachments used by this renderpass
        std::array<VkAttachmentDescription, 2> attachments = {};

        // Color attachment
        attachments[0].format = swapChain.colorFormat;                                  // Use the color format selected by the swapchain
        attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;                                 // We don't use multi sampling in this example
        attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;                            // Clear this attachment at the start of the render pass
        attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;                          // Keep its contents after the render pass is finished (for displaying it)
        attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;                 // We don't use stencil, so don't care for load
        attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;               // Same for store
        attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;                       // Layout at render pass start. Initial doesn't matter, so we use undefined
        attachments[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;                   // Layout to which the attachment is transitioned when the render pass is finished
                                                                                        // As we want to present the color buffer to the swapchain, we transition to PRESENT_KHR
        // Depth attachment
        attachments[1].format = depthFormat;                                           // A proper depth format is selected in the example base
        attachments[1].samples = VK_SAMPLE_COUNT_1_BIT;
        attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;                           // Clear depth at start of first subpass
        attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;                     // We don't need depth after render pass has finished (DONT_CARE may result in better performance)
        attachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;                // No stencil
        attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;              // No Stencil
        attachments[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;                      // Layout at render pass start. Initial doesn't matter, so we use undefined
        attachments[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL; // Transition to depth/stencil attachment

        // Setup attachment references
        VkAttachmentReference colorReference = {};
        colorReference.attachment = 0;                                    // Attachment 0 is color
        colorReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL; // Attachment layout used as color during the subpass

        VkAttachmentReference depthReference = {};
        depthReference.attachment = 1;                                            // Attachment 1 is color
        depthReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL; // Attachment used as depth/stemcil used during the subpass

        // Setup a single subpass reference
        VkSubpassDescription subpassDescription = {};
        subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpassDescription.colorAttachmentCount = 1;                            // Subpass uses one color attachment
        subpassDescription.pColorAttachments = &colorReference;                 // Reference to the color attachment in slot 0
        subpassDescription.pDepthStencilAttachment = &depthReference;           // Reference to the depth attachment in slot 1
        subpassDescription.inputAttachmentCount = 0;                            // Input attachments can be used to sample from contents of a previous subpass
        subpassDescription.pInputAttachments = nullptr;                         // (Input attachments not used by this example)
        subpassDescription.preserveAttachmentCount = 0;                         // Preserved attachments can be used to loop (and preserve) attachments through subpasses
        subpassDescription.pPreserveAttachments = nullptr;                      // (Preserve attachments not used by this example)
        subpassDescription.pResolveAttachments = nullptr;                       // Resolve attachments are resolved at the end of a sub pass and can be used for e.g. multi sampling

        // Setup subpass dependencies
        // These will add the implicit ttachment layout transitionss specified by the attachment descriptions
        // The actual usage layout is preserved through the layout specified in the attachment reference
        // Each subpass dependency will introduce a memory and execution dependency between the source and dest subpass described by
        // srcStageMask, dstStageMask, srcAccessMask, dstAccessMask (and dependencyFlags is set)
        // Note: VK_SUBPASS_EXTERNAL is a special constant that refers to all commands executed outside of the actual renderpass)
        std::array<VkSubpassDependency, 2> dependencies;

        // First dependency at the start of the renderpass
        // Does the transition from final to initial layout
        dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;                             // Producer of the dependency
        dependencies[0].dstSubpass = 0;                                               // Consumer is our single subpass that will wait for the execution depdendency
        dependencies[0].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT; // Match our pWaitDstStageMask when we vkQueueSubmit
        dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT; // is a loadOp stage for color attachments
        dependencies[0].srcAccessMask = 0;                                            // semaphore wait already does memory dependency for us
        dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;         // is a loadOp CLEAR access mask for color attachments
        dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

        // Second dependency at the end the renderpass
        // Does the transition from the initial to the final layout
        // Technically this is the same as the implicit subpass dependency, but we are gonna state it explicitly here
        dependencies[1].srcSubpass = 0;                                               // Producer of the dependency is our single subpass
        dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;                             // Consumer are all commands outside of the renderpass
        dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT; // is a storeOp stage for color attachments
        dependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;          // Do not block any subsequent work
        dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;         // is a storeOp `STORE` access mask for color attachments
        dependencies[1].dstAccessMask = 0;
        dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

        // Create the actual renderpass
        VkRenderPassCreateInfo renderPassInfo = {};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());  // Number of attachments used by this render pass
        renderPassInfo.pAttachments = attachments.data();                            // Descriptions of the attachments used by the render pass
        renderPassInfo.subpassCount = 1;                                             // We only use one subpass in this example
        renderPassInfo.pSubpasses = &subpassDescription;                             // Description of that subpass
        renderPassInfo.dependencyCount = static_cast<uint32_t>(dependencies.size()); // Number of subpass dependencies
        renderPassInfo.pDependencies = dependencies.data();                          // Subpass dependencies used by the render pass

        VK_CHECK_RESULT(vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass));
    }

    virtual void viewChanged() override
    {
        // This function is called by the base example class each time the view is changed by user input
        // updateUniformBuffers(); // Update Projection and View
    }

    virtual void mapExternalObjectToGraphicsSubSystem()
    {
#ifdef UNITY_BUILD
        vkGetPhysicalDeviceMemoryProperties(m_Instance.physicalDevice, &deviceMemoryProperties);

        device = m_Instance.device;

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

//protected:
#ifdef UNITY_BUILD
    IUnityGraphicsVulkan* m_UnityVulkan = NULL;
    XXUnityVulkanInstance m_Instance;
#endif
};
