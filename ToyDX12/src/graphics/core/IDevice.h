#pragma once

class IDevice
{
public:
	virtual void Init() = 0;
	virtual void Terminate() = 0;

	virtual ~IDevice() = default;
};