obj-m += pseudo8.o

KSRC = /home/arrijith/ebuild/package/KSRC

all:
	make -C ${KSRC} M=${PWD} modules ARCH=arm CROSS_COMPILE=arm-linux-gnueabi-
clean:
	make -C ${KSRC} M=${PWD} clean 
