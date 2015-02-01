#include <iostream>
#include <sstream>
#include <string>
#include "HttpServer.h"

int main(int argc, char** argv)
{
  const std::string rootHTML =
    "<html><body>\
     What's your name, Sir?<br>\
     <form action=\"/namepost\" method=\"post\">\
     <input name=\"name\" type=\"text\"\
     <input type=\"submit\" value=\" Send \"></form>\
     </body></html>";

  simple_cpp_httpd::HttpServer server;
  server.addGetHandler("/", [=]() { return rootHTML; });
  server.addPostHandler(
    "/namepost",
    [](simple_cpp_httpd::PostKeysAndValues& postKeysAndValues) 
    {
      std::stringstream ss;
      ss << "<html><body><h1>Welcome, " << (postKeysAndValues["name"]) << "</h1></body></html>";
      return ss.str();
    });
  server.start(8888);
  std::cout << "started server" << std::endl;

  getchar();

  return 0;
}
