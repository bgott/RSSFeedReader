/**
 * File: request-handler.h
 * -----------------------
 * Defines the HTTPRequestHandler class, which proxies and
 * services a single client request.  
 */

#ifndef _http_request_handler_
#define _http_request_handler_

#include <utility>     // for pair
#include <string>      // for string
#include "response.h"
#include "blacklist.h"
#include "cache.h" 
#include "request.h" 
#include "socket++/sockstream.h" // for sockbuf, iosockstream

//using namespace std;

class HTTPRequestHandler {

public:

 	HTTPRequestHandler();

/**
 * Reads the entire HTTP request from the provided socket (the int portion
 * of the connection pair), passes it on to the origin server,
 * waits for the response, and then passes that response back to
 * the client (whose IP address is given by the string portion
 * of the connection pair.
 */

  void serviceRequest(const std::pair<int, std::string>& connection) throw();

private:
	void formRequest(HTTPRequest& request, iosockstream& client_stream, std::string IPaddr);
	void formResponse(HTTPResponse& response, iosockstream& server_stream);
	bool blacklistedRequest(HTTPRequest& request, iosockstream& client_stream, HTTPResponse& response);
	HTTPBlacklist blacklist;
	HTTPCache cache;
};

#endif
