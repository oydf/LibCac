#ifndef _LOGGER_H
#define _LOGGER_H 
#include"Common.hpp"
#include"lambdaTask.hpp"
#include<ctime>
#include<atomic>

#define LOG_TRACE(filename,line,traceInfo) if (Logger::getLogLevel() <= LogLevel::TRACE) \
		Logger::Info(filename, line,traceInfo)
#define LOG_DEBUG(filename,line,str) if (Logger::getLogLevel() <= LogLevel::DEBUG) \
		Logger::Info(filename,line, str)
#define LOG_INFO if (Logger::getLogLevel() <= LogLevel::INFO) Logger::Info
#define LOG_WARN(filename, line,Info) Logger::Info(filename,line,Info)
#define LOG_EOR(filename, line,Info) Logger::Info(filename,line,Info)
#define LOG_FATAL(filename, line,Info) Logger::Info(filename,line,Info)

enum LogLevel
{
	TRACE,
	DEBUG,
	INFO,
	WARN,
	EOR,
	FATAL,
	NUM_LOG_LEVELS
};

class Logger
{
private:
	FILE* _logFile = nullptr;
	LambdaTask _taskServer;
	long long logId;
	bool logFile;
	Logger():logFile(true)
	{
		_glevel = (LogLevel)2;
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
	LogLevel _glevel;

	static LogLevel getLogLevel()
	{
		return Instance()._glevel;
	}

	static bool getLogFile()
	{
		return Instance().logFile;
	}

	static void setLogLevel(LogLevel level)
	{
		Instance()._glevel = level;
	}

	static void setLogFile(bool lf)
	{
		Instance().logFile = lf;
	}

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
			Info("CELLLog::setLogPath success,<%s,%s>\n", logPath, mode);
		}
		else 
		{
			Info("CELLLog::setLogPath failed,<%s,%s>\n", logPath, mode);
		}
	}

	static void Info(const char* pStr)
	{
		Logger* pLog = &Instance();
		
		pLog->_taskServer.addTask([=]() {
			if (pLog->_logFile && pLog->logFile)
			{
				auto t = system_clock::now();
				auto tNow = system_clock::to_time_t(t);
				//fprintf(pLog->_logFile, "%s", ctime(&tNow));
				std::tm* now = std::gmtime(&tNow);
				fprintf(pLog->_logFile, "%s%ld", "Log... ", pLog->logId++);
				fprintf(pLog->_logFile, "[ %d-%d-%d %d:%d:%d ] ", now->tm_year + 1900, now->tm_mon + 1, now->tm_mday, now->tm_hour, now->tm_min, now->tm_sec);
				fprintf(pLog->_logFile, " %s", pStr);
				fflush(pLog->_logFile);
			}
			else
			{
				printf("%s", pStr);
			}
		});
	}

	template<typename ...Args>
	static void Info(const char* pformat, Args ... args)
	{
		Logger* pLog = &Instance();
		pLog->_taskServer.addTask([=]() {
			if (pLog->_logFile && pLog->logFile)
			{
				auto t = system_clock::now();
				auto tNow = system_clock::to_time_t(t);
				//fprintf(pLog->_logFile, "%s", ctime(&tNow));
				std::tm* now = std::gmtime(&tNow);
				fprintf(pLog->_logFile, "%s%ld", "Log... ", pLog->logId++);
				fprintf(pLog->_logFile, " [ %d-%d-%d %d:%d:%d ] ", now->tm_year + 1900, now->tm_mon + 1, now->tm_mday, now->tm_hour, now->tm_min, now->tm_sec);
				fprintf(pLog->_logFile, pformat, args...);
				fflush(pLog->_logFile);
			}
			else
			{
				printf(pformat, args...);
			}
		});
	}
};

#endif // _LOGGER_H
