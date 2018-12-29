KERNEL := /home/shobhit/sandbox/kernel/linux
#KERNEL := /home/shobhit/sandbox/trainings/rPI/kernel

obj-m := twb_mem.o kmemleak-test.o oops.o

default:
	${MAKE} -C ${KERNEL} M=${PWD} modules
	#${MAKE} ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- -C ${KERNEL} M=${PWD} modules

clean:
	${MAKE} -C ${KERNEL} M=${PWD} clean
	#${MAKE} ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- -C ${KERNEL} M=${PWD} clean

alltests: test nbtest nbtest_select mmap syscall clone nice
test:
	gcc -o test test.c

nbtest:
	gcc -o nbtest nbtest.c

nbtest_select:
	gcc -o nbtest_select nbtest_select.c

testclean:
	rm -f test nbtest nbtest_select syshello clone nice policy mmap trace

syscall:
	gcc -o syshello syscall.c

clone:
	gcc -o clone clone.c

nice:
	gcc -o nice nice.c
	gcc -o policy policy.c

mmap:
	gcc -o mmap mmap-test.c

trace:
	gcc -o trace trace.c
