#include <cstdlib>
#include <iostream>
#include <string>

#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>

#include <boost/asio.hpp> // must be ater logging (otherwise you get linker error, don't know why)

#include "config.h"

static const std::string DELIMETER = "\n";

std::string calculateResponse(std::string request)
{
	return "ggggg response to request: " + request;
}

int main(int argc, char* argv[])
{
	// global logging filter
	boost::log::core::get()->set_filter(boost::log::trivial::severity >= boost::log::trivial::trace);

	if (argc != 2) {
		std::cout << "Usage: " << argv[0] << " config_file_full_path" << std::endl;
		exit(EXIT_FAILURE);
	}
	std::string configFileFullPath = argv[1];

	try
	{
		config config(configFileFullPath);

		// preparing service
		using namespace boost::asio;
		io_service io_service;
		ip::tcp::acceptor acceptor(io_service, ip::tcp::endpoint(ip::tcp::v4(), config.getPort()));

		int requests = 0;

		while(true)
		{
			requests++;

			// prepaaring socket
			ip::tcp::socket socket(io_service);
			BOOST_LOG_TRIVIAL(debug) << "\tWaiting for requests on port " << config.getPort();
			acceptor.accept(socket);
			BOOST_LOG_TRIVIAL(debug) << "(" << requests << ") Request arrived";

			// receiving request
			boost::system::error_code errorCode;
			boost::asio::streambuf buf;
			size_t bytesReceived = read_until(socket, buf, DELIMETER, errorCode);
			if (errorCode != 0) {
				// TODO handle eof when client gets down
				BOOST_LOG_TRIVIAL(fatal) << "\tException: [" << errorCode.message() << "]";
			}
			std::string request;
			std::getline(std::istream(&buf), request);
			BOOST_LOG_TRIVIAL(debug) << "\tRequest received: " << request;

			// send response
			std::string response = calculateResponse(request);
			write(socket, buffer(response + DELIMETER));
			BOOST_LOG_TRIVIAL(debug) << "\tResposne sent: " << response;
		}
	}
	catch (std::exception& e)
	{
		BOOST_LOG_TRIVIAL(fatal) << "Exception: [" << e.what() << "]";
	}

	return EXIT_SUCCESS;
}