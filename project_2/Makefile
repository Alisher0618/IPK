# Compiler and flags
CC = gcc
CFLAGS = -g 

# Source files and headers
SRCS = ipk_server.c receive_msg.c parse_args.c
HEADERS = ipk_server.h receive_msg.h parse_args.h define_macros.h

# Object files
OBJS = $(SRCS:.c=.o)

# Executable name
TARGET = ipk24chat-server

# Targets
all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(TARGET)

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean