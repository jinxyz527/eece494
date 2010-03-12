# Platform name  cc2420DK, firefly, micaZ, firefly2, firefly2_2
PLATFORM = micaZ

# Target file name (without extension).
TARGET = main

# Set the Port that you programmer is connected to 
PROGRAMMING_PORT = /dev/ttyUSB0  # Default micaZ serial port 

# Set this such that the nano-RK directory is the base path
ROOT_DIR = ../../../nano-RK

# Set platform specific defines 
# The following will be defined based on the PLATFORM variable:
# PROG_TYPE  (e.g. avrdude, or uisp)
# MCU (e.g. atmega32, atmega128, atmega1281) 
# RADIO (e.g. cc2420)
include $(ROOT_DIR)/include/platform.mk


# Add extra source files. 
SRC = $(TARGET).c
SRC += $(ROOT_DIR)/src/net/bmac/bmac.c 

# Add extra includes files. 
EXTRAINCDIRS =
EXTRAINCDIRS += $(ROOT_DIR)/src/net/bmac/

#  This is where the final compile and download happens
include $(ROOT_DIR)/include/platform/$(PLATFORM)/common.mk
