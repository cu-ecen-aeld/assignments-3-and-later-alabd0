# Author: Abdullah Alabd (abdullah.alabd.else@gmail.com)

CFILES = *.c

ifndef CROSS_COMPILE
TARGET  ?= aesdsocket
CC      ?= gcc
CFLAGS  ?= -g -Wall -Werror
LDFLAGS ?= -pthread
else
TARGET  ?= aesdsocket
CC      ?= $(CROSS_COMPILE)gcc
CFLAGS  ?= -g -Wall -Werror
LDFLAGS ?= -pthread
endif

all : $(TARGET)

$(TARGET) : $(CFILES)
		$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS) 

.PHONY = clean
clean :
		rm -rf $(TARGET) $(TARGET).o