# Compiler and flags
CC = gcc
CFLAGS = -g 

# Source files and headers
SRCS = ipk24chat-client.c parse_args.c tcp_send.c tcp_receive.c udp.c udp_send.c udp_receive.c
HEADERS = ipk24chat-client.h parse_args.h tcp_send.h tcp_receive.h udp.h udp_send.h udp_receive.h define_macros.h

# Object files
OBJS = $(SRCS:.c=.o)

# Executable name
TARGET = ipk24chat-client

# Targets
all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(TARGET)

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean