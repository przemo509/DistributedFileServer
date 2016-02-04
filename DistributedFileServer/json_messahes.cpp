#include <string>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

using boost::property_tree::ptree;
using namespace std;

const string NAME = "name";
const string TRANSACTION_ID = "transaction_id";
const string FILENAME = "filename";
const string DATA = "data";

const string MSG_WRITE = "write";
const string MSG_WRITE_RESPONSE = "write_response";
const string MSG_READ = "read";
const string MSG_READ_RESPONSE = "read_response";
const string MSG_PREPARE = "prepare";
const string MSG_VOTE_ABORT = "vote_abort";
const string MSG_VOTE_COMMIT = "vote_commit";
const string MSG_GLOBAL_ABORT = "global_abort";
const string MSG_GLOBAL_COMMIT = "global_commit";
const string MSG_UNSUPPORTED = "unsupported";

ptree fromString(string s) {
	stringstream ss(s);
	ptree tree;
	read_json(ss, tree);
	return tree;
}

string toString(ptree& tree) {
	stringstream ss;
	write_json(ss, tree);
	return ss.str();
}

string makeReadResponseMessage(string transactionId, string filename, string fileContent) {
	ptree tree;

	tree.put(NAME, MSG_READ_RESPONSE);
	tree.put(TRANSACTION_ID, transactionId);
	tree.put(FILENAME, filename);
	tree.put(DATA, fileContent);

	return toString(tree);
}

string makeSimpleMessage(string messageName, string transactionId) {
	ptree tree;

	tree.put(NAME, messageName);
	tree.put(TRANSACTION_ID, transactionId);

	return toString(tree);
}

string makeUnsupportedMessage(string transactionId) {
	return makeSimpleMessage(MSG_UNSUPPORTED, transactionId);
}

string makeWriteResponseMessage(string transactionId) {
	return makeSimpleMessage(MSG_WRITE_RESPONSE, transactionId);
}

string makeVoteCommitMessage(string transactionId) {
	return makeSimpleMessage(MSG_VOTE_COMMIT, transactionId);
}

string makeVoteAbortMessage(string transactionId) {
	return makeSimpleMessage(MSG_VOTE_ABORT, transactionId);
}