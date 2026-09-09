#pragma once

#include <iostream>
#include <string>

enum LogLevel
{
	DEBUG,
	INFO,
	WARNING,
	ERROR
};

class LogManager
{
	private:
		LogLevel _level;

	public:
		LogManager();
		~LogManager();

		void setLogLevel(LogLevel level);
		void log(LogLevel level, const std::string& message);
		void debug(const std::string& message);
		void info(const std::string& message);
		void warning(const std::string& message);
		void error(const std::string& message);
};