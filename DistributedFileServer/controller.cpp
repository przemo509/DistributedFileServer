#include <string>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/filesystem.hpp>

#include "json_messages.h"
#include "controller.h"
#include "config.h"
#include "dao.h"

using namespace boost::property_tree;
using namespace std;

controller::controller(config& config):cfg(config), dao(config.getLogFileFullPath())
{
}

controller::~controller()
{
}

string controller::calculateResponse(string request) {
	ptree reqJson = fromString(request);
	string name = reqJson.get<string>(NAME);
	string transactionId = reqJson.get<string>(TRANSACTION_ID);
	
	if (name == MSG_READ) {
		return handleReadMessage(transactionId, reqJson.get<string>(FILENAME));
	} else if (name == MSG_WRITE) {
		return handleWriteMessage(transactionId, reqJson.get<string>(FILENAME), reqJson.get<string>(DATA));
	} else if (name == MSG_PREPARE) {
		return handlePrepareMessage(transactionId);
	} else if (name == MSG_GLOBAL_COMMIT) {
		return handleGlobalCommitMessage(transactionId);
	} else if (name == MSG_GLOBAL_ABORT) {
		return handleGlobalAbortMessage(transactionId);
	} else {
		return makeUnsupportedMessage(transactionId);
	}
}

string controller::handleReadMessage(string transactionId, string filename) {
	string fileContent = getFileContent(transactionId, filename);
	dao.insertOperation(MSG_READ, transactionId, filename, fileContent);
	return makeReadResponseMessage(transactionId, filename, fileContent);
}

string controller::getFileContent(string transactionId, string filename) {
	if (dao.fileWasWrittenInThisTransaction(transactionId, filename)) {
		return dao.lastFileContentFromThisTransaction(transactionId, filename);
	} else {
		return getFileContentFromHardDrive(filename);
	}
}

string controller::getFileContentFromHardDrive(string filename) {
	boost::filesystem::path file(cfg.getSharedDirectoryFullPath());
	file /= filename;
	if (!boost::filesystem::exists(file)) {
		return "";
	} else {
		boost::filesystem::ifstream fileStream(file, std::ios_base::in);
		stringstream stringStream;
		stringStream << fileStream.rdbuf();
		return stringStream.str();
	}
}

string controller::handleWriteMessage(string transactionId, string filename, string fileContent) {
	dao.insertOperation(MSG_WRITE, transactionId, filename, fileContent);
	return makeWriteResponseMessage(transactionId);
}

string controller::handlePrepareMessage(string transactionId) {
	if (canCommitTransaction(transactionId)) {
		dao.insertOperation(MSG_VOTE_COMMIT, transactionId);
		return makeVoteCommitMessage(transactionId);
	} else {
		dao.insertOperation(MSG_VOTE_ABORT, transactionId);
		return makeVoteAbortMessage(transactionId);
	}
}

bool controller::canCommitTransaction(string transactionId) {
	if (dao.writeIntercepted(transactionId)) {
		return false;
	} else if (dao.readIntercepted(transactionId)) {
		return false;
	} 
	return true;
}

string controller::handleGlobalCommitMessage(string transactionId) {
	commitTransaction(transactionId);
	return makeVoteCommitMessage(transactionId);
}

void controller::commitTransaction(string transactionId) {
	commitPendingWrites(transactionId);
	dao.insertOperation(MSG_GLOBAL_COMMIT, transactionId);
}

void controller::commitPendingWrites(string transactionId) {
	vector<pair<string, string>> pendingWrites = dao.getPendingWrites(transactionId);
	for (auto write : pendingWrites) {
		boost::filesystem::path file(cfg.getSharedDirectoryFullPath());
		file /= write.first;
		boost::filesystem::ofstream fileStream(file, std::ios_base::out);
		fileStream << write.second;
		fileStream.close();
	}
}

string controller::handleGlobalAbortMessage(string transactionId) {
	dao.insertOperation(MSG_GLOBAL_ABORT, transactionId);
	return makeVoteAbortMessage(transactionId);
}