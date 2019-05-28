#ifndef _LOGGER_H
#define _LOGGER_H 
#include"Common.hpp"
#include"lambdaTask.hpp"
#include<ctime>
#include<iostream>
using namespace std;
//#define LOG_TRACE if (Logger::logLevel() <= Logger::TRACE) Logger(__FILE__, __LINE__, Logger::TRACE, __func__).stream()
//#define LOG_DEBUG if (Logger::logLevel() <= Logger::DEBUG) Logger(__FILE__, __LINE__, Logger::DEBUG, __func__).stream()
//#define LOG_INFO if (Logger::logLevel() <= Logger::INFO) Logger(__FILE__, __LINE__).stream()
//#define LOG_WARN Logger(__FILE__, __LINE__, Logger::WARN).stream()
//#define LOG_ERROR Logger(__FILE__, __LINE__, Logger::ERROR).stream()
//#define LOG_FATAL Logger(__FILE__, __LINE__, Logger::FATAL).stream()
//#define LOG_SYSERR Logger(__FILE__, __LINE__, false).stream()
//#define LOG_SYSFATAL Logger(__FILE__, __LINE__, true).stream()
//#define LOG_DEBUG_BIN(x,l) if (Logger::logLevel() <= Logger::DEBUG) Logger(__FILE__, __LINE__, Logger::DEBUG, __func__).WriteLog((x), (l))

class Logger
{
private:
	FILE* _logFile = nullptr;
	LambdaTask _taskServer;
	Logger()
	{
		_taskServer.Start();
	}

	~Logger()
	{
		_taskServer.Close();
		if (_logFile)
		{
			Info("CELLLog fclose(_logFile)\n");
			fclose(_logFile);
			_logFile = nullptr;
		}
	}
public:
	static Logger& Instance()
	{
		static Logger sLog;
		return sLog;
	}

	void setLogPath(const char* logPath, const char* mode)
	{
		if (_logFile)
		{
			Info("CELLLog::setLogPath _logFile != nullptr\n");
			fclose(_logFile);
			_logFile = nullptr;
		}


		_logFile = fopen(logPath, mode);
		if (_logFile)
		{
			std::cout << "fileRight" << std::endl;
			Info("CELLLog::setLogPath success,<%s,%s>\n", logPath, mode);
		}
		else 
		{
			std::cout << "fileError" << std::endl;
			Info("CELLLog::setLogPath failed,<%s,%s>\n", logPath, mode);
		}
	}

	static void Info(const char* pStr)
	{
		Logger* pLog = &Instance();
		pLog->_taskServer.addTask([=]() {
			if (pLog->_logFile)
			{
				auto t = system_clock::now();
				auto tNow = system_clock::to_time_t(t);
				//fprintf(pLog->_logFile, "%s", ctime(&tNow));
				std::tm* now = std::gmtime(&tNow);
				fprintf(pLog->_logFile, "%s", "Info ");
				fprintf(pLog->_logFile, "[%d-%d-%d %d:%d:%d]", now->tm_year + 1900, now->tm_mon + 1, now->tm_mday, now->tm_hour, now->tm_min, now->tm_sec);
				fprintf(pLog->_logFile, "%s", pStr);
				fflush(pLog->_logFile);
			}
			printf("%s", pStr);
		});
	}

	template<typename ...Args>
	static void Info(const char* pformat, Args ... args)
	{
		Logger* pLog = &Instance();
		pLog->_taskServer.addTask([=]() {
			if (pLog->_logFile)
			{
				auto t = system_clock::now();
				auto tNow = system_clock::to_time_t(t);
				//fprintf(pLog->_logFile, "%s", ctime(&tNow));
				std::tm* now = std::gmtime(&tNow);
				fprintf(pLog->_logFile, "%s", "Info ");
				fprintf(pLog->_logFile, "[%d-%d-%d %d:%d:%d]", now->tm_year + 1900, now->tm_mon + 1, now->tm_mday, now->tm_hour, now->tm_min, now->tm_sec);
				fprintf(pLog->_logFile, pformat, args...);
				fflush(pLog->_logFile);
			}
			printf(pformat, args...);
		});
	}
};

#endif // _LOGGER_H
