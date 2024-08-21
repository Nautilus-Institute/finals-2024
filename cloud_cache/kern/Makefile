KERNEL_SRC := /linux-6.2.14

obj-m += jscache.o
ccflags-y := -std=gnu99 -Wno-declaration-after-statement

all:
	$(MAKE) -C $(KERNEL_SRC) M=$(PWD) modules

clean:
	$(MAKE) -C $(KERNEL_SRC) M=$(PWD) clean

.PHONY: all clean headers
