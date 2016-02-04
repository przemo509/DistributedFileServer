#pragma once

#include <string>

extern "C" {
#include "sqlite3.h"
}

class dao
{
public:
	dao(std::string databaseLocation);
	~dao();

	bool fileWasWrittenInThisTransaction(std::string transactionId, std::string filename);
	std::string lastFileContentFromThisTransaction(std::string transactionId, std::string filename);
private:
	std::string databaseLocation;

	void initDatabase();
	void runQuery(std::string query);
	sqlite3 * openDatabase();
	void closeDatabase(sqlite3 * db);
	void prepareStatement(std::string query, sqlite3_stmt ** stmt, sqlite3 ** db);
	void closeStatement(sqlite3_stmt * stmt, sqlite3 * db);
};

