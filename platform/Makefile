KERNEL := /home/shobhit/sandbox/kernel/linux

obj-m := twb_net_drv.o twb_net_dev.o
CFLAGS_twb_net_drv.o := -DUSE_TASKLET

default:
	${MAKE} -C ${KERNEL} M=${PWD} modules

clean:
	${MAKE} -C ${KERNEL} M=${PWD} clean
