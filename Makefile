KERNEL := /home/shobhit/sandbox/kernel/linux

obj-m := twb_mem.o

default:
	${MAKE} -C ${KERNEL} M=${PWD} modules

clean:
	${MAKE} -C ${KERNEL} M=${PWD} clean
