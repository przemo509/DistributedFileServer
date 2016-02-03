#include <cstdlib>
#include <iostream>
#include <string>

#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>

#include <boost/asio.hpp> // must be ater logging (otherwise you get linker error, don't know why)

#include "config.h"

using namespace boost::asio;

static const int HEADER_LENGTH = 5;

void assertRequestOk(size_t bytesReceived, int assumedLength, boost::system::error_code& errorCode) {
	if (bytesReceived != assumedLength) {
		throw new std::exception(std::string("Exception while reading request: unexpected length: " + bytesReceived).c_str());
	}
	if (errorCode != 0) {
		// TODO handle eof when server gets down
		throw new std::exception(("Exception while reading request: error: " + errorCode.message()).c_str());
	}
}

std::string getBufAsString(streambuf& buf) {
	std::ostringstream oss;
	oss << &buf;
	return oss.str();
}

int getRequestLength(streambuf& buf) {
	std::string header = getBufAsString(buf);
	int requestLength = atoi(header.c_str());
	return requestLength;
}

std::string makeHeader(int bodySize) {
	char h[HEADER_LENGTH + 1];
	sprintf_s(h, ("%0" + std::to_string(HEADER_LENGTH) + "d").c_str(), bodySize);
	return h;
}

std::string calculateResponse(std::string request)
{
	std::string response("{psadlo server test: response to request: " + request + "}");
	return makeHeader(response.size()) + response;
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
		io_service io_service;
		ip::tcp::acceptor acceptor(io_service, ip::tcp::endpoint(ip::tcp::v4(), config.getPort()));

		int requests = 0;

		while(true)
		{
			requests++;

			// prepaaring socket
			ip::tcp::socket socket(io_service);
			BOOST_LOG_TRIVIAL(debug) << "###############################################################";
			BOOST_LOG_TRIVIAL(debug) << "Waiting for requests on port " << config.getPort();
			acceptor.accept(socket);
			BOOST_LOG_TRIVIAL(debug) << "(" << requests << ") Request arrived";

			// receiving request header (number of remaining bytes)
			boost::system::error_code errorCode;
			streambuf buf;
			BOOST_LOG_TRIVIAL(debug) << "Waiting for request length header...";
			size_t bytesReceived = read(socket, buf, detail::transfer_exactly_t(HEADER_LENGTH), errorCode);
			assertRequestOk(bytesReceived, HEADER_LENGTH, errorCode);
			int requestLength = getRequestLength(buf);
			//buf.consume(bytesReceived); // clear buf
			BOOST_LOG_TRIVIAL(debug) << "Request length header received: " << requestLength;

			// receiving request body
			BOOST_LOG_TRIVIAL(debug) << "Waiting for request body...";
			bytesReceived = read(socket, buf, detail::transfer_exactly_t(requestLength), errorCode);
			assertRequestOk(bytesReceived, requestLength, errorCode);
			std::string request = getBufAsString(buf);
			BOOST_LOG_TRIVIAL(debug) << "Request body received:\n" << request;


			// sending response
			std::string response = calculateResponse(request);
			BOOST_LOG_TRIVIAL(debug) << "Sending response:\n" << response;
			write(socket, buffer(response));
			BOOST_LOG_TRIVIAL(debug) << "Response sent";
		}
	}
	catch (std::exception& e)
	{
		BOOST_LOG_TRIVIAL(fatal) << "Exception: [" << e.what() << "]";
	}

	return EXIT_SUCCESS;
}