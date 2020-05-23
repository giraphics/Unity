#include "RenderAPI.h"
#include "Unity/IUnityGraphics.h"
#include "RenderAPI_Vulkan.h"

RenderAPI* CreateRenderAPI(UnityGfxRenderer apiType)
{
	if (apiType == kUnityGfxRendererVulkan)
	{
        return new RenderAPI_VulkanNew();
	}

	// Unknown or unsupported graphics API
	return NULL;
}
