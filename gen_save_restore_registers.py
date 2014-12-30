
#registers = 'rax rbx rcx rdx rsi rdi rbp rsp r8 r9 r10 r11 r12 r13 r14 r15'
registers = 'rax rbx rcx rdx rsi rdi r8 r9 r10 r11 r12 r13 r14 r15'
registers = registers.split(' ')
n = len(registers)

f = file('save_registers.inc', 'w')
f.write('\t/*\n')
f.write('\t * Mete en la pila los valores actuales de todos\n')
f.write('\t * los registros de proposito general, para usarlos\n')
f.write('\t * como raices, y asi poder calcular cuales son las\n')
f.write('\t * estructuras alcanzables.\n')
f.write('\t *\n')
f.write('\t * No se guardan los registros rsp ni rbp, porque\n')
f.write('\t * nunca deberian contener handles que apunten al\n')
f.write('\t * heap.\n')
f.write('\t */\n')
f.write('\tuint64 registers[%u];\n' % (n,))
f.write('\t__asm__ __volatile__(\n')
i = 0
for reg in registers:
    f.write('\t\t"movq %%' + reg + ', %' + str(i) + ';\\t\\n"\n')
    i += 1
f.write('\t:')
for i in range(n):
    f.write('\t\t"=m"(registers[' + str(i) + '])' + (',\n' if i < n - 1 else '\n'))
f.write('\t);\n')
f.close()

f = file('restore_registers.inc', 'w')
f.write('\t/*\n')
f.write('\t * Recupera de la pila los valores actualizados de los registros\n')
f.write('\t * de proposito general, porque el proceso de garbage collection\n')
f.write('\t * actualiza los que representan referencias.\n')
f.write('\t */\n')
f.write('\t__asm__ __volatile__(\n')
i = 0
for reg in registers:
    f.write('\t\t"movq %' + str(i) + ', %%' + reg + ';\\t\\n"\n')
    i += 1
f.write('\t:\n')
f.write('\t:\n')
for i in range(n):
    f.write('\t\t"m"(registers[' + str(i) + '])' + (',\n' if i < n - 1 else '\n'))
# clobber
f.write('\t:\n')
f.write('\t\t/* Lista de registros modificados */\n')
f.write('\t\t' + ', '.join(['"' + reg + '"' for reg in registers]) + '\n')
f.write('\t);\n')
f.close()

