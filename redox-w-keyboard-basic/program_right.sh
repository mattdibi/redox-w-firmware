#!/bin/bash
SCRIPT_DIR="$(dirname "$(readlink -f "$0")")"

echo '=============================== MAKING ================================'
cd custom/armgcc
make keyboard_side=right
if [[ $? -ne 0 ]] ; then
    exit 0
fi
sleep 0.1
HEX=`readlink -f "$SCRIPT_DIR/_build/nrf51822_xxac-keyboard-right.hex"`
du -b $HEX

echo
echo '============================= PROGRAMMING ============================='
{
	echo "reset halt";
	sleep 0.1;
	echo "flash write_image erase" $HEX;
	sleep 11;
	echo "reset";
	sleep 0.1;
	exit;

} | telnet localhost 4444

echo
echo '============================== FINISHED ==============================='
