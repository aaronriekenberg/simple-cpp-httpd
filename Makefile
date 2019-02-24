CXX = c++
CXXFLAGS = -g -Wall -pthread -I/usr/local/include
LDFLAGS = -pthread -L/usr/local/lib -lmicrohttpd
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
