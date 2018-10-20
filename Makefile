KERNEL := /home/shobhit/sandbox/kernel/linux

# if KERNELRELEASE is defined, we've been invoked from the
# kernel build system and can use its language.
ifneq (${KERNELRELEASE},)
	obj-m := hello.o
# Otherwise we were called directly from the command line.
# Invoke the kernel build system.
else
default:
	${MAKE} -C ${KERNEL} M=${PWD} modules

clean:
	${MAKE} -C ${KERNEL} M=${PWD} clean
endif
