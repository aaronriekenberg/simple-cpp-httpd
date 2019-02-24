CXX = c++
CXXFLAGS = -g -Wall -pthread
LDFLAGS = -pthread -lmicrohttpd
SRC = HttpServer.cpp Main.cpp
OBJS=${SRC:.cpp=.o}

all: server

clean:
	rm -f *.o server

server: $(OBJS)
	$(CXX) $(OBJS) $(LDFLAGS) -o $@

depend:
	$(CXX) $(CXXFLAGS) -MM $(SRC) > .makeinclude

include .makeinclude
