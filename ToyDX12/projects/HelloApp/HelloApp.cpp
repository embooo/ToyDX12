#include "pch.h"

#include "HelloApp.h"

HelloApp::HelloApp(HINSTANCE hInstance, const char* sz_DebugName)
	: DX12App(hInstance), m_DebugName(sz_DebugName)
{

}

void HelloApp::Init()
{
	// Initialize basic functionalities : Window, Direct3D...
	if (!DX12App::Initialize())
	{
		return;
	}

	LOG_INFO("{0}::Init() Hello !", m_DebugName);
}

void HelloApp::Update()
{
	LOG_ERROR("Update");
}

void HelloApp::Draw()
{
	LOG_ERROR("Draw");
}

void HelloApp::Terminate()
{
}

HelloApp::~HelloApp()
{
}