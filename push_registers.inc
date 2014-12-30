	/*
     * Mete en la pila los valores actuales de todos
     * los registros de proposito general, para usarlos
     * como raices, y asi poder calcular cuales son las
     * estructuras alcanzables.
     */
	uint64 registers[16];
	__asm__ __volatile__(
		"movq %%rax, %0;\t\n"
		"movq %%rbx, %1;\t\n"
		"movq %%rcx, %2;\t\n"
		"movq %%rdx, %3;\t\n"
		"movq %%rsi, %4;\t\n"
		"movq %%rdi, %5;\t\n"
		"movq %%rbp, %6;\t\n"
		"movq %%rsp, %7;\t\n"
		"movq %%r8, %8;\t\n"
		"movq %%r9, %9;\t\n"
		"movq %%r10, %10;\t\n"
		"movq %%r11, %11;\t\n"
		"movq %%r12, %12;\t\n"
		"movq %%r13, %13;\t\n"
		"movq %%r14, %14;\t\n"
		"movq %%r15, %15;\t\n"
	:
		"=m"(registers[0]),
		"=m"(registers[1]),
		"=m"(registers[2]),
		"=m"(registers[3]),
		"=m"(registers[4]),
		"=m"(registers[5]),
		"=m"(registers[6]),
		"=m"(registers[7]),
		"=m"(registers[8]),
		"=m"(registers[9]),
		"=m"(registers[10]),
		"=m"(registers[11]),
		"=m"(registers[12]),
		"=m"(registers[13]),
		"=m"(registers[14]),
		"=m"(registers[15])
	);
