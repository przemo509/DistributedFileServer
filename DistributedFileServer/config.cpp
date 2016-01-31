#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/log/trivial.hpp>

#include "config.h"

config::config(const std::string& pathToFile)
{
	BOOST_LOG_TRIVIAL(info) << "Reading server configuration file [" << pathToFile << "]:";
	boost::property_tree::ptree tree;
	boost::property_tree::read_json(pathToFile, tree);

	autoMode = tree.get<bool>("auto_mode");
	BOOST_LOG_TRIVIAL(info) << " - server auto mode is set to [" << autoMode << "]";
	port = tree.get<int>("port");
	BOOST_LOG_TRIVIAL(info) << " - server listening port is [" << port << "]";
	logFileFullPath = tree.get<std::string>("log_file_full_path");
	BOOST_LOG_TRIVIAL(info) << " - log file is [" << logFileFullPath << "]";
	sharedDirectoryFullPath = tree.get<std::string>("shared_directory_full_path");
	BOOST_LOG_TRIVIAL(info) << " - shared direcotry is [" << sharedDirectoryFullPath << "]";

	check();
	BOOST_LOG_TRIVIAL(info) << "Reading server configuration file - OK";
}

void config::check() {
	// log file
	if (!boost::filesystem::exists(logFileFullPath)) {
		throw "Log file [" + logFileFullPath + "] does not exist!";
	}
	if (!boost::filesystem::is_regular_file(logFileFullPath)) {
		throw "Path [" + logFileFullPath + "] is not a file!";
	}

	// shared direcotry
	if (!boost::filesystem::exists(sharedDirectoryFullPath)) {
		throw "Shared directory [" + sharedDirectoryFullPath + "] does not exist!";
	}
	if (!boost::filesystem::is_directory(sharedDirectoryFullPath)) {
		throw "Path [" + sharedDirectoryFullPath + "] is not a directory!";
	}
}


config::~config()
{
}
