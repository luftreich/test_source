TMP=*.o *.mod.c .*.cmd *.markers *.order *.symvers .tmp_versions

obj-m :=create_sys.o
KERNELDIR := /home/vincent/samba/Vanilla_Latte/Board/trunk/2.0/kmodules/include/kernel
all:
	make -C $(KERNELDIR) M=$(PWD) modules ARCH=arm CROSS_COMPILE=arm-brcm-linux-uclibcgnueabi-
	rm -fr $(TMP)	
	cp *.ko $(ROOT)/build/bin
clean:
	rm -rf *.o *.ko *.mod.c .*.cmd *.markers *.order *.symvers .tmp_versions
