#include "pch.h"

#include "Logger.h"
#include "spdlog/sinks/stdout_color_sinks.h"

std::shared_ptr<spdlog::logger> Logger::s_Logger;

void Logger::Init(const char* loggerName, spdlog::level::level_enum e_LogLevel)
{
	// https://github.com/gabime/spdlog#create-stdoutstderr-logger-object
	// https://github.com/gabime/spdlog/wiki/3.-Custom-formatting

	spdlog::set_pattern("%^[%T] %n: %v%$");

	// Create 
	s_Logger = spdlog::stdout_color_mt(loggerName);
	s_Logger->set_level(e_LogLevel);
	s_Logger->flush();
}
