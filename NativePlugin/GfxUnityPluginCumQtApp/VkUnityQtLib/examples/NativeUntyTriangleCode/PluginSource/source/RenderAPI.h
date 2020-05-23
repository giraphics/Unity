#pragma once

#include "Unity/IUnityGraphics.h"

#include <stddef.h>

struct IUnityInterfaces;


class RenderAPI
{
public:
	virtual ~RenderAPI() { }

	// Process general event like initialization, shutdown, device loss/reset etc.
	virtual void ProcessDeviceEvent(UnityGfxDeviceEventType type, IUnityInterfaces* interfaces) = 0;

    virtual void DrawTriangle() = 0;
};

RenderAPI* CreateRenderAPI(UnityGfxRenderer apiType);

