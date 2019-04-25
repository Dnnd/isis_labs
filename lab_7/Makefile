ifneq (${KERNELRELEASE},)
obj-m := sniffer.o
# Иначе вызов из командной строки.
# Тогда вызываем среду компиляции ядра.
else
KERNEL_SOURCE := /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)
default:
	${MAKE} -C ${KERNEL_SOURCE} SUBDIRS=${PWD} modules
clean:
	${MAKE} -C ${KERNEL_SOURCE} SUBDIRS=${PWD} clean
endif
