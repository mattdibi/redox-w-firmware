
all:
	make -C redox-w-receiver-basic/custom/armgcc
	make -C redox-w-keyboard-basic/custom/armgcc

clean:
	make -C redox-w-receiver-basic/custom/armgcc clean
	make -C redox-w-keyboard-basic/custom/armgcc clean
