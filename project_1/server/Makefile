# Compiler and flags
CC = gcc
CFLAGS = -g 

# Source files and headers
SRCS = server.c server_tcp.c server_udp.c
HEADERS = server.h server_tcp.h server_udp.h

# Object files
OBJS = $(SRCS:.c=.o)

# Executable name
TARGET = server

# Targets
all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(TARGET)

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean