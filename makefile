DIST ?= ./dist

all:
	make -C server
	make -C clients/C_Cpp/
	make -C clients/C_Cpp/mouse
clean:
	make -C server clean
	make -C clients/C_Cpp/ clean
	make -C clients/C_Cpp/mouse clean
	rm -rvf $(DIST)
install:
	mkdir -p $(DIST)
	install clients/C_Cpp/client $(DIST)
	install clients/C_Cpp/mouse/mouse_client $(DIST)
	install server/bin/server $(DIST)

all.cross.arm.gnueabihf:
	make all CC=arm-linux-gnueabihf-gcc LD=arm-linux-gnueabihf-gcc
	make install
