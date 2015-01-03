
.PHONY : clean

INCS=mm.h

CFLAGS= -Wall -g -ggdb

unittest : unittest.c mm.o $(INCS)
	gcc -o unittest unittest.c mm.o $(CFLAGS)

mm.o : mm.c save_registers.inc restore_registers.inc $(INCS)
	gcc -c -o mm.o mm.c $(CFLAGS)

save_registers.inc restore_registers.inc : gen_save_restore_registers.py
	python gen_save_restore_registers.py

clean :
	rm -f *.o
	rm -f restore_registers.inc
	rm -f save_registers.inc
	rm -f unittest

