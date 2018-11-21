KERNEL := /home/shobhit/sandbox/kernel/linux

obj-m := twb_mem.o kmemleak-test.o

default:
	${MAKE} -C ${KERNEL} M=${PWD} modules

clean:
	${MAKE} -C ${KERNEL} M=${PWD} clean

alltests: test nbtest nbtest_select
test:
	gcc -o test test.c

nbtest:
	gcc -o nbtest nbtest.c

nbtest_select:
	gcc -o nbtest_select nbtest_select.c

testclean:
	rm -f test nbtest nbtest_select syshello

syscall:
	gcc -o syshello syscall.c
