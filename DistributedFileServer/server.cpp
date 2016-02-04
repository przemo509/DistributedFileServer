#include <cstdlib>
#include <iostream>
#include <string>

#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>

#include <boost/asio.hpp> // must be ater logging (otherwise you get linker error, don't know why)

#include "config.h"
#include "controller.h"

using namespace boost::asio;
using namespace std;

static const int HEADER_LENGTH = 5;

void assertRequestOk(size_t bytesReceived, int assumedLength, boost::system::error_code& errorCode) {
	if (errorCode != 0) {
		// TODO handle eof when server gets down
		throw exception(("Exception while reading request: error: " + errorCode.message()).c_str());
	}
	if (bytesReceived != assumedLength) {
		throw exception(string("Exception while reading request: unexpected length: " + bytesReceived).c_str());
	}
}

string getBufAsString(boost::asio::streambuf& buf) {
	ostringstream oss;
	oss << &buf;
	return oss.str();
}

int getRequestLength(boost::asio::streambuf& buf) {
	string header = getBufAsString(buf);
	int requestLength = atoi(header.c_str());
	return requestLength;
}

string makeHeader(int bodySize) {
	char h[HEADER_LENGTH + 1];
	sprintf_s(h, ("%0" + to_string(HEADER_LENGTH) + "d").c_str(), bodySize);
	return h;
}

string addHeader(string& request) {
	return makeHeader(request.size()) + request;
}

int main(int argc, char* argv[])
{
	// global logging filter
	boost::log::core::get()->set_filter(boost::log::trivial::severity >= boost::log::trivial::info);

	if (argc != 2) {
		cout << "Usage: " << argv[0] << " config_file_full_path" << endl;
		exit(EXIT_FAILURE);
	}
	string configFileFullPath = argv[1];

	try
	{
		config config(configFileFullPath);
		controller controller(config);

		// preparing service
		io_service io_service;
		ip::tcp::acceptor acceptor(io_service, ip::tcp::endpoint(ip::tcp::v4(), config.getPort()));

		int requests = 0;

		while(true)
		{
			try
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
				boost::asio::streambuf buf;
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
				string request = getBufAsString(buf);
				BOOST_LOG_TRIVIAL(debug) << "Request body received:\n" << request;

				// sending response
				string response = controller.calculateResponse(request);
				BOOST_LOG_TRIVIAL(debug) << "Sending response:\n" << response;
				write(socket, buffer(addHeader(response)));
				BOOST_LOG_TRIVIAL(debug) << "Response sent";
			}
			catch (exception& e)
			{
				BOOST_LOG_TRIVIAL(fatal) << "Request processing exception: [" << e.what() << "]";
			}
			catch (...)
			{
				BOOST_LOG_TRIVIAL(fatal) << "Unknown request processing exception";
			}
		}
	}
	catch (exception& e)
	{
		BOOST_LOG_TRIVIAL(fatal) << "Global exception: [" << e.what() << "]";
	} 
	catch (...)
	{
		BOOST_LOG_TRIVIAL(fatal) << "Unknown global exception";
	}

	return EXIT_SUCCESS;
}