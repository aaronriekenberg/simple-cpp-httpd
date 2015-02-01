#include <functional>
#include <iostream>
#include <unordered_map>
#include <string>
#include <string.h>
#include <sstream>
#include <vector>
#include <microhttpd.h>
#include "HttpServer.h"

namespace
{

const size_t POST_BUFFER_SIZE = 1024;

}

namespace simple_cpp_httpd
{

class HttpServerImpl
{
public:

  HttpServerImpl()
  {

  }

  ~HttpServerImpl()
  {
    stop();
  }

  void addGetHandler(const std::string& url, GetHandler getHandler)
  {
    m_urlToGetHandler[url] = getHandler;
  }

  void addPostHandler(const std::string& url, PostHandler postHandler)
  {
    m_urlToPostHandler[url] = postHandler;
  }

  void start(int port)
  {
    m_pDaemon = MHD_start_daemon (
      MHD_USE_EPOLL_INTERNALLY_LINUX_ONLY | MHD_USE_DUAL_STACK,
      port, 0, 0,
      &HttpServerImpl::handleRequest, this,
      MHD_OPTION_NOTIFY_COMPLETED, &HttpServerImpl::requestCompleted, this,
      MHD_OPTION_END);
    if (!m_pDaemon)
    {
      std::cout << "MHD_start_daemon error" << std::endl;
      abort();
    }
  }

  void stop()
  {
    if (m_pDaemon)
    {
      MHD_stop_daemon(m_pDaemon);
    }
    m_pDaemon = 0;
  }

  HttpServerImpl(const HttpServerImpl& rhs) = delete;

  HttpServerImpl& operator=(const HttpServerImpl& rhs) = delete;

private:

  static int handleRequest(void *cls, struct MHD_Connection *connection,
                            const char *url, const char *method,
                            const char *version, const char *upload_data,
                            size_t *upload_data_size, void **con_cls)
  {
    std::cout << "handleRequest url = " << url << " method = " << method << std::endl;
    HttpServerImpl& server = *(static_cast<HttpServerImpl*>(cls));
    if (strcmp(method, "GET") == 0)
    {
      return server.handleGet(connection, url);
    }
    else if (strcmp(method, "POST") == 0)
    {
      return server.handlePost(connection, url, upload_data, upload_data_size, con_cls);
    }
    return server.returnResponse(connection, "Unknown method", MHD_HTTP_METHOD_NOT_ALLOWED);
  }

  static int requestCompleted(void *cls, struct MHD_Connection *connection,
                              void **con_cls, enum MHD_RequestTerminationCode toe)
  {
    PostConnectionInfo* pConnectionInfo = static_cast<PostConnectionInfo*>(*con_cls);
    delete pConnectionInfo;
    (*con_cls) = 0;
    return MHD_YES;
  }

  int handleGet(MHD_Connection* connection, const std::string& url)
  {
    auto iter = m_urlToGetHandler.find(url);
    if (iter == m_urlToGetHandler.end())
    {
      return returnResponse(connection, "Unknown GET location", MHD_HTTP_NOT_FOUND);
    }

    const std::string response = (iter->second)();
    return returnResponse(connection, response);
  }

  static int iteratePost(void *coninfo_cls, enum MHD_ValueKind kind, const char *key,
                         const char *filename, const char *content_type,
                         const char *transfer_encoding, const char *data, uint64_t off,
                         size_t size)
  {
    PostConnectionInfo& connectionInfo = *(static_cast<PostConnectionInfo*>(coninfo_cls));

    const std::string keyString(key);
    connectionInfo.postKeysAndValues[keyString] += data;

    return MHD_YES;
  }

  int handlePost(MHD_Connection* connection, const std::string& url,
                 const char* upload_data, size_t* upload_data_size,
                 void** con_cls)
  {
    auto iter = m_urlToPostHandler.find(url);
    if (iter == m_urlToPostHandler.end())
    {
      return returnResponse(connection, "Unknown POST location", MHD_HTTP_NOT_FOUND);
    }

    if (0 == (*con_cls))
    {
      PostConnectionInfo* pConnectionInfo = new PostConnectionInfo;
      pConnectionInfo->postProcessor =
        MHD_create_post_processor(connection, POST_BUFFER_SIZE,
                                  &HttpServerImpl::iteratePost, pConnectionInfo);
      if (0 == pConnectionInfo->postProcessor)
      {
        delete pConnectionInfo;
        return MHD_NO;
      }

      (*con_cls) = pConnectionInfo;

      return MHD_YES;
    }
    else
    {
      PostConnectionInfo& connectionInfo = *(static_cast<PostConnectionInfo*>(*con_cls));

      if ((*upload_data_size) != 0)
      {
        MHD_post_process (connectionInfo.postProcessor, upload_data,
                          *upload_data_size);
        *upload_data_size = 0;

        return MHD_YES;
      }
      else
      {
        const std::string response = (iter->second)(connectionInfo.postKeysAndValues);
        return returnResponse(connection, response);
      }
    }
  }

  int returnResponse(MHD_Connection* connection, const std::string& responseString,
                     int responseType = MHD_HTTP_OK)
  {
    MHD_Response* response =
      MHD_create_response_from_buffer(
        responseString.size(),
        (void*)(responseString.c_str()),
        MHD_RESPMEM_MUST_COPY);
    if (!response)
    {
      return MHD_NO;
    }

    const int ret = MHD_queue_response (connection, responseType, response);
    MHD_destroy_response (response);
    return ret;
  }

  struct PostConnectionInfo
  {
    PostKeysAndValues postKeysAndValues;
    MHD_PostProcessor* postProcessor = 0;

    ~PostConnectionInfo()
    {
      if (postProcessor)
      {
        MHD_destroy_post_processor(postProcessor);
      }
      postProcessor = 0;
    }
  };

  MHD_Daemon* m_pDaemon = 0;

  std::unordered_map<std::string, GetHandler> m_urlToGetHandler;

  std::unordered_map<std::string, PostHandler> m_urlToPostHandler;

};

HttpServer::HttpServer() :
  m_pImpl(new HttpServerImpl)
{

}

HttpServer::~HttpServer()
{
  delete m_pImpl;
}

void HttpServer::addGetHandler(const std::string& url, GetHandler getHandler)
{
  m_pImpl->addGetHandler(url, getHandler);
}

void HttpServer::addPostHandler(const std::string& url, PostHandler postHandler)
{
  m_pImpl->addPostHandler(url, postHandler);
}

void HttpServer::start(int port)
{
  m_pImpl->start(port);
}

void HttpServer::stop()
{
  m_pImpl->stop();
}

}
