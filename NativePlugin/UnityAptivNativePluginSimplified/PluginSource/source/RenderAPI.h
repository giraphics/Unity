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

	// Is the API using "reversed" (1.0 at near plane, 0.0 at far plane) depth buffer?
	// Reversed Z is used on modern platforms, and improves depth buffer precision.
	virtual bool GetUsesReverseZ() = 0;

	// Draw some triangle geometry, using some simple rendering state.
	// Upon call into our plug-in the render state can be almost completely arbitrary depending
	// on what was rendered in Unity before. Here, we turn off culling, blending, depth writes etc.
	// and draw the triangles with a given world matrix. The triangle data is
	// float3 (position) and byte4 (color) per vertex.
	virtual void DrawSimpleTriangles(const float worldMatrix[16], int triangleCount, const void* verticesFloat3Byte4) = 0;
};

RenderAPI* CreateRenderAPI(UnityGfxRenderer apiType);

