CFILES = *.c

ifndef CROSS_COMPILE
BINARY = writer
COMP = gcc
else
BINARY = writer
COMP = $(CROSS_COMPILE)
endif

all : $(BINARY)

$(BINARY) : $(CFILES)
		$(COMP) -o $@ $^ 
ifdef CROSS_COMPILE
		file $(BINARY) > ../assignments/assignment2/fileresult.txt
endif

.PHONY = clean
clean :
		rm -rf $(BINARY)