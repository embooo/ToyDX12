#pragma once

class IPipeline
{
public:
	virtual void Init() = 0;
	virtual void Terminate() = 0;
	virtual ~IPipeline() = 0;
};

inline IPipeline::~IPipeline() {}