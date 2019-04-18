ifneq (${KERNELRELEASE},)
obj-m := sniffer.o
# Иначе вызов из командной строки.
# Тогда вызываем среду компиляции ядра.
else
KERNEL_SOURCE := /usr/src/linux-headers-4.18.0-17-generic/
PWD := $(shell pwd)
default:
	${MAKE} -C ${KERNEL_SOURCE} SUBDIRS=${PWD} modules
clean:
	${MAKE} -C ${KERNEL_SOURCE} SUBDIRS=${PWD} clean
endif
