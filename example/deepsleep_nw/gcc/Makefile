
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
all: ${BINDIR}/deepsleep.axf

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
# Rules for building the deepsleep example.
#
#${BINDIR}/deepsleep.axf: ${OBJDIR}/i2c_if.o
${BINDIR}/deepsleep.axf: ${OBJDIR}/main.o
${BINDIR}/deepsleep.axf: ${OBJDIR}/pinmux.o
${BINDIR}/deepsleep.axf: ${OBJDIR}/gpio_if.o
${BINDIR}/deepsleep.axf: ${OBJDIR}/uart_if.o
${BINDIR}/deepsleep.axf: ${OBJDIR}/timer_if.o
${BINDIR}/deepsleep.axf: ${OBJDIR}/nw_if.o

${BINDIR}/deepsleep.axf: ${OBJDIR}/startup_${COMPILER}.o
${BINDIR}/deepsleep.axf: ${ROOT}/simplelink/${COMPILER}/${BINDIR}/libsimplelink.a
${BINDIR}/deepsleep.axf: ${ROOT}/driverlib/${COMPILER}/${BINDIR}/libdriver.a
${BINDIR}/deepsleep.axf: ${ROOT}/oslib/${COMPILER}/${BINDIR}/FreeRTOS.a
SCATTERgcc_deepsleep=deepsleep.ld
ENTRY_deepsleep=ResetISR


#
# Include the automatically generated dependency files.
#
ifneq (${MAKECMDGOALS},clean)
-include ${wildcard ${COMPILER}/*.d} __dummy__
endif
