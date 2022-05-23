#pragma once

#include "Window.h"
#include "DX12RenderingPipeline.h"

class IApp
{
public:
	virtual ~IApp() = 0;

public:
	virtual void Init() = 0;
	virtual int  Update() = 0;
	virtual void Terminate() = 0;
};

inline IApp::~IApp() {}