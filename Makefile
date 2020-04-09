CC = gcc
CFLAGS = -Wall
OBJS = simpledu.o logs.o utils.o
XHDRS = args.h logs.h utils.h
EXEC = simpledu

$(EXEC): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $@

%.o: %.c %.h $(XHDRS)
	$(CC) $(CFLAGS) -c $<

.PHONY : clean
clean :
	-rm $(OBJS) $(EXEC)
