KERNEL := /home/shobhit/sandbox/kernel/linux

twbnet-y := twb_net_drv.o twb_net_sysfs.o twb_net_debugfs.o
ccflags-y := -DUSE_TASKLET -I${PWD}

obj-m := twbnet.o twb_net_dev.o notify.o kprobe.o kretprobe.o


default:
	${MAKE} -C ${KERNEL} M=${PWD} modules

clean:
	${MAKE} -C ${KERNEL} M=${PWD} clean
