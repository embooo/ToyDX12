#pragma once

#include "DX12App.h"

class HelloApp : DX12App
{
public:
	HelloApp(HINSTANCE hInstance, const char* sz_DebugName = "DefaultAppName");
	~HelloApp() override;

	virtual void Init() override;
	virtual int  Update() override;
	virtual void Terminate() override;
protected:
	const char* m_DebugName;
};