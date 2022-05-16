#pragma once

#include "spdlog/spdlog.h"

class Logger
{
public:
	static void Init(const char* loggerName, spdlog::level::level_enum e_LogLevel = spdlog::level::trace);
	static std::shared_ptr<spdlog::logger>& GetLogger() { return s_Logger; }
	
	Logger() = delete;
protected:
	static std::shared_ptr<spdlog::logger> s_Logger;
};

#define LOG_ERROR(...) Logger::GetLogger()->error(__VA_ARGS__)
#define LOG_WARN(...)  Logger::GetLogger()->warn(__VA_ARGS__)
#define LOG_INFO(...)  Logger::GetLogger()->info(__VA_ARGS__)
#define LOG_DEBUG(...) Logger::GetLogger()->debug(__VA_ARGS__)
