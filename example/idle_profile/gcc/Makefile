
#
# The base directory.
#
ROOT=../../..

#
# Include the common make definitions.
#
include ${ROOT}/tools/gcc_scripts/makedefs

#
# Where to find source files that do not live in this directory.
#
VPATH=..
VPATH+=../../drivers
VPATH+=../../common

VPATH+=${ROOT}/middleware/framework/pm
VPATH+=${ROOT}/middleware/framework/timer
VPATH+=${ROOT}/middleware/driver/hal 
VPATH+=${ROOT}/middleware/driver
VPATH+=${ROOT}/middleware/soc

#
# Additional Compiler Flags
#
CFLAGS+=-DSL_PLATFORM_MULTI_THREADED
CFLAGS+=-DUSE_FREERTOS

#
# Where to find header files that do not live in the source directory.
#
IPATH=..
IPATH+=../..
IPATH+=../../common
IPATH+=${ROOT}/inc
IPATH+=${ROOT}/oslib
IPATH+=${ROOT}/driverlib

IPATH+=${ROOT}/middleware/framework/pm
IPATH+=${ROOT}/middleware/framework/timer
IPATH+=${ROOT}/middleware/driver/hal 
IPATH+=${ROOT}/middleware/driver
IPATH+=${ROOT}/middleware/soc

IPATH+=${ROOT}/simplelink
IPATH+=${ROOT}/simplelink/source
IPATH+=${ROOT}/simplelink/include
IPATH+=${ROOT}/third_party/FreeRTOS
IPATH+=${ROOT}/third_party/FreeRTOS/source
IPATH+=${ROOT}/third_party/FreeRTOS/source/portable/GCC/ARM_CM4
IPATH+=${ROOT}/third_party/FreeRTOS/source/include

#
# The default rule, which causes the driver library to be built.
#
all: ${OBJDIR} ${BINDIR}
all: ${BINDIR}/idle.axf

#
# The rule to clean out all the build products.
#
clean:
	@rm -rf ${OBJDIR} ${wildcard *~}
	@rm -rf ${BINDIR} ${wildcard *~}


#
# The rule to create the target directories.
#
${OBJDIR}:
	@mkdir -p ${OBJDIR}

${BINDIR}:
	@mkdir -p ${BINDIR}

#
# Rules for building the idle example.
#
#${BINDIR}/idle.axf: ${OBJDIR}/i2c_if.o
${BINDIR}/idle.axf: ${OBJDIR}/main.o
${BINDIR}/idle.axf: ${OBJDIR}/pinmux.o
#${BINDIR}/idle.axf: ${OBJDIR}/gpio_if.o
#${BINDIR}/idle.axf: ${OBJDIR}/uart_if.o
#${BINDIR}/idle.axf: ${OBJDIR}/timer_if.o
${BINDIR}/idle.axf: ${OBJDIR}/utils_if.o
${BINDIR}/idle.axf: ${OBJDIR}/wdt_if.o
${BINDIR}/idle.axf: ${OBJDIR}/lp3p0_board.o
${BINDIR}/idle.axf: ${OBJDIR}/lp3p0_plat_ops.o

${BINDIR}/idle.axf: ${OBJDIR}/cc_pm.o
${BINDIR}/idle.axf: ${OBJDIR}/spi_hal.o
${BINDIR}/idle.axf: ${OBJDIR}/cc_timer.o
${BINDIR}/idle.axf: ${OBJDIR}/cc_io_park.o
${BINDIR}/idle.axf: ${OBJDIR}/cc_pm_ops.o
${BINDIR}/idle.axf: ${OBJDIR}/cc_pm.o
${BINDIR}/idle.axf: ${OBJDIR}/uart_drv.o
${BINDIR}/idle.axf: ${OBJDIR}/spi_drv.o
${BINDIR}/idle.axf: ${OBJDIR}/hw_timer32.o
#${BINDIR}/idle.axf: ${OBJDIR}/
${BINDIR}/idle.axf: ${OBJDIR}/gpio_hal.o
${BINDIR}/idle.axf: ${OBJDIR}/rtc_hal.o
${BINDIR}/idle.axf: ${OBJDIR}/interrupt_hal.o
${BINDIR}/idle.axf: ${OBJDIR}/uart_hal.o
${BINDIR}/idle.axf: ${OBJDIR}/dma_hal.o

${BINDIR}/idle.axf: ${OBJDIR}/startup_${COMPILER}.o
${BINDIR}/idle.axf: ${ROOT}/simplelink/${COMPILER}/${BINDIR}/libsimplelink.a
${BINDIR}/idle.axf: ${ROOT}/driverlib/${COMPILER}/${BINDIR}/libdriver.a
${BINDIR}/idle.axf: ${ROOT}/oslib/${COMPILER}/${BINDIR}/FreeRTOS.a
SCATTERgcc_idle=idle.ld
ENTRY_idle=ResetISR


#
# Include the automatically generated dependency files.
#
ifneq (${MAKECMDGOALS},clean)
-include ${wildcard ${COMPILER}/*.d} __dummy__
endif
