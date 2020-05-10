CC = gcc
CFLAGS = -Wall -pthread
DEPS = logs.h timer.h
OBJ = logs.o timer.o
TARGETS = U1 Q1

all: U1 Q1

%.o: %.c $(DEPS)
	@$(CC) $(CFLAGS) -c -o $@ $<
	@echo $@

U1: $(OBJ)
	@$(CC) $(CFLAGS) -o $@ $@.c $(OBJ) -lm
	@echo $@

Q1: $(OBJ)
	@$(CC) $(CFLAGS) -o $@ $@.c $(OBJ) -lm
	@echo $@

clean:
	@rm *.o $(TARGETS)
