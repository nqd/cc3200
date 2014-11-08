CC3200 Development under Linux
================================================================================

This document contains a HOWTO on developing applications for the TI CC3200
Launchpad under Ubuntu 64bit. It is a work-in-progress, but all information contained
should work and has been tested on my Ubuntu system. If you feel that
something is missing, can be improved or is wrong, please let me know.

Setup
---------------------
Install GNU Tools for ARM Embeded Processor: `gcc-arm-none-eabi`, `gdb-arm-none-eabi`

Install OpenOCD for onchip debugging: `openocd` (v0.7)

The launchpad is based on an FTDI chip, and current Linux kernels provide very good
support for it. If the kernel module does not kick in automatically, because TI did
change VID and PID code. Run the command below (for kernel >= 3.12)

      modprobe ftdi_sio
      echo 0451 c32a > /sys/bus/usb-serial/drivers/ftdi_sio/new_id

The board should have enumberated as `/dev/ttyUSB{0,1}`. We will use `ttyUSB1` for UART
backchannel


Build your first example
---------------------
Take `example/blinky' as example.
Go to gcc, type `make -f Makefile`, you will see the blinky.axf generated under `exe` folder.
Copy this file to `/tools/gcc_scripts`.

First, make sure you got openocd setup by running `openocd -f cc3200.cfg`, you should get
output like
      Open On-Chip Debugger 0.7.0 (2013-10-22-08:31)
      Licensed under GNU GPL v2
      For bug reports, read
        http://openocd.sourceforge.net/doc/doxygen/bugs.html
      Info : only one transport option; autoselect 'jtag'
      adapter speed: 1000 kHz
      cc3200_dbginit
      Info : clock speed 1000 kHz
      Info : JTAG tap: cc3200.jrc tap/device found: 0x0b97c02f (mfg: 0x017, part: 0xb97c, ver: 0x0)
      Info : JTAG tap: cc3200.dap enabled
      Info : cc3200.cpu: hardware has 6 breakpoints, 4 watchpoints

Then, begin to debug with `arm-none-eabi-gdb -x gdbinit blinky.axf`. Typing continue when got 
input prompt, you will see three LEDs of the launchpad blinking.


