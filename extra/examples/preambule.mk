#TODO faire une entête avec licence et tout

#-------------------------------------------------------------------------------
# Tools and definitions
#-------------------------------------------------------------------------------

# Set DEBUG variable for once if not coming from command line
ifndef DEBUG
DEBUG := 0
endif

# Tool suffix when cross-compiling
CROSS_COMPILE := avr-

# Compilation tools
CC = $(CROSS_COMPILE)gcc
AR = $(CROSS_COMPILE)ar
SIZE = $(CROSS_COMPILE)size
STRIP = $(CROSS_COMPILE)strip
OBJCOPY = $(CROSS_COMPILE)objcopy
OBJDUMP = $(CROSS_COMPILE)objdump
SIZE = $(CROSS_COMPILE)size
GDB = $(CROSS_COMPILE)gdb
NM = $(CROSS_COMPILE)nm

ROOT_PATH = ../../../robus

HAL ?= stub
HAL_PATH = $(ROOT_PATH)/hal/$(HAL)
include $(HAL_PATH)/mcu.mk

PROJECT_PATH := ../../../extra/examples/$(PROJECT_NAME)

# RESOURCES_JLINK_UPLOAD := $(PROJECT_PATH)/$(HAL).jlink
# RESOURCES_OPENOCD_UPLOAD:= $(VARIANT_PATH)/openocd_scripts/variant_upload.cfg
# RESOURCES_OPENOCD_START := $(VARIANT_PATH)/openocd_scripts/variant_debug_start.cfg
# RESOURCES_GDB := $(VARIANT_PATH)/debug_scripts/variant.gdb
# RESOURCES_LINKER := $(VARIANT_PATH)/linker_scripts/gcc/variant_without_bootloader.ld

#|---------------------------------------------------------------------------------------|
#| Upload tools                                                                          |
#|---------------------------------------------------------------------------------------|
# change this value if openocd isn't in the user/system PATH
# TOOL_OPENOCD := openocd
# TOOL_JLINK := JLink
# TOOL_EDBG := $(ROOT_PATH)/tools/edbg

#|---------------------------------------------------------------------------------------|
#| Include paths                                                                         |
#|---------------------------------------------------------------------------------------|
# INCLUDES  = -I$(ROOT_PATH)/tools/CMSIS_API/Include
# INCLUDES += -I$(ROOT_PATH)/tools/CMSIS_Devices/ATMEL
INCLUDES += -I$(HAL_PATH)
INCLUDES += -I$(ROOT_PATH)
INCLUDES += -I$(PROJECT_PATH)

#|---------------------------------------------------------------------------------------|
#| Output paths                                                                          |
#|---------------------------------------------------------------------------------------|
OBJ_PATH := $(PROJECT_PATH)/obj_$(HAL)
OUTPUT_FILE_PATH := $(PROJECT_PATH)/$(PROJECT_NAME)_$(HAL)
HAL_LIB_PATH := $(HAL_PATH)/lib$(HAL).a

#|---------------------------------------------------------------------------------------|
#| Source files                                                                          |
#|---------------------------------------------------------------------------------------|
SOURCES =\
$(PROJECT_PATH)/$(PROJECT_NAME).c
