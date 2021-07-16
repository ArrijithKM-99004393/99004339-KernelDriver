obj-m += hello.o

UBUNTU = /lib/modules/`uname -r`/build
KSRC = /home/arrijith/KSRC

all:
	make -C ${UBUNTU} M=${PWD} modules #ARCH=arm CROSS_COMPILE=arm-linux-gnueabi-
clean:
	make -C ${UBUNTU} M=${PWD} clean 
