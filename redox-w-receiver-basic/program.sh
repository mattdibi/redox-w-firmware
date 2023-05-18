#!/bin/bash

SCRIPT_DIR=$(dirname "$(readlink -f "$0")")
cd $SCRIPT_DIR

echo '=============================== MAKING ================================'
cd custom/armgcc
make
if [[ $? -ne 0 ]] ; then
    exit 0
fi
sleep 0.1
HEX=`readlink -f _build/nrf51822_xxac-receiver.hex`
du -b $HEX

echo
echo '============================= PROGRAMMING ============================='
{
	echo "reset halt";
	sleep 0.1;
	echo "nrf51 mass_erase";
	sleep 1;
	echo "flash write_image erase" $HEX;
	sleep 11;
	echo "reset";
	sleep 0.1;
	exit;

} | telnet 127.0.0.1 4444

echo
echo '============================== FINISHED ==============================='
