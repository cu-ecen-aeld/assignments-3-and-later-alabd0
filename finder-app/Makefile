all: writer

writer.o: writer.c
	$(CC) $(CCFLAGS) -c $^

writer: writer.o
	$(CC) $(LDFLAGS) $^ -o $@

clean :
	rm -f writer writer.o