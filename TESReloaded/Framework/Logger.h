#pragma once

class Logger {
public:
	static void CreateLog(char* FileName);
	static void Log(char* Message, ...);
	
	static char			MessageBuffer[1024];
	static FILE*		LogFile;

};