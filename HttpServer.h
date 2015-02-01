#ifndef HTTP_SERVER_H
#define HTTP_SERVER_H

#include <functional>
#include <string>
#include <unordered_map>

namespace simple_cpp_http
{

typedef std::function<std::string()> GetHandler;

typedef std::unordered_map<std::string, std::string> PostKeysAndValues;

typedef std::function<std::string(PostKeysAndValues&)> PostHandler;

class HttpServerImpl;

class HttpServer
{
public:

  HttpServer();

  ~HttpServer();

  void addGetHandler(const std::string& url, GetHandler getHandler);

  void addPostHandler(const std::string& url, PostHandler postHandler);

  void start(int port);

  void stop();

  HttpServer(const HttpServer& rhs) = delete;

  HttpServer& operator=(const HttpServer& rhs) = delete;

private:

  HttpServerImpl* m_pImpl;

};

}

#endif
