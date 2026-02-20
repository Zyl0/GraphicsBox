#pragma once

#ifdef PLATFORM_WINDOWS
#define _LOGGER_UseHeaderSTDIO
#endif

#ifdef _LOGGER_UseHeaderSTDIO
#include <stdio.h>
namespace Logger
{
#define _LoggerFormatedLog(Level, format) 		printf("[" Level ": %s line %d] " format ".\n", __FILE__, __LINE__)
#define _LoggerFormatedLogF(Level, format,...) 	printf("[" Level ": %s line %d] " format ".\n", __FILE__, __LINE__, ##__VA_ARGS__)
}
#else
namespace Logger
{
	/**
		 * @brief Log a string to the console
		 * @param file the file calling the log
		 * @param line the line from where it has been called
		 * @param level the level of the log
		 * @param format log text format
		 * @param ... log format variable arguments
		*/
	void Log(const char* file, unsigned int line, const char* level, const char* format, ...); 
}
#define _LoggerFormatedLog(Level, format) 		Logger::Log(__FILE__, __LINE__, Level, format)
#define _LoggerFormatedLogF(Level, format,...)  Logger::Log(__FILE__, __LINE__, Level, format, __VA_ARGS__)
#endif

	/**
		* @brief Log a message to the console
		* @param format log text format
		*/
#define EngineLoggerLog(format) _LoggerFormatedLog("LOG", format)
	/**
		* @brief Log a message to the console
		* @param format log text format
		 * @param ... log format variable arguments
		*/
#define EngineLoggerLogF(format,...) _LoggerFormatedLogF("LOG", format, ##__VA_ARGS__)


	/**
		* @brief Log a warning to the console
		* @param format log text format
		*/
#define EngineLoggerWarn(format) _LoggerFormatedLog("WARNING", format)
	/**
		* @brief Log a warning to the console
		* @param format log text format
		 * @param ... log format variable arguments
		*/
#define EngineLoggerWarnF(format,...) _LoggerFormatedLogF("WARNING", format, ##__VA_ARGS__)


	/**
		* @brief Log an error to the console
		* @param format log text format
		*/
#define EngineLoggerError(format) _LoggerFormatedLog("ERROR", format)
	/**
		* @brief Log an error to the console
		* @param format log text format
		 * @param ... log format variable arguments
		*/
#define EngineLoggerErrorF(format,...) _LoggerFormatedLogF("ERROR", format, ##__VA_ARGS__)


	/**
		* @brief Log an fatal error to the console
		* @param format log text format
		*/
#define EngineLoggerFatal(format) _LoggerFormatedLog("FATAL", format)
	/**
		* @brief Log an fatal error to the console
		* @param format log text format
		 * @param ... log format variable arguments
		*/
#define EngineLoggerFatalF(format,...) _LoggerFormatedLogF("FATAL", format, ##__VA_ARGS__)