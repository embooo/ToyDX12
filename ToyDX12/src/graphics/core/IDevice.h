#pragma once

class IDevice
{
public:
	virtual void Init() = 0;
	virtual void Terminate() = 0;
	virtual ~IDevice() = 0;
};

inline IDevice::~IDevice() {}