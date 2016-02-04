#pragma once

#include <string>

class controller
{
public:
	controller(config& config);
	~controller();
	std::string calculateResponse(std::string request);
private:
	config cfg;

	std::string handleReadMessage(std::string transactionId, std::string filename);
	std::string getFileContent(std::string filename);
	std::string handleWriteMessage(std::string transactionId, std::string filename, std::string fileContent);
	std::string handlePrepareMessage(std::string transactionId);
	bool canCommitTransaction(std::string transactionId);
	std::string handleGlobalCommitMessage(std::string transactionId);
	void commitTransaction(std::string transaction);
	void commitPendingWrites(std::string transactionId);
	std::string handleGlobalAbortMessage(std::string transactionId);
};

