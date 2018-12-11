KERNEL := /home/shobhit/sandbox/kernel/linux

obj-m := twb_mem.o kmemleak-test.o oops.o

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
	rm -f test nbtest nbtest_select syshello clone nice policy

syscall:
	gcc -o syshello syscall.c

clone:
	gcc -o clone clone.c

nice:
	gcc -o nice nice.c
	gcc -o policy policy.c
