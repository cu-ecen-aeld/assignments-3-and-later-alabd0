# Author: Abdullah Alabd (abdullah.alabd.else@gmail.com)

CFILES = *.c

ifndef CROSS_COMPILE
BINARY  = aesdsocket
CC      = gcc
CFLAGS  = -Wall -Werror -pthread -g
else
BINARY  = aesdsocket
CC      = $(CC)
CFLAGS  = -Wall -Werror -pthread -g
endif

all : $(BINARY)

$(BINARY) : $(CFILES)
		$(CC)  $(CFLAGS) -o $@ $^ $(LDFLAGS) 

.PHONY = clean
clean :
		rm -rf $(BINARY) $(BINARY).o