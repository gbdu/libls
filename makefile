#

NAME = libls 
CC = gcc
FLAGS = -Wall -g 
OUTPUT = lstest 

OBJS = main.o ls_store.o libls.o ls_print.o ls_deb.o
SRCS = $(OBJS, .o=.c)

# Executable
$(OUTPUT): $(OBJS)
	$(CC) $(OBJS) $(FLAGS) -o $(OUTPUT)
	
# Files
.c.o:
	$(CC) $(FLAGS) -c $*.c -o $*.o

clean:
	rm $(OBJS) 

