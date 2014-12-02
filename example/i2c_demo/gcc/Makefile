#******************************************************************************
#
# Makefile - Rules for building the i2c_demo application.
#
#
#  Copyright (C) 2014 Texas Instruments Incorporated - http://www.ti.com/
#
#
#  Redistribution and use in source and binary forms, with or without
#  modification, are permitted provided that the following conditions
#  are met:
#
#    Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
#
#    Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the
#    distribution.
#
#    Neither the name of Texas Instruments Incorporated nor the names of
#    its contributors may be used to endorse or promote products derived
#    from this software without specific prior written permission.
#
#  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
#  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
#  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
#  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
#  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
#  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
#  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
#  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
#  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
#  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
#  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
#*****************************************************************************

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
#CFLAGS+=-DSL_PLATFORM_MULTI_THREADED
#CFLAGS+=-DUSE_FREERTOS
CFLAGS+=-DUSER_INPUT_ENABLE

#
# Where to find header files that do not live in the source directory.
#
#IPATH=..
#IPATH+=../..
IPATH+=${ROOT}/example/common
IPATH+=${ROOT}/inc
IPATH+=${ROOT}/oslib
IPATH+=${ROOT}/driverlib
IPATH+=${ROOT}/simplelink
IPATH+=${ROOT}/simplelink/source
IPATH+=${ROOT}/simplelink/include
#IPATH+=../../../third_party/FreeRTOS
#IPATH+=../../../third_party/FreeRTOS/source
#IPATH+=../../../third_party/FreeRTOS/source/portable/GCC/ARM_CM4
#IPATH+=../../../third_party/FreeRTOS/source/include

#
# The default rule, which causes the driver library to be built.
#
all: ${OBJDIR} ${BINDIR}
all: ${BINDIR}/i2c_demo.axf

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
# Rules for building the i2c_demo example.
#
${BINDIR}/i2c_demo.axf: ${OBJDIR}/main.o
${BINDIR}/i2c_demo.axf: ${OBJDIR}/pinmux.o
${BINDIR}/i2c_demo.axf: ${OBJDIR}/uart_if.o
${BINDIR}/i2c_demo.axf: ${OBJDIR}/i2c_if.o
${BINDIR}/i2c_demo.axf: ${OBJDIR}/startup_${COMPILER}.o
${BINDIR}/i2c_demo.axf: ${ROOT}/simplelink/${COMPILER}/${BINDIR}/libsimplelink_nonos.a
#${BINDIR}/i2c_demo.axf: ${ROOT}/simplelink/${COMPILER}/${BINDIR}/libsimplelink.a
${BINDIR}/i2c_demo.axf: ${ROOT}/driverlib/${COMPILER}/${BINDIR}/libdriver.a
#${BINDIR}/i2c_demo.axf: ${ROOT}/oslib/${COMPILER}/${BINDIR}/FreeRTOS.a
SCATTERgcc_i2c_demo=i2c_demo.ld
ENTRY_i2c_demo=ResetISR


#
# Include the automatically generated dependency files.
#
ifneq (${MAKECMDGOALS},clean)
-include ${wildcard ${COMPILER}/*.d} __dummy__
endif
