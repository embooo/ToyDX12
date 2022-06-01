#pragma once

#include "Window.h"
#include "DX12RenderingPipeline.h"

class IApp
{
public:
	virtual ~IApp() = 0;

public:
	virtual void Init() = 0; // App specific code for example init meshes, cameras ...
	virtual int  Update() = 0; // App specific code for example update animations, camera ...
	virtual void Terminate() = 0;
};

inline IApp::~IApp() {}