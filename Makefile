# Teensyduino Core Library
# http://www.pjrc.com/teensy/
# Copyright (c) 2019 PJRC.COM, LLC.
#
# Permission is hereby granted, free of charge, to any person obtaining
# a copy of this software and associated documentation files (the
# "Software"), to deal in the Software without restriction, including
# without limitation the rights to use, copy, modify, merge, publish,
# distribute, sublicense, and/or sell copies of the Software, and to
# permit persons to whom the Software is furnished to do so, subject to
# the following conditions:
#
# 1. The above copyright notice and this permission notice shall be
# included in all copies or substantial portions of the Software.
#
# 2. If the Software is incorporated into a build system that allows
# selection among a list of target devices, then similar target
# devices manufactured by PJRC.COM must be included in the list of
# target devices and selectable in the same manner.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
# MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
# NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
# BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
# ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
# CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

CORES_DIR = cores/teensy4
FREERTOS_DIR = freertos
FREERTOS_HEAP_DIR = $(FREERTOS_DIR)/portable/MemMang
CMSIS_6_DIR = CMSIS_6/CMSIS/Core
CMSIS_DAP_DIR = CMSIS-DAP/Firmware
TARGET_DIR = device

# Use these lines for Teensy 4.0
#MCU = IMXRT1062
#MCU_LD = imxrt1062.ld
#MCU_DEF = ARDUINO_TEENSY40

# Use these lines for Teensy 4.1
MCU = IMXRT1062
MCU_LD = $(TARGET_DIR)/imxrt1062_t41.ld
MCU_DEF = ARDUINO_TEENSY41

# The name of your project (used to name the compiled .hex file)
TARGET = logicanalyzer

# configurable options
OPTIONS = -DF_CPU=600000000 -DUSB_SERIAL -DLAYOUT_US_ENGLISH -DUSING_MAKEFILE

# usb serial
# OPTIONS += -DPRINT_DEBUG_STUFF -DUSB_SERIAL -DPRINT_DEBUG_USING_USB
OPTIONS += -DPRINT_DEBUG_STUFF -DUSB_CMSIS_DAP

#
# USB Type configuration:
#   -DUSB_SERIAL
#   -DUSB_DUAL_SERIAL
#   -DUSB_TRIPLE_SERIAL
#   -DUSB_KEYBOARDONLY
#   -DUSB_TOUCHSCREEN
#   -DUSB_HID_TOUCHSCREEN
#   -DUSB_HID
#   -DUSB_SERIAL_HID
#   -DUSB_MIDI
#   -DUSB_MIDI4
#   -DUSB_MIDI16
#   -DUSB_MIDI_SERIAL
#   -DUSB_MIDI4_SERIAL
#   -DUSB_MIDI16_SERIAL
#   -DUSB_AUDIO
#   -DUSB_MIDI_AUDIO_SERIAL
#   -DUSB_MIDI16_AUDIO_SERIAL
#   -DUSB_MTPDISK
#   -DUSB_RAWHID
#   -DUSB_FLIGHTSIM
#   -DUSB_FLIGHTSIM_JOYSTICK

# options needed by many Arduino libraries to configure for Teensy model
OPTIONS += -D__$(MCU)__ -DARDUINO=10813 -DTEENSYDUINO=159 -D$(MCU_DEF)

# for Cortex M7 with single & double precision FPU
CPUOPTIONS = -mcpu=cortex-m7 -mfloat-abi=hard -mfpu=fpv5-d16 -mthumb

# use this for a smaller, no-float printf
#SPECS = --specs=nano.specs

# Other Makefiles and project templates for Teensy
#
# https://forum.pjrc.com/threads/57251?p=213332&viewfull=1#post213332
# https://github.com/apmorton/teensy-template
# https://github.com/xxxajk/Arduino_Makefile_master
# https://github.com/JonHylands/uCee


#************************************************************************
# Location of Teensyduino utilities, Toolchain, and Arduino Libraries.
# To use this makefile without Arduino, copy the resources from these
# locations and edit the pathnames.  The rest of Arduino is not needed.
#************************************************************************

# Those that specify a NO_ARDUINO environment variable will
# be able to use this Makefile with no Arduino dependency.
# Please note that if ARDUINOPATH was set, it will override
# the NO_ARDUINO behaviour.
NO_ARDUINO = 1
ifndef NO_ARDUINO
# Path to your arduino installation
ARDUINOPATH ?= ../../../../..
endif

ifdef ARDUINOPATH

# path location for Teensy Loader, teensy_post_compile and teensy_reboot (on Linux)
TOOLSPATH = $(abspath $(ARDUINOPATH)/hardware/tools)

# path location for Arduino libraries (currently not used)
LIBRARYPATH = $(abspath $(ARDUINOPATH)/libraries)

# path location for the arm-none-eabi compiler
COMPILERPATH = $(abspath $(ARDUINOPATH)/hardware/tools/arm/bin)

else
# Default to the normal GNU/Linux compiler path if NO_ARDUINO
# and ARDUINOPATH was not set.
COMPILERPATH ?= /usr/bin

endif

#************************************************************************
# Settings below this point usually do not need to be edited
#************************************************************************

# CPPFLAGS = compiler options for C and C++
CPPFLAGS = -Wall -g -O2 $(CPUOPTIONS) -MMD $(OPTIONS) -ffunction-sections -fdata-sections

# compiler options for C++ only
CXXFLAGS = -std=gnu++17 -felide-constructors -fno-exceptions -fpermissive -fno-rtti -Wno-error=narrowing

# compiler options for C only
CFLAGS =

# linker options
BUILD_DIR = build
LDFLAGS = -Os -Wl,--gc-sections,--relax $(SPECS) $(CPUOPTIONS) -T$(MCU_LD) -L$(BUILD_DIR)

# additional libraries to link
LIBS = -lm -lstdc++ -lfreertos -lcores
# LIBS = -larm_cortexM7lfsp_math -lm -lstdc++ -lfreertos -lcores

# names for the compiler programs
FREERTOS_LIB = $(BUILD_DIR)/libfreertos.a
CORES_LIB = $(BUILD_DIR)/libcores.a
CMSIS_DAP_LIB = $(BUILD_DIR)/libCMSIS-DAP.a

CC = $(COMPILERPATH)/arm-none-eabi-gcc
CXX = $(COMPILERPATH)/arm-none-eabi-g++
AR = $(COMPILERPATH)/arm-none-eabi-ar
OBJCOPY = $(COMPILERPATH)/arm-none-eabi-objcopy
OBJDUMP = $(COMPILERPATH)/arm-none-eabi-objdump
SIZE = $(COMPILERPATH)/arm-none-eabi-size
READELF = $(COMPILERPATH)/arm-none-eabi-readelf

# automatically create lists of the sources and objects
C_SRC := $(wildcard $(TARGET_DIR)/src/*.c)
CPP_SRC := $(wildcard $(TARGET_DIR)/src/*.cpp)
FREERTOS_SRC := $(wildcard $(FREERTOS_DIR)/*.c)
FREERTOS_HEAP_SRC := $(FREERTOS_HEAP_DIR)/heap_1.c
CORES_C_SRC := $(wildcard $(CORES_DIR)/*.c)
CORES_CPP_SRC := $(wildcard $(CORES_DIR)/*.cpp)
CMSIS_DAP_C_SRC := $(wildcard $(CMSIS_DAP_DIR)/Source/*.c)

FREERTOS_INC := -I$(FREERTOS_DIR)/include -I$(TARGET_DIR)/inc -I$(CORES_DIR)
CORES_INC := -I$(CORES_DIR)
CMSIS_6_INC := -I$(CMSIS_6_DIR)/Include
CMSIS_DAP_INC := -I$(CMSIS_DAP_DIR)/Include -I$(TARGET_DIR)/inc -I$(CORES_DIR)
INC := $(FREERTOS_INC) $(CORES_INC) $(CMSIS_6_INC) $(CMSIS_DAP_INC)

OBJS := $(patsubst $(TARGET_DIR)/src/%.c,$(BUILD_DIR)/%.o,$(C_SRC))
OBJS += $(patsubst $(TARGET_DIR)/src/%.cpp,$(BUILD_DIR)/%.o,$(CPP_SRC))
FREERTOS_OBJS := $(patsubst $(FREERTOS_DIR)/%.c,$(BUILD_DIR)/freertos/%.o,$(FREERTOS_SRC))
FREERTOS_HEAP_OBJS := $(patsubst $(FREERTOS_HEAP_DIR)/%.c,$(BUILD_DIR)/freertos/portable/MemMang/%.o,$(FREERTOS_HEAP_SRC))
CORES_OBJS := $(patsubst $(CORES_DIR)/%.c,$(BUILD_DIR)/cores/%.o,$(CORES_C_SRC))
CORES_OBJS += $(patsubst $(CORES_DIR)/%.cpp,$(BUILD_DIR)/cores/%.o,$(CORES_CPP_SRC))
CMSIS_DAP_OBJS := $(patsubst $(CMSIS_DAP_DIR)/Source/%.c,$(BUILD_DIR)/CMSIS-DAP/%.o,$(CMSIS_DAP_C_SRC))

TARGET_ELF := $(BUILD_DIR)/$(TARGET).elf
TARGET_HEX := $(BUILD_DIR)/$(TARGET).hex

# the actual makefile rules (all .o files built by GNU make's default implicit rules)

all: $(TARGET_HEX)

# Free-RTOS
$(BUILD_DIR)/freertos/%.o: $(FREERTOS_DIR)/%.c
	@mkdir -p $(@D)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(FREERTOS_INC) -c -o $@ $<

$(BUILD_DIR)/freertos/portable/MemMang/%.o: $(FREERTOS_HEAP_DIR)/%.c
	@mkdir -p $(@D)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(FREERTOS_INC) -c -o $@ $<

$(FREERTOS_LIB): $(FREERTOS_OBJS) $(FREERTOS_HEAP_OBJS)
	@echo "Archiving $@"
	$(AR) rcs $@ $^

# Cores
$(BUILD_DIR)/cores/%.o: $(CORES_DIR)/%.c
	@mkdir -p $(@D)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(CORES_INC) -c -o $@ $<

$(BUILD_DIR)/cores/%.o: $(CORES_DIR)/%.cpp
	@mkdir -p $(@D)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(CORES_INC) -c -o $@ $<

$(CORES_LIB): $(CORES_OBJS)
	@echo "Archiving $@"
	$(AR) rcs $@ $^

# CMSIS-DAP
$(BUILD_DIR)/CMSIS-DAP/%.o: $(CMSIS_DAP_DIR)/Source/%.c
	@mkdir -p $(@D)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(CMSIS_6_INC) $(CMSIS_DAP_INC)  -c -o $@ $<

$(CMSIS_DAP_LIB): $(CMSIS_DAP_OBJS)
	@echo "Archiving $@"
	$(AR) rcs $@ $^

# Target
$(BUILD_DIR)/%.o: $(TARGET_DIR)/src/%.c
	@mkdir -p $(@D)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(INC) -c -o $@ $<

$(BUILD_DIR)/%.o: $(TARGET_DIR)/src/%.cpp
	@mkdir -p $(@D)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(INC) -c -o $@ $<

$(TARGET_ELF): $(OBJS) $(FREERTOS_LIB) $(CORES_LIB) $(CMSIS_DAP_LIB) $(MCU_LD)
	@echo "Linking $@"
	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)
	$(READELF) -a $@ > $(BUILD_DIR)/$(TARGET)_list.txt
	$(OBJDUMP) -Sdl $@ > build/logicanalyzer.asm

$(TARGET_HEX): $(TARGET_ELF)
	$(SIZE) $<
	$(OBJCOPY) -O ihex -R .eeprom $< $@
ifneq (,$(wildcard $(TOOLSPATH)))
	$(TOOLSPATH)/teensy_post_compile -file=$(basename $@) -path=$(shell pwd) -tools=$(TOOLSPATH)
	-$(TOOLSPATH)/teensy_reboot
endif

# compiler generated dependency info
-include $(OBJS:.o=.d) $(FREERTOS_OBJS:.o=.d)

clean:
	@echo "Cleaning project..."
	rm -rf $(BUILD_DIR)
