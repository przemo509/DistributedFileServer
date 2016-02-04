#include <string>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/filesystem.hpp>

#include "controller.h"
#include "config.h"
#include "json_messages.h"
#include "log.h"

using namespace boost::property_tree;
using namespace std;

controller::controller(config& config):cfg(config)
{
	initLog(cfg.getLogFileFullPath());
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
	string fileContent = getFileContent(filename);
	return makeReadResponseMessage(transactionId, filename, fileContent);
}

string controller::getFileContent(string filename) {
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
	logWrite(transactionId, filename, fileContent);
	return makeWriteResponseMessage(transactionId);
}

string controller::handlePrepareMessage(string transactionId) {
	if (canCommitTransaction(transactionId)) {
		logVoteCommit(transactionId);
		return makeVoteCommitMessage(transactionId);
	} else {
		logVoteAbort(transactionId);
		return makeVoteAbortMessage(transactionId);
	}
}

bool controller::canCommitTransaction(string transactionId) {
	return true; // TODO
}

string controller::handleGlobalCommitMessage(string transactionId) {
	commitTransaction(transactionId);
	return makeVoteCommitMessage(transactionId);
}

void controller::commitTransaction(string transactionId) {
	commitPendingWrites(transactionId);
	logCommit(transactionId);
}

void controller::commitPendingWrites(string transactionId) {
	// TODO
}

string controller::handleGlobalAbortMessage(string transactionId) {
	logAbort(transactionId);
	return makeVoteAbortMessage(transactionId);

}