
mm : mm.c save_registers.inc restore_registers.inc
	gcc -o mm mm.c -Wall

save_registers.inc restore_registers.inc : gen_save_restore_registers.py
	python gen_save_restore_registers.py

