#include "dao.h"
#include <string>

extern "C" {
#include "sqlite3.h"
}

using namespace std;


dao::dao(string databaseLocation): databaseLocation(databaseLocation)
{
	initDatabase();
}


dao::~dao()
{
}

void dao::initDatabase() {
	runQuery(
		"CREATE TABLE IF NOT EXISTS OPERATION (\n" \
		"id INTEGER PRIMARY KEY AUTOINCREMENT,\n" \
		"name TEXT NOT NULL,\n" \
		"transaction_id TEXT NOT NULL,\n" \
		"filename TEXT,\n" \
		"data TEXT" \
		")");
}

void dao::runQuery(string query) {
	sqlite3 *db = openDatabase();
	sqlite3_exec(db, query.c_str(), NULL, NULL, NULL);
	closeDatabase(db);
}

sqlite3 * dao::openDatabase() {
	sqlite3 * db;
	if (sqlite3_open(databaseLocation.c_str(), &db)) {
		throw exception(("Cannot open database [" + databaseLocation + "]. Error message is: " + string(sqlite3_errmsg(db))).c_str());
	}
	return db;
}

void dao::closeDatabase(sqlite3 * db) {
	if (sqlite3_close(db)) {
		throw exception(("Closing database exception: " + string(sqlite3_errmsg(db))).c_str());
	}
}

void dao::prepareStatement(string query, sqlite3_stmt **stmt, sqlite3 **db) {
	*db = openDatabase();
	if (sqlite3_prepare_v2(*db, query.c_str(), -1, stmt, 0) != SQLITE_OK) {
		throw exception(("Prepare statement exception: " + string(sqlite3_errmsg(*db)) + "\nQuery was: " + query).c_str());
	}
}

void dao::closeStatement(sqlite3_stmt *stmt, sqlite3 *db) {
	if (sqlite3_finalize(stmt) != SQLITE_OK) {
		throw exception(("Finalize statement exception: " + string(sqlite3_errmsg(db))).c_str());
	}
	closeDatabase(db);
}

bool dao::fileWasWrittenInThisTransaction(string transactionId, string filename) {
	sqlite3_stmt *stmt = NULL;
	sqlite3 *db = NULL;
	string query = "SELECT";
	prepareStatement(query, &stmt, &db);


	// if there were parameters to bind, we'd do that here

	while (sqlite3_step(stmt) == SQLITE_ROW) {
		// retrieve the value of the first column (0-based)
		int count = sqlite3_column_int(stmt, 0);

		// do something with count
	}
	closeStatement(stmt, db);
	return false;
}

string dao::lastFileContentFromThisTransaction(string transactionId, string filename) {
	sqlite3_stmt *stmt = NULL;
	sqlite3 *db = NULL;
	string query = "SELECT";
	prepareStatement(query, &stmt, &db);


	// if there were parameters to bind, we'd do that here

	while (sqlite3_step(stmt) == SQLITE_ROW) {
		// retrieve the value of the first column (0-based)
		int count = sqlite3_column_int(stmt, 0);

		// do something with count
	}
	closeStatement(stmt, db);
	return "";
}