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

	void insertOperation(std::string name, std::string transactionId);
	void insertOperation(std::string name, std::string transactionId, std::string filename, std::string fileContent);

	bool fileWasWrittenInThisTransaction(std::string transactionId, std::string filename);
	std::string lastFileContentFromThisTransaction(std::string transactionId, std::string filename);
private:
	std::string databaseLocation;

	void initDatabase();
	void runQuery(std::string query);
	sqlite3 * openDatabase();
	void closeDatabase(sqlite3 * db);
	void prepareStatement(sqlite3 **db, sqlite3_stmt **stmt, std::string query);
	void closeStatement(sqlite3 * db, sqlite3_stmt * stmt);
	void bindTextParam(sqlite3 *db, sqlite3_stmt * stmt, int idx, std::string value);
	std::string getText(sqlite3_stmt * stmt, int col);
};

