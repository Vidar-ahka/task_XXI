CC      = gcc
CFLAGS  = -Wall -Wextra -std=c11 -O2
LDFLAGS = -pthread
TARGET  = cpu_monitor
SRCS    = main.c udp_client.c cpu_usage.c

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(SRCS) 
	$(CC) $(CFLAGS) -o $@ $(SRCS) $(LDFLAGS)

clean:
	rm -f $(TARGET) 