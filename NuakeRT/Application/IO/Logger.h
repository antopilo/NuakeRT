#pragma once
#include <string>
#include <vector>

class Logger
{
private:
	std::vector<std::string> Logs;

public:
	static Logger& Get()
	{
		static Logger logger;
		return logger;
	}
	
	Logger() = default;
	~Logger() = default;

	void Log(const std::string& log);

	std::vector<std::string> GetLogs() const
	{
		return Logs;
	}

	void Clear()
	{
		Logs.clear();
	}
};