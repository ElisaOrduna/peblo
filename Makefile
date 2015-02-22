
.PHONY : all clean

CFLAGS= -Wall -g -ggdb

all : mm_unittest main

# Compilador #

COMP_INCS=tokenizer.h parser.h environment.h ast.h rename.h unify.h
COMP_OBJS=main.o tokenizer.o parser.o ast.o rename.o unify.o

main : $(COMP_INCS) $(COMP_OBJS)
	g++ -o main $(COMP_OBJS) $(CFLAGS)

%.o : %.cpp $(COMP_INCS)
	g++ -o $@ -c $< $(CFLAGS)

# Memory manager #

MM_INCS=mm.h

mm_unittest : mm_unittest.c mm.o $(MM_INCS)
	gcc -o mm_unittest mm_unittest.c mm.o $(CFLAGS)

mm.o : mm.c save_registers.inc restore_registers.inc $(MM_INCS)
	gcc -c -o mm.o mm.c $(CFLAGS)

save_registers.inc restore_registers.inc : gen_save_restore_registers.py
	python gen_save_restore_registers.py

##

clean :
	rm -f *.o
	rm -f restore_registers.inc
	rm -f save_registers.inc
	rm -f unittest
	rm -f parser
