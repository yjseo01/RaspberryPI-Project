TARGET = daemon_server
CC = gcc
CFLAGS = -Wall -O2
#LDFLAGS = -lpthread -lwiringPi
LDFLAGS = -lpthread

SRCS = main.c thd_func.c
OBJS = $(SRCS:.c=.o)

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

$%.o: $(SRCDIR)/%.c daemon_server.h
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(TARGET) $(OBJS)
