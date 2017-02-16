#TODO faire une entête avec licence et tout

# make all = Make software and program
# make clean = Clean out built project files.
# make program = Download the hex file to the device, using avrdude.  Please
#                customize the avrdude settings below first!
# make docs = compile with doxygen the code documentation

#-------------------------------------------------------------------------------
# Tools
#-------------------------------------------------------------------------------

# Set DEBUG variable for once if not coming from command line
ifndef DEBUG
DEBUG = 0
endif

# Tool suffix when cross-compiling
CROSS_COMPILE ?=

# Compilation tools
CC = $(CROSS_COMPILE)gcc
AR = $(CROSS_COMPILE)ar
SIZE = $(CROSS_COMPILE)size
STRIP = $(CROSS_COMPILE)strip
ifeq ($(OS),Windows_NT)
$(error no gnu objcopy and objdump on your os)
endif
ifeq ($(shell uname -s),Linux)
OBJCOPY = $(CROSS_COMPILE)objcopy
OBJDUMP = $(CROSS_COMPILE)objdump
endif
ifeq ($(shell uname -s),Darwin)
OBJCOPY = $(CROSS_COMPILE)gobjcopy
OBJDUMP = $(CROSS_COMPILE)gobjdump
endif
SIZE = $(CROSS_COMPILE)size
GDB = $(CROSS_COMPILE)gdb
NM = $(CROSS_COMPILE)nm

ROOT_PATH = ../..
HAL_PATH = $(ROOT_PATH)/hal/$(BOARD)
DOC_PATH = $(ROOT_PATH)/../extra/Docs

# Doxygen configuration file name
DOXYFILE = $(ROOT_PATH)/../extra/.Doxyfile

OBJ_PATH = $(HAL_PATH)/obj
OUTPUT_NAME = lib$(BOARD)
OUTPUT_FILE_PATH = $(HAL_PATH)/$(OUTPUT_NAME).a

#|---------------------------------------------------------------------------------------|
#| Source files                                                                          |
#|---------------------------------------------------------------------------------------|
include ../sources.mk

#|---------------------------------------------------------------------------------------|
#| Extract file names and path                                                           |
#|---------------------------------------------------------------------------------------|
PROJ_ASRCS   = $(filter %.s,$(foreach file,$(SOURCES),$(file)))
PROJ_ASRCS  += $(filter %.S,$(foreach file,$(SOURCES),$(file)))
PROJ_CSRCS   = $(filter %.c,$(foreach file,$(SOURCES),$(file)))
PROJ_CPPSRCS = $(filter %.cpp,$(foreach file,$(SOURCES),$(file)))

#|---------------------------------------------------------------------------------------|
#| Set important path variables                                                          |
#|---------------------------------------------------------------------------------------|
VPATH    = $(foreach path,$(sort $(foreach file,$(SOURCES),$(dir $(file)))),$(path) :)
INC_PATH = $(INCLUDES)
LIB_PATH = -L$(dir $(RESOURCES_LINKER))

#|---------------------------------------------------------------------------------------|
#| Options for compiler binaries                                                         |
#|---------------------------------------------------------------------------------------|

COMMON_FLAGS = \
-g -O$(OPT)                                                      \
-funsigned-char -fpack-struct -fshort-enums                      \
-Wall -Wstrict-prototypes                                        \
-DMCU=$(MCU)                                                     \
-DMAINCLOCK=$(MAINCLOCK)                                         \
-DSCLFREQ=$(SCLFREQ)                                             \
$(INCLUDES)


ifeq ($(CROSS_COMPILE),avr-)
	COMMON_FLAGS += -mmcu=$(MCU)
	ASFLAGS = -mmcu=$(MCU)
endif

CFLAGS += $(COMMON_FLAGS) -std=gnu99

CPPFLAGS = $(COMMON_FLAGS) -std=gnu++11 -fno-rtti -fno-exceptions

ASFLAGS += -Wa,-adhlns=$(<:.S=.lst),-gstabs -I. -x assembler-with-cpp

LDFLAGS = -Wl,-Map=$(BOARD).map,--cref

#|---------------------------------------------------------------------------------------|
#| Define targets                                                                        |
#|---------------------------------------------------------------------------------------|
#AOBJS += $(patsubst %.S,%.o,$(PROJ_ASRCS))
AOBJS = $(patsubst %.s,%.o,$(addprefix $(OBJ_PATH)/, $(notdir $(PROJ_ASRCS))))
COBJS = $(patsubst %.c,%.o,$(addprefix $(OBJ_PATH)/, $(notdir $(PROJ_CSRCS))))
CPPOBJS = $(patsubst %.cpp,%.o,$(addprefix $(OBJ_PATH)/, $(notdir $(PROJ_CPPSRCS))))

.PHONY: all clean print_info packaging

all: $(OUTPUT_FILE_PATH)

print_info:
	@echo DEFAULT_GOAL ---------------------------------------------------------------------------------
	@echo $(.DEFAULT_GOAL)
	@echo VPATH ---------------------------------------------------------------------------------
	@echo $(VPATH)
	@echo SOURCES -------------------------------------------------------------------------------
	@echo $(SOURCES)
#	@echo PROJ_ASRCS ----------------------------------------------------------------------------
#	@echo $(PROJ_ASRCS)
#	@echo AOBJS ---------------------------------------------------------------------------------
#	@echo $(AOBJS)
	@echo PROJ_CSRCS ----------------------------------------------------------------------------
	@echo $(PROJ_CSRCS)
	@echo COBJS ---------------------------------------------------------------------------------
	@echo $(COBJS)
	@echo PROJ_CPPSRCS --------------------------------------------------------------------------
	@echo $(PROJ_CPPSRCS)
	@echo CPPOBJS -------------------------------------------------------------------------------
	@echo $(CPPOBJS)
	@echo ---------------------------------------------------------------------------------------
	@echo $(CURDIR)
	@echo $(OUTPUT_FILE_PATH)
	@echo ---------------------------------------------------------------------------------------

$(OUTPUT_FILE_PATH): $(OBJ_PATH) ../rules.mk ../sources.mk $(HAL_PATH)/Makefile $(AOBJS) $(COBJS) $(CPPOBJS)
ifneq ($(strip $(AOBJS)),)
	$(AR) -rv $(OUTPUT_FILE_PATH) $(AOBJS)
endif
ifneq ($(strip $(COBJS)),)
	$(AR) -rv $(OUTPUT_FILE_PATH) $(COBJS)
endif
ifneq ($(strip $(CPPOBJS)),)
	$(AR) -rv $(OUTPUT_FILE_PATH) $(CPPOBJS)
endif
	$(NM) $(OUTPUT_FILE_PATH) > $(HAL_PATH)/$(OUTPUT_NAME)_symbols.txt

#|---------------------------------------------------------------------------------------|
#| Compile or assemble                                                                   |
#|---------------------------------------------------------------------------------------|
$(AOBJS): $(OBJ_PATH)/%.o: %.s
	@echo +++ Assembling [$(notdir $<)]
	@$(AS) $(AFLAGS) $< -o $@

$(AOBJS): $(OBJ_PATH)/%.o: %.S
	@echo +++ Assembling [$(notdir $<)]
	@$(AS) $(AFLAGS) $< -o $@

$(COBJS): $(OBJ_PATH)/%.o: %.c
	@echo +++ Compiling [$(notdir $<)]
	@$(CC) $(CFLAGS) -c $< -o $@

$(CPPOBJS): $(OBJ_PATH)/%.o: %.cpp
	@echo +++ Compiling [$(notdir $<)]
	@$(CC) $(CPPFLAGS) -c $< -o $@

#|---------------------------------------------------------------------------------------|
#| Output folder                                                                         |
#|---------------------------------------------------------------------------------------|
$(OBJ_PATH):
	@echo +++ Creation of [$@]
	@-mkdir $(OBJ_PATH)

#|---------------------------------------------------------------------------------------|
#| Cleanup                                                                               |
#|---------------------------------------------------------------------------------------|
clean:
	-rm -f $(OBJ_PATH)/* $(OBJ_PATH)/*.*
	-rmdir $(OBJ_PATH)
	-rm -f $(OUTPUT_FILE_PATH)
	-rm -f $(HAL_PATH)/$(OUTPUT_NAME)_symbols.txt
	-rm -rf $(DOC_PATH)

#|---------------------------------------------------------------------------------------|
#| Dependencies                                                                          |
#|---------------------------------------------------------------------------------------|
$(OBJ_PATH)/%.d: %.s $(OBJ_PATH)
	@echo +++ Dependencies of [$(notdir $<)]
	@$(CC) $(AFLAGS) -MM -c $< -MT $(basename $@).o -o $@

$(OBJ_PATH)/%.d: %.S $(OBJ_PATH)
	@echo +++ Dependencies of [$(notdir $<)]
	@$(CC) $(AFLAGS) -MM -c $< -MT $(basename $@).o -o $@

$(OBJ_PATH)/%.d: %.c $(OBJ_PATH)
	@echo +++ Dependencies of [$(notdir $<)]
	@$(CC) $(CFLAGS) -MM -c $< -MT $(basename $@).o -o $@

$(OBJ_PATH)/%.d: %.cpp $(OBJ_PATH)
	@echo +++ Dependencies of [$(notdir $<)]
	@$(CC) $(CPPFLAGS) -MM -c $< -MT $(basename $@).o -o $@

#|---------------------------------------------------------------------------------------|
#| Include dependencies, if existing                                                     |
#| Little trick to avoid dependencies build for some rules when useless                  |
#| CAUTION: this won't work as expected with 'make clean all'                            |
#|---------------------------------------------------------------------------------------|
DEP_EXCLUDE_RULES := clean print_info
ifeq (,$(findstring $(MAKECMDGOALS), $(DEP_EXCLUDE_RULES)))
-include $(AOBJS:%.o=%.d)
-include $(COBJS:%.o=%.d)
-include $(CPPOBJS:%.o=%.d)
endif


#|---------------------------------------------------------------------------------------|
#| Module packaging for Arduino IDE Board Manager                                        |
#|---------------------------------------------------------------------------------------|
packaging: $(OUTPUT_FILE_PATH)

docs:
	( cat $(DOXYFILE) ; echo "OUTPUT_DIRECTORY = $(DOC_PATH)" ; echo "INPUT = $(DOC_SOURCES)" ) | doxygen -
#	doxygen $(DOXYFILE)

%.d:
