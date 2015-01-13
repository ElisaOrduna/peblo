
.PHONY : all clean

MM_INCS=mm.h

CFLAGS= -Wall -g -ggdb

all : unittest parser

unittest : unittest.c mm.o $(MM_INCS)
	gcc -o unittest unittest.c mm.o $(CFLAGS)

parser : tokenizer.h tokenizer.cpp parser.cpp
	g++ -o parser parser.cpp tokenizer.cpp $(CFLAGS)

mm.o : mm.c save_registers.inc restore_registers.inc $(MM_INCS)
	gcc -c -o mm.o mm.c $(CFLAGS)

save_registers.inc restore_registers.inc : gen_save_restore_registers.py
	python gen_save_restore_registers.py

clean :
	rm -f *.o
	rm -f restore_registers.inc
	rm -f save_registers.inc
	rm -f unittest
	rm -f parser
