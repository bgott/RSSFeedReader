/**
 * File: scheduler.cc
 * ------------------
 * Presents the implementation of the HTTPProxyScheduler class.
 */

#include "scheduler.h"
#include <utility>      // for make_pair
#include <thread>
using namespace std;

void HTTPProxyScheduler::scheduleRequest(int connectionfd, const string& clientIPAddress) {
	pair<int, const string> p = make_pair(connectionfd, clientIPAddress);
	
	workers.schedule([this, p] {
		handler.serviceRequest(p);
	});

}
