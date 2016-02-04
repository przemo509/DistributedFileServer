#pragma once

#include <string>
#include "config.h"
#include "dao.h"

class controller
{
public:
	static const std::string NO_RESPONSE;
	controller(config& config);
	~controller();
	std::string calculateResponse(std::string request);
private:
	config cfg;
	dao dao;

	std::string handleReadMessage(std::string transactionId, std::string filename);
	std::string getFileContent(std::string transactionId, std::string filename);
	std::string getFileContentFromHardDrive(std::string filename);
	std::string handleWriteMessage(std::string transactionId, std::string filename, std::string fileContent);
	std::string handlePrepareMessage(std::string transactionId);
	bool canCommitTransaction(std::string transactionId);
	std::string handleGlobalCommitMessage(std::string transactionId);
	void commitTransaction(std::string transaction);
	void commitPendingWrites(std::string transactionId);
	std::string handleGlobalAbortMessage(std::string transactionId);
};

