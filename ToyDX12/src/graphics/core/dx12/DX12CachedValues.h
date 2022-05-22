#pragma once

struct DescriptorSizes
{
	unsigned int RTV;
	unsigned int DSV;
	unsigned int CBV_SRV_UAV;
};

struct DX12CachedValues
{
	DescriptorSizes descriptorSizes;
};