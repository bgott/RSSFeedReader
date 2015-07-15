/**
 * File: request-handler.cc
 * ------------------------
 * Provides the implementation for the HTTPRequestHandler class.
 */

#include "request-handler.h"
#include <iostream>              // for flush
#include <netdb.h>                // for gethostbyname
#include <sys/socket.h>           // for socket, AF_INET
#include <sys/types.h>            // for SOCK_STREAM
#include <cstring>                // for memset

using namespace std;

const int kSuccessfulResponseCode = 200;
const int kForbidden = 403;
const int kBadRequest = 400;
const int kProxyFailure = 510;
const int kBufferSize = 64000;


/**
 * Constant: kClientSocketError
 * ----------------------------
 * Sentinel used to communicate that a connection
 * to a remote server could not be made.
 */

const int kClientSocketError = -1;

HTTPRequestHandler::HTTPRequestHandler() : blacklist("blocked-domains.txt") {}

/**
 * Function: createClientSocket
 * ----------------------------
 * Establishes a connection to the provided host
 * on the specified port, and returns a bidirectional 
 * socket descriptor that can be used for two-way
 * communication with the service running on the
 * identified host's port. Usage of the reentrant 
 * method was inspired by user cnicutar on stackoverflow
 */

int createClientSocket(const string& host, 
		       unsigned short port) {
	int r, error;
	int len = kBufferSize;
	char buf[kBufferSize];
	struct hostent hostbuf;
	struct hostent *result;

	r = gethostbyname_r(host.c_str(), &hostbuf, buf, len, &result, &error);
  	if (result == NULL || r != 0) return kClientSocketError;

  	int s = socket(AF_INET, SOCK_STREAM, 0);
  	if (s < 0) return kClientSocketError;
  
  	struct sockaddr_in serverAddress;
  	memset(&serverAddress, 0, sizeof(serverAddress));
  	serverAddress.sin_family = AF_INET;
  	serverAddress.sin_port = htons(port);
  	serverAddress.sin_addr.s_addr = 
    	((struct in_addr *)result->h_addr)->s_addr;
  
  	if (connect(s, (struct sockaddr *) &serverAddress, 
		sizeof(serverAddress)) != 0) {
    		close(s);
    	return kClientSocketError;
  	}
  
  	return s;
}


void HTTPRequestHandler::formRequest(HTTPRequest& request, iosockstream& client_stream, 
									string IPaddr) {
 	request.ingestRequestLine(client_stream);
 	request.ingestHeader(client_stream, IPaddr);
 	request.ingestPayload(client_stream);
}

bool HTTPRequestHandler::blacklistedRequest(HTTPRequest& request, iosockstream& client_stream, 
						HTTPResponse& response) {
	if (!blacklist.serverIsAllowed(request.getServer())) {
 		response.setResponseCode(kForbidden);
 		response.setProtocol(request.getProtocol());
 		response.setPayload("Forbidden Content");
 		client_stream << response << flush;
 		return true;
 	}
 	return false;
}

void HTTPRequestHandler::formResponse(HTTPResponse& response, iosockstream& server_stream) {
 	response.ingestResponseHeader(server_stream);
 	response.ingestPayload(server_stream);
}


/**
 * Services a request by posting a placeholder payload.  The implementation
 * of this method will change dramatically as you work toward your final milestone.
 */

void HTTPRequestHandler::serviceRequest(const pair<int, string>& connection) throw() {
 	sockbuf sb(connection.first);
 	iosockstream client_stream(&sb);

 	HTTPRequest request; 
 	HTTPResponse response;

 	try { 
 		formRequest(request, client_stream, connection.second);
 
 		cout << request.getMethod() << " " << request.getURL() << endl;
 		
 		if (blacklistedRequest(request, client_stream, response)) return;

 		if (cache.containsCacheEntry_ts(request, response)) {
 			client_stream << response << flush;
 			return;
 		} 

 		int clientSocket = createClientSocket(request.getServer(), request.getPort());
 		if(clientSocket == kClientSocketError) {
 			cerr << "Cannot connect to host named \"" 
	 			<< connection.second << "\"" << endl;
 			return;
 		}
 		sockbuf sb2(clientSocket);
 		iosockstream server_stream(&sb2);
		server_stream << request << flush;

		formResponse(response, server_stream);

 		if (cache.shouldCache(request, response)) cache.cacheEntry_ts(request, response);
 		client_stream << response << flush;

 	} catch (const HTTPBadRequestException &bre) {
 		response.setPayload(bre.what());
 		response.setResponseCode(kBadRequest);
 		client_stream << response << flush;
 	} catch (const HTTPProxyException& hpe) {
 		response.setPayload(hpe.what());
 		response.setResponseCode(kProxyFailure);
 		client_stream << response << flush;
 	}
}
