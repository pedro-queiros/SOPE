CC = gcc
CFLAGS = -Wall
OBJS = simpledu.o
XHDRS = args.h
EXEC = simpledu

$(EXEC): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $@

%.o: %.c %.h $(XHDRS)
	$(CC) $(CFLAGS) -c $<

.PHONY : clean
clean :
	-rm $(OBJS) $(EXEC)