# -----------------------------------
# -------- OPENOCD SERVER -----------
# -----------------------------------
FROM ubuntu:16.04 as redox-fw-openocd

# Install dependencies
RUN apt-get update && apt-get install -y \
    openocd

# Expose port
EXPOSE 4444

CMD openocd -s /usr/local/Cellar/open-ocd/0.8.0/share/openocd/scripts/ \
    -f interface/stlink-v2.cfg \
    -f target/nrf51_stlink.tcl

# -----------------------------------
# -------- REDOX TOOLCHAIN ----------
# -----------------------------------
FROM ubuntu:16.04 as redox-fw-toolchain

# Install dependencies
RUN apt-get update && apt-get install -y \
    make \
    telnet \
    gcc-arm-none-eabi \
    wget \
    zip

# Download and install OpenOCD
RUN wget https://developer.nordicsemi.com/nRF5_SDK/nRF5_SDK_v11.x.x/nRF5_SDK_11.0.0_89a8197.zip && \
    unzip nRF5_SDK_11.0.0_89a8197.zip -d /usr/src/nRF5_SDK_11 && \
    rm nRF5_SDK_11.0.0_89a8197.zip

# Workdir setting
WORKDIR /usr/src/nRF5_SDK_11

# Toolchain setup
RUN sed -i "s/\/usr\/local\/gcc-arm-none-eabi-4_9-2015q1/\/usr\//" \
    components/toolchain/gcc/Makefile.posix
