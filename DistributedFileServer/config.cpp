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
	BOOST_LOG_TRIVIAL(info) << " - server auto mode is set to   " << autoMode;
	port = tree.get<int>("port");
	BOOST_LOG_TRIVIAL(info) << " - server listening port is     " << port;
	logFileFullPath = tree.get<std::string>("log_file_full_path");
	BOOST_LOG_TRIVIAL(info) << " - log file is                  " << logFileFullPath;
	sharedDirectoryFullPath = tree.get<std::string>("shared_directory_full_path");
	BOOST_LOG_TRIVIAL(info) << " - shared directory is          " << sharedDirectoryFullPath;

	check();
	BOOST_LOG_TRIVIAL(info) << "Reading server configuration file - OK";
}

void config::check() {
	// log file
	if (!boost::filesystem::exists(logFileFullPath)) {
		throw std::exception(("Log file [" + logFileFullPath + "] does not exist!").c_str());
	}
	if (!boost::filesystem::is_regular_file(logFileFullPath)) {
		throw std::exception(("Path [" + logFileFullPath + "] is not a file!").c_str());
	}

	// shared directory
	if (!boost::filesystem::exists(sharedDirectoryFullPath)) {
		throw std::exception(("Shared directory [" + sharedDirectoryFullPath + "] does not exist!").c_str());
	}
	if (!boost::filesystem::is_directory(sharedDirectoryFullPath)) {
		throw std::exception(("Path [" + sharedDirectoryFullPath + "] is not a directory!").c_str());
	}
}


config::~config()
{
}
