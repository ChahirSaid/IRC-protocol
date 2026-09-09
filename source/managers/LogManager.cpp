#include "LogManager.hpp"

LogManager::LogManager() : _level(DEBUG) {}

LogManager::~LogManager() {}

void LogManager::setLogLevel(LogLevel level)
{
	_level = level;

	return;
}

void LogManager::log(LogLevel level, const std::string& message)
{
	if(level < _level)
	{
		return;
	}

	switch(level)
	{
		case DEBUG:
			std::cout << "[DEBUG]: " << message << std::endl;
			break;

		case INFO:
			std::cout << "[INFO]: " << message << std::endl;
			break;

		case WARNING:
			std::cerr << "[WARNING]: " << message << std::endl;
			break;

		case ERROR:
			std::cerr << "[ERROR]: " << message << std::endl;
			break;

		default:
			std::cout << "[UNKNOWN]: " << message << std::endl;
			break;

	}

	return;
}

void LogManager::debug(const std::string& message)
{
	log(DEBUG, message);

	return;
}

void LogManager::info(const std::string& message)
{
	log(INFO, message);

	return;
}

void LogManager::warning(const std::string& message)
{
	log(WARNING, message);

	return;
}

void LogManager::error(const std::string& message)
{
	log(ERROR, message);

	return;
}