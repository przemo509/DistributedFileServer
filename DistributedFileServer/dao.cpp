#include <iostream>
#include <iomanip>
#include <string>
#include <vector>

#include "json_messages.h"
#include "dao.h"

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

	if (sqlite3_exec(db, query.c_str(), NULL, NULL, NULL) != SQLITE_OK) {
		closeDatabase(db);
		throw exception(("Run query exception: " + string(sqlite3_errmsg(db)) + "\nQuery was: " + query).c_str());
	}
	
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

void dao::prepareStatement(sqlite3 **db, sqlite3_stmt **stmt, string query) {
	*db = openDatabase();
	if (sqlite3_prepare_v2(*db, query.c_str(), -1, stmt, 0) != SQLITE_OK) {
		throw exception(("Prepare statement exception: " + string(sqlite3_errmsg(*db)) + "\nQuery was: " + query).c_str());
	}
}

void dao::closeStatement(sqlite3 *db, sqlite3_stmt *stmt) {
	if (sqlite3_finalize(stmt) != SQLITE_OK) {
		throw exception(("Finalize statement exception: " + string(sqlite3_errmsg(db))).c_str());
	}
	closeDatabase(db);
}

void dao::bindTextParam(sqlite3 *db, sqlite3_stmt *stmt, int idx, string value) {
	if (sqlite3_bind_text(stmt, idx, value.c_str(), -1, SQLITE_TRANSIENT) != SQLITE_OK) {
		throw exception(("Bind statement exception: " + string(sqlite3_errmsg(db))).c_str());
	}
}

string dao::getText(sqlite3_stmt *stmt, int col) {
	return string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, col)));
}

void dao::insertOperation(string name, string transactionId) {
	insertOperation(name, transactionId, "", "");
}

void dao::insertOperation(string name, string transactionId, string filename, string fileContent) {
	sqlite3_stmt *stmt = NULL;
	sqlite3 *db = NULL;
	string query = "INSERT INTO OPERATION (name, transaction_id, filename, data) VALUES (?, ?, ?, ?)";
	prepareStatement(&db, &stmt, query);
	bindTextParam(db, stmt, 1, name);
	bindTextParam(db, stmt, 2, transactionId);
	bindTextParam(db, stmt, 3, filename);
	bindTextParam(db, stmt, 4, fileContent);

	if (sqlite3_step(stmt) != SQLITE_DONE) {
		throw exception(("Insert exception: " + string(sqlite3_errmsg(db)) + "\nQuery was: " + query).c_str());
	}

	closeStatement(db, stmt);

	cout << setw(50) << transactionId << setw(20) << name << setw(20) << filename << "\t" << fileContent << endl;
}

bool dao::fileWasWrittenInThisTransaction(string transactionId, string filename) {
	sqlite3_stmt *stmt = NULL;
	sqlite3 *db = NULL;
	string query = "SELECT COUNT(*) " \
		"FROM OPERATION o " \
		"WHERE o.transaction_id = ? " \
		"AND o.filename = ? " \
		"AND o.name = ?";
	prepareStatement(&db, &stmt, query);
	bindTextParam(db, stmt, 1, transactionId);
	bindTextParam(db, stmt, 2, filename);
	bindTextParam(db, stmt, 3, MSG_WRITE);

	if (sqlite3_step(stmt) == SQLITE_ROW) {
		int count = sqlite3_column_int(stmt, 0);
		if (sqlite3_step(stmt) != SQLITE_DONE) {
			throw exception(("Select count not done exception: " + string(sqlite3_errmsg(db)) + "\nQuery was: " + query).c_str());
		}
		closeStatement(db, stmt);
		return count > 0;
	} else {
		throw exception(("Select count exception: " + string(sqlite3_errmsg(db)) + "\nQuery was: " + query).c_str());
	}
}

string dao::lastFileContentFromThisTransaction(string transactionId, string filename) {
	sqlite3_stmt *stmt = NULL;
	sqlite3 *db = NULL;
	string query = "SELECT o.data " \
		"FROM OPERATION o " \
		"WHERE o.transaction_id = ? " \
		"AND o.filename = ? " \
		"AND o.name = ?" \
		"ORDER BY o.id DESC " \
		"LIMIT 1";
	prepareStatement(&db, &stmt, query);
	bindTextParam(db, stmt, 1, transactionId);
	bindTextParam(db, stmt, 2, filename);
	bindTextParam(db, stmt, 3, MSG_WRITE);

	if (sqlite3_step(stmt) == SQLITE_ROW) {
		string fileContent = getText(stmt, 0);
		if (sqlite3_step(stmt) != SQLITE_DONE) {
			throw exception(("Select not done exception: " + string(sqlite3_errmsg(db)) + "\nQuery was: " + query).c_str());
		}
		closeStatement(db, stmt);
		return fileContent;
	} else {
		throw exception(("Select exception: " + string(sqlite3_errmsg(db)) + "\nQuery was: " + query).c_str());
	}
}

vector<pair<string, string>> dao::getPendingWrites(string transactionId) {
	sqlite3_stmt *stmt = NULL;
	sqlite3 *db = NULL;
	string query = "SELECT o.filename, o.data " \
		"FROM OPERATION o " \
		"WHERE o.transaction_id = ? "\
		"AND o.name = ?";
	prepareStatement(&db, &stmt, query);
	bindTextParam(db, stmt, 1, transactionId);
	bindTextParam(db, stmt, 2, MSG_WRITE);

	vector<pair<string, string>> writes;

	int result;
	while ((result = sqlite3_step(stmt)) == SQLITE_ROW) {
		string filename = getText(stmt, 0);
		string fileContent = getText(stmt, 1);
		writes.push_back(pair<string, string>(filename, fileContent));
	}
	if (result != SQLITE_DONE) {
		throw exception(("Select not done exception: " + string(sqlite3_errmsg(db)) + "\nQuery was: " + query).c_str());
	}
	closeStatement(db, stmt);
	return writes;
}

bool dao::writeIntercepted(string transactionId) {
	sqlite3_stmt *stmt = NULL;
	sqlite3 *db = NULL;
	string query = "SELECT COUNT(*)\n" \
		"FROM OPERATION o\n" \
		"-- je�li piszemy do tego samego pliku...\n" \
		"WHERE o.filename = (SELECT filename FROM OPERATION WHERE transaction_id = ? AND name = 'write' LIMIT 1)\n" \
		"-- ... co inne transakcje...\n" \
		"AND o.transaction_id != ?\n" \
		"-- ... pisa�y lub czyta�y...\n" \
		"AND (o.name = 'write' OR o.name = 'read')\n" \
		"-- ... po rozpocz�ciu naszego zapisu...\n" \
		"AND o.id > (SELECT id FROM OPERATION WHERE transaction_id = ? ORDER BY id ASC LIMIT 1)\n" \
		"-- ... i sko�czy�y si�...\n" \
		"AND EXISTS(SELECT 1 FROM OPERATION WHERE transaction_id = o.transaction_id AND name = 'global_commit')";
	prepareStatement(&db, &stmt, query);
	bindTextParam(db, stmt, 1, transactionId);
	bindTextParam(db, stmt, 2, transactionId);
	bindTextParam(db, stmt, 3, transactionId);

	if (sqlite3_step(stmt) == SQLITE_ROW) {
		int count = sqlite3_column_int(stmt, 0);
		if (sqlite3_step(stmt) != SQLITE_DONE) {
			throw exception(("Select count not done exception: " + string(sqlite3_errmsg(db)) + "\nQuery was: " + query).c_str());
		}
		closeStatement(db, stmt);
		return count > 0;
	} else {
		throw exception(("Select count exception: " + string(sqlite3_errmsg(db)) + "\nQuery was: " + query).c_str());
	}
}

bool dao::readIntercepted(string transactionId) {
	sqlite3_stmt *stmt = NULL;
	sqlite3 *db = NULL;
	string query = "SELECT COUNT(*)\n" \
		"FROM OPERATION o\n" \
		"-- je�li czytamy z tego samego pliku...\n" \
		"WHERE o.filename = (SELECT filename FROM OPERATION WHERE transaction_id = ? AND name = 'read' LIMIT 1)\n" \
		"-- ... co inne transakcje...\n" \
		"AND o.transaction_id != ?\n" \
		"-- ... pisa�y...\n" \
		"AND o.name = 'write'\n" \
		"-- ... po rozpocz�ciu naszego odczytu...\n" \
		"AND o.id > (SELECT id FROM OPERATION WHERE transaction_id = ? ORDER BY id ASC LIMIT 1)\n" \
		"-- ... i sko�czy�y si�...\n" \
		"AND EXISTS(SELECT 1 FROM OPERATION WHERE transaction_id = o.transaction_id AND name = 'global_commit')";
	prepareStatement(&db, &stmt, query);
	bindTextParam(db, stmt, 1, transactionId);
	bindTextParam(db, stmt, 2, transactionId);
	bindTextParam(db, stmt, 3, transactionId);

	if (sqlite3_step(stmt) == SQLITE_ROW) {
		int count = sqlite3_column_int(stmt, 0);
		if (sqlite3_step(stmt) != SQLITE_DONE) {
			throw exception(("Select count not done exception: " + string(sqlite3_errmsg(db)) + "\nQuery was: " + query).c_str());
		}
		closeStatement(db, stmt);
		return count > 0;
	} else {
		throw exception(("Select count exception: " + string(sqlite3_errmsg(db)) + "\nQuery was: " + query).c_str());
	}
}