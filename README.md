# Redox Wireless Keyboard Firmware
Firmware for Nordic MCUs used in the Redox wireless Keyboard, contains precompiled .hex files, as well as sources buildable with the Nordic SDK
This firmware was derived from [Reversebias' Mitosis](https://github.com/reversebias/mitosis) and [Durburz's Interphase](https://github.com/Durburz/interphase-firmware/) firmware.

## Redox's documentation page
For additional information about the Redox keyboard visit:
- [Redox's Github page](https://github.com/mattdibi/redox-keyboard)
- [Redox's Hackaday page](https://hackaday.io/project/160610-redox-keyboard)

## Install Docker-based development environment (WIP)

**Requirements:**
- Linux-based distro (macOS should work but wasn't tested)
- [Docker](https://docs.docker.com/get-docker/)
- [Docker-compose](https://docs.docker.com/compose/install/)

#### Build the two container images

From inside the `redox-w-firmware` folder run:

```
$ docker-compose build
```

After the process completes you should see three new images:

```
$ docker images
REPOSITORY           TAG       IMAGE ID       CREATED          SIZE
redox-fw-toolchain   latest    810de995238e   16 minutes ago   896MB
redox-fw-openocd     latest    173d86d1409c   18 minutes ago   168MB
ubuntu               16.04     8185511cd5ad   4 weeks ago      132MB
```

#### Run the two images using docker compose

After connecting the STLinkV2 debugger, from inside the `redox-w-firmware` folder run:

```
$ docker-compose up -d
```

You should see the two containers running:

```
$ docker-compose up -d
Creating redox-w-firmware_openocd_1 ... done
Creating redox-w-firmware_toolchain_1 ... done
$ docker ps
CONTAINER ID   IMAGE                       COMMAND                  CREATED         STATUS         PORTS     NAMES
84f606ad7e25   redox-fw-toolchain:latest   "tail -f /dev/null"      4 seconds ago   Up 3 seconds             redox-w-firmware_toolchain_1
8d0d6b5da95a   redox-fw-openocd:latest     "/bin/sh -c 'openocd…"   4 seconds ago   Up 3 seconds             redox-w-firmware_openocd_1
```

You can now start making changes in the code.

#### Build and upload the firmware

After you're satisfied with your changes you can build and upload the firmware by issuing the following:

```
$ docker exec -it redox-w-firmware_toolchain_1 ./redox-w-firmware/redox-w-receiver-basic/program.sh
$ docker exec -it redox-w-firmware_toolchain_1 ./redox-w-firmware/redox-w-keyboard-basic/program_right.sh
$ docker exec -it redox-w-firmware_toolchain_1 ./redox-w-firmware/redox-w-keyboard-basic/program_left.sh
```

#### Stop the containers

To stop the two running containers, from inside the `redox-w-firmware` folder run:

```
$ docker-compose down
```

## Install native development environment

Tested on Ubuntu 16.04.2, but should be able to find alternatives on all distros.

```
$ sudo apt install openocd gcc-arm-none-eabi
```

## Download Nordic SDK

Nordic does not allow redistribution of their SDK or components, so download and extract from their site:

https://developer.nordicsemi.com/nRF5_SDK/nRF5_SDK_v11.x.x/nRF5_SDK_11.0.0_89a8197.zip

Firmware written and tested with version 11

```
$ unzip nRF5_SDK_11.0.0_89a8197.zip -d nRF5_SDK_11
$ cd nRF5_SDK_11
```

## Toolchain set-up

A cofiguration file that came with the SDK needs to be changed. Assuming you installed gcc-arm with apt, the compiler root path needs to be changed in /components/toolchain/gcc/Makefile.posix, the line:

```
$ GNU_INSTALL_ROOT := /usr/local/gcc-arm-none-eabi-4_9-2015q1
```

Replaced with:

```
$ GNU_INSTALL_ROOT := /usr/
```

## Clone repository
Inside nRF5_SDK_11/

```
$ git clone https://github.com/mattdibi/redox-w-firmware
```

## Install udev rules

```
$ sudo cp redox-w-firmware/49-stlinkv2.rules /etc/udev/rules.d/
```
Plug in, or replug in the programmer after this.

## OpenOCD server
The programming header on the side of the keyboard:

<p align="center">
<img src="img/IMAG0596.jpg" alt="Redox-programming-headers" width="600"/>
</p>

It's best to remove the battery during long sessions of debugging, as charging non-rechargeable lithium batteries isn't recommended.

Launch a debugging session with:

```
$ openocd -s /usr/local/Cellar/open-ocd/0.8.0/share/openocd/scripts/ -f interface/stlink-v2.cfg -f target/nrf51_stlink.tcl
```
Should give you an output ending in:

```
Info : nrf51.cpu: hardware has 4 breakpoints, 2 watchpoints
```
Otherwise you likely have a loose or wrong wire.


## Manual programming
From the factory, these chips need to be erased:

```
$ echo reset halt | telnet localhost 4444
$ echo nrf51 mass_erase | telnet localhost 4444
```
From there, the precompiled binaries can be loaded:

```
$ echo reset halt | telnet localhost 4444
$ echo flash write_image `readlink -f precompiled-basic-left.hex` | telnet localhost 4444
$ echo reset | telnet localhost 4444
```

## Automatic make and programming scripts
To use the automatic build scripts:
* keyboard-left: `./redox-w-keyboard-basic/program_left.sh`
* keyboard-right: `./redox-w-keyboard-basic/program_right.sh`
* receiver: `./redox-w-receiver-basic/program.sh`

An openocd session should be running in another terminal, as this script sends commands to it.
