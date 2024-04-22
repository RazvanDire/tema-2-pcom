CC = g++
CFLAGS = -g -Wall

TARGET = server subscriber
SERVER_SRCS = server.cpp utils.cpp server_utils.cpp
SUBSCRIBER_SRCS = tcp_client.cpp utils.cpp
SERVER_OBJS = $(SERVER_SRCS:.cpp=.o)
SUBSCRIBER_OBJS = $(SUBSCRIBER_SRCS:.cpp=.o)

.PHONY: clean
 
all: $(TARGET)

server: $(SERVER_OBJS)
	$(CC) $(CFLAGS) $^ -o $@

subscriber: $(SUBSCRIBER_OBJS)
	$(CC) $(CFLAGS) $^ -o $@

%.o : %.cpp %.hpp
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(TARGET) *.o