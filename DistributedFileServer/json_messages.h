#pragma once

#include <string>
#include <boost/property_tree/ptree.hpp>

static const std::string NAME = "name";
static const std::string TRANSACTION_ID = "transaction_id";
static const std::string FILENAME = "filename";
static const std::string DATA = "data";

static const std::string MSG_WRITE = "write";
static const std::string MSG_WRITE_RESPONSE = "write_response";
static const std::string MSG_READ = "read";
static const std::string MSG_READ_RESPONSE = "read_response";
static const std::string MSG_PREPARE = "prepare";
static const std::string MSG_VOTE_ABORT = "vote_abort";
static const std::string MSG_VOTE_COMMIT = "vote_commit";
static const std::string MSG_GLOBAL_ABORT = "global_abort";
static const std::string MSG_GLOBAL_COMMIT = "global_commit";
static const std::string MSG_UNSUPPORTED = "unsupported";

boost::property_tree::ptree fromString(std::string s);
std::string toString(boost::property_tree::ptree& tree);
std::string makeReadResponseMessage(std::string transactionId, std::string filename, std::string fileContent);
std::string makeSimpleMessage(std::string messageName, std::string transactionId);
std::string makeUnsupportedMessage(std::string transactionId);
std::string makeWriteResponseMessage(std::string transactionId);
std::string makeVoteCommitMessage(std::string transactionId);
std::string makeVoteAbortMessage(std::string transactionId);