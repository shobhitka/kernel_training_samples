KERNEL := /home/shobhit/sandbox/kernel/linux

twbnet-y := twb_net_drv.o twb_net_sysfs.o twb_net_debugfs.o

obj-m := twbnet.o twb_net_dev.o

CFLAGS_twbnet.o := -DUSE_TASKLET

default:
	${MAKE} -C ${KERNEL} M=${PWD} modules

clean:
	${MAKE} -C ${KERNEL} M=${PWD} clean
