obj-m+=led_send.o

EXTRA_CFLAGS := -I/lib/modules/$(shell uname -r)/build/include/xenomai

all::
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

modules:
	@echo "$(CFLAGS)"

clean::
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
