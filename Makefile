.PHONY:
	all clean
obj-m = port.o
port-objs += CircularBuffer.o tcphook.o
KVERSION = $(shell uname -r)
all: 
	make -C /lib/modules/$(KVERSION)/build M=$(PWD) 
clean: 
	make -C /lib/modules/$(KVERSION)/build M=$(PWD) clean
