#pragma once
#include <string>

extern "C" {
	#include "sqlite3.h"
}

using namespace std;


sqlite3 * openDatabase(string logFilePath) {
	sqlite3 * db;
	if (sqlite3_open(logFilePath.c_str(), &db)) {
		throw new exception(("Cannot open database [" + logFilePath + "]. Error message is:" + string(sqlite3_errmsg(db))).c_str());
	}
	return db;
}

void closeDatabase(sqlite3 * db) {
	sqlite3_close(db);
}

void initLog(string logFilePath) {
	sqlite3 * db = openDatabase(logFilePath);

	char * sErrMsg = 0;
	int returnCode = sqlite3_exec(db,
		"CREATE TABLE IF NOT EXISTS TRANSACTION_ (\n" \
		"id TEXT PRIMARY KEY,\n" \
		"state TEXT\n" \
		")",
		NULL, NULL, &sErrMsg);
	if (returnCode != SQLITE_OK) {
		throw new exception(("Query exception: " + string(sqlite3_errmsg(db))).c_str());
	}

	closeDatabase(db);
}

void logWrite(string transactionId, string filename, string fileContent) {

}

void logVoteCommit(string transactionId) {

}

void logVoteAbort(string transactionId) {

}

void logCommit(string transactionId) {

}

void logAbort(string transactionId) {

}