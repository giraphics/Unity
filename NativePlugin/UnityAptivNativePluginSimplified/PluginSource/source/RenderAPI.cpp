#include "RenderAPI.h"
#include "Unity/IUnityGraphics.h"


RenderAPI* CreateRenderAPI(UnityGfxRenderer apiType)
{
	if (apiType == kUnityGfxRendererVulkan)
	{
		extern RenderAPI* CreateRenderAPI_Vulkan();
		return CreateRenderAPI_Vulkan();
	}

	// Unknown or unsupported graphics API
	return NULL;
}
