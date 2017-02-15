LIB_SRC_PATH = $(ROOT_PATH)/src
LIB_INC_PATH = $(ROOT_PATH)/inc

SRC_DISCARDED=


SRC_LIB += \
$(LIB_SRC_PATH)/sys_msg.c   			\
$(LIB_SRC_PATH)/reception.c    			\
$(LIB_SRC_PATH)/robus.c   				\
$(LIB_SRC_PATH)/target.c
INCLUDES += -I$(LIB_INC_PATH) -I$(LIB_SRC_PATH) -I$(ROOT_PATH)

SRC_HAL += \
$(HAL_PATH)/hal.c
INCLUDES += -I$(HAL_PATH)

SOURCES=$(SRC_LIB) $(SRC_HAL)

DOC_SOURCES=$(ROOT_PATH)/../ $(ROOT_PATH)/../extra/doc $(SOURCES)
