#TODO faire une entête avec licence et tout

SHELL := /bin/sh

.SUFFIXES: .d .o .c .h .cpp .hpp .s .S

PROJECT_NAME := tests
HAL = stub

include ../preambule.mk

INCLUDES += -I$(PROJECT_PATH)/inc
INCLUDES += -I$(PROJECT_PATH)/src

SOURCES += $(PROJECT_PATH)/src/test_mngmnt.c

include ../rules.mk
