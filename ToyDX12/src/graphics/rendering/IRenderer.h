#pragma once

class IRenderer
{
public:
	IRenderer() = default;

	virtual void Initialize() = 0;
	virtual void Render() = 0;
	virtual void Terminate() = 0;

	virtual ~IRenderer() = default;
};