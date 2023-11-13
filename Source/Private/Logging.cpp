#include "../Public/Logging.h"

Logging::Logger* Logging::Logger::Instance = Logging::Logger::GetInstance();

Logging::Logger* Logging::Logger::GetInstance()
{
	if (Instance == nullptr)
	{
		Instance = new Logger();
	}
	return Instance;
}
