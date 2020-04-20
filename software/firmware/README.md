# Fluepdot firmware

Welcome to the fluepdot firmware.
You can use this code and the documented flipdot library to build your own, custom firmware for your fluepdot.

## Features
* CLI for configuration and testing purposes
* mDNS for simple service discovery
* HTTP API for framebuffer manipulations
* SNMP for monitoring and framebuffer manipulations
* BT LE support for framebuffer manipulations
* C flipdot library for custom applications

## Compilation
* Follow the [installation instructions](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/get-started/) for the ESP-IDF, but make sure to clone [my fork](https://github.com/fluepke/esp-idf) which contains some minor tweaks concerning LWIP SNMP support.
* Compile the external `mcufont` dependency: `cd components/mcufont/mcufont && make -C encoder && make -C fonts`.
  * You can simply include additional fonts by adding these to `components/mcufont/mcufont/fonts` and patching the Makefile.
* If you wish to use the pre-compiled compiler toolchain, which you installed by following the installation guide, you want to enable the Y2k38 bug in `sdkconfig.defaults`. Set `CONFIG_SDK_TOOLCHAIN_SUPPORTS_TIME_WIDE_64_BITS=n`.
* You can now compile, good luck: `cmake . && make -j8`
