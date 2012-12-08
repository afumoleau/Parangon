# Makefile

ifeq ($(OS),Windows_NT)
	SEPARATOR = &
else
	SEPARATOR = ;
endif

#
# Paths
#
PREFIX = 
SRCDIR = $(PREFIX)src
BINDIR = $(PREFIX)bin
OBJDIR = $(PREFIX)obj

#
# Build commands and options
#
CC = gcc
CFLAGS= -std=gnu99 -g -Wall -Werror
EXEC = par

#
# Files variables
#
EXECUTABLE= par
SRC = main.c arguments.c fileOperations.c programOptions.c commands.c archiveOperations.c
OBJS = $(SRC:.c=.o)
OBJFILES = $(patsubst %,$(OBJDIR)/%,$(OBJS))

all : executable

#
# Directories
#
ifeq ($(OS),Windows_NT)
$(BINDIR) :
	@IF exist $@ ( echo ) ELSE ( mkdir $@ && echo $@ created)
$(OBJDIR) :
	@IF exist $@ ( echo ) ELSE ( mkdir $@ && echo $@ created)
else
$(BINDIR) :
	test -d $@ | mkdir -p $@
$(OBJDIR) :
	test -d $@ | mkdir -p $@
endif

#
# Main build targets
#
executable : $(BINDIR)/$(EXECUTABLE)

$(BINDIR)/$(EXECUTABLE) : $(OBJS)
	$(CC) $(LDFLAGS) $(patsubst %,$(OBJDIR)/%,$^) $(LDLIBS) -o $@

%.o : $(SRCDIR)/%.c $(OBJDIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $(OBJDIR)/$@

arguments.o: $(SRCDIR)/arguments.c $(SRCDIR)/arguments.h
fileOperations.o: $(SRCDIR)/fileOperations.c $(SRCDIR)/fileOperations.h
programOptions.o: $(SRCDIR)/programOptions.c $(SRCDIR)/programOptions.h
par.o: $(SRCDIR)/main.c $(SRCDIR)/arguments.h $(SRCDIR)/commands.h
commands.o: $(SRCDIR)/commands.c $(SRCDIR)/commands.h $(SRCDIR)/programOptions.h $(SRCDIR)/fileHeader.h $(SRCDIR)/fileOperations.h
archiveOperations.o : $(SRCDIR)/archiveOperations.c $(SRCDIR)/archiveOperations.h $(SRCDIR)/fileOperations.h $(SRCDIR)/fileHeader.h

clean : 
	rm -rf $(OBJDIR)/* $(BINDIR)/$(EXECUTABLE)