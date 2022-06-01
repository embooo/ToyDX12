#include "pch.h"
#include "HelloApp.h"

HelloApp::HelloApp(HINSTANCE hInstance, const char* sz_DebugName)
	: DX12App(hInstance), m_DebugName(sz_DebugName)
{

}

void HelloApp::Init()
{
	if (!DX12App::Initialize())
	{
		return;
	}

	LOG_INFO("{0}::Init() Hello !", m_DebugName);

	Update();
}

int HelloApp::Update()
{
	if (!DX12App::UpdateWindow())
	{
		return false;
	}

	return true;
}

void HelloApp::Terminate()
{
}

HelloApp::~HelloApp()
{
}