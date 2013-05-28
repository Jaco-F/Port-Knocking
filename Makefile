.PHONY:
	all clean
obj-m = test.o
test-objs += CircularBuffer.o provaTcp.o
KVERSION = $(shell uname -r)
all: 
	make -C /lib/modules/$(KVERSION)/build M=$(PWD) 
clean: 
	make -C /lib/modules/$(KVERSION)/build M=$(PWD) clean
