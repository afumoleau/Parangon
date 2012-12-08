# Makefile

CC = gcc

CFLAGS= -std=c99 -g
EXEC = par
LDFLAGS=

all: $(EXEC)
 
par: par.o arguments.o fileOperations.o programOptions.o commands.o
  $(CC) $^ $(LDFLAGS) -o $(EXEC)

arguments.o: arguments.c arguments.h
	$(CC) -c $(CFLAGS) arguments.c -o arguments.o

fileOperations.o: fileOperations.c fileOperations.h
	$(CC) -c $(CFLAGS) fileOperations.c -o fileOperations.o

programOptions.o: programOptions.c programOptions.h
	$(CC) -c $(CFLAGS) programOptions.c -o programOptions.o

par.o: main.c arguments.h commands.h
	$(CC) -c $(CFLAGS) main.c -o par.o

commands.o: commands.c commands.h programOptions.h fileHeader.h fileOperations.h
	$(CC) -c $(CFLAGS) commands.c -o commands.o

clean:
	@rm *.o
	
mrproper: clean
	@rm $(EXEC)
	
valgrind: all
	valgrind --leak-check=yes ./$(EXEC)