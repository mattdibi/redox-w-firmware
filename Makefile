DIRS=redox-w-receiver-basic/custom/armgcc redox-w-keyboard-basic/custom/armgcc

all::
	make -C redox-w-receiver-basic/custom/armgcc; \
	make -C redox-w-keyboard-basic/custom/armgcc keyboard_side=left; \
	make -C redox-w-keyboard-basic/custom/armgcc keyboard_side=right; 

clean::
	make -C redox-w-receiver-basic/custom/armgcc clean; \
	make -C redox-w-keyboard-basic/custom/armgcc clean; \
	make -C redox-w-keyboard-basic/custom/armgcc clean; 