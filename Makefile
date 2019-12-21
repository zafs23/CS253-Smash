#Define the gcc options
CFLAGS = -Wall -std=c99

#Define the names of the object modules in myLib.a
OBJS = commands.o history.o

#Define a pattern for building an object file from a C file.  Note:  The
#variable $< expands to the name of the first prerequisite.
%.o : %.c
	gcc $(CFLAGS) -c -o $@ $<

#Define the default rule
all: smash

#Define a rule for building the library.  Note:  The variable $? expands to
#just those preprequisites that are out-of-date with the target.
smashLib.a : $(OBJS)
	ar r $@ $?

#Define a rule for building the executable.  Note:  Unlike $?, the $^ variable
#expands to all the prerequisites required to build the target.
smash: smash.o smashLib.a
	gcc -o $@ $^


#--------Build a DEBUG version 
debug: CFLAGS += -DDEBUG -g -O0
debug: smash

#--------Builds a debug version and executes under valgrind
valgrind: debug
	valgrind --track-origins=yes --leak-check=full --show-leak-kinds=all --child-silent-after-fork=yes --trace-children=no smash


#Define a rule to clean-up the mess.  Note:  Projects don't agree on whether
#to delete the library.  The rule below deletes it.
clean:
	rm -f *.o *.a smash *~