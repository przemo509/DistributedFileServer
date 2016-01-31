#pragma once
class config
{
public:
	config(const std::string& path);
	~config();


	bool getAutoMode() { return autoMode; }
	int getPort() { return port; }
	std::string getLogFileFullPath() { return logFileFullPath; }
	std::string getSharedDirectoryFullPath() { return sharedDirectoryFullPath; }
private:
	bool autoMode;
	int port;
	std::string logFileFullPath;
	std::string sharedDirectoryFullPath;

	void check();
};

