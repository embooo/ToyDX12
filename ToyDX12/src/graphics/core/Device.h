#pragma once

class Device
{
	Device() = default;

	void Init();
	void Terminate();

	virtual ~Device() = default;
};