/**
 * File: cache.h
 * -------------
 * Defines a class to help manage an 
 * HTTP response cache.
 */

#ifndef _http_cache_
#define _http_cache_

#include <string>
#include <map>
#include <memory>
#include <mutex>
#include "request.h"
#include "response.h"

class HTTPCache {
 public:

/**
 * Constructs the HTTPCache object.
 */

  HTTPCache();

/*
 *  Threadsafe wrappers for the cache manipulation methods
 */

  bool containsCacheEntry_ts(const HTTPRequest& request, HTTPResponse& response);
  void cacheEntry_ts(const HTTPRequest& request, const HTTPResponse& response);

/*
 * shouldCache does not access the filesystem manipulated by the cache functions, 
 * thus it is not thread unsafe
 */ 
  bool shouldCache(const HTTPRequest& request, const HTTPResponse& response) const;
  
 private:
  std::mutex map_lock;
  std::map<std::string, std::unique_ptr<std::mutex> > cache_locks;
  void aquireLock(const HTTPRequest& request);
  void releaseLock(const HTTPRequest& request);
  
  bool containsCacheEntry(const HTTPRequest& request, HTTPResponse& response) const;
  void cacheEntry(const HTTPRequest& request, const HTTPResponse& response);

  std::string hashRequest(const HTTPRequest& request) const;
  std::string serializeRequest(const HTTPRequest& request) const;
  bool cacheEntryExists(const std::string& filename) const;
  std::string getRequestHashCacheEntryName(const std::string& requestHash) const;
  void ensureDirectoryExists(const std::string& directory, bool empty = false) const;
  std::string getExpirationTime(int ttl) const;
  bool cachedEntryIsValid(const std::string& cachedFileName) const;
  
  std::string cacheDirectory;
};

#endif
