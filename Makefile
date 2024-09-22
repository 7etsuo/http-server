CC = gcc

CFLAGS = -g

SRCS = http_handler.c my_socket.c request.c response.c server.c
OBJS = $(SRCS:.c=.o)
HEADERS = http_handler.h my_http.h my_socket.h request.h response.h

TARGET = server

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

rebuild: clean all

tags:
	ctags -R .

.PHONY: all clean rebuild tags

