#TODO faire une entête avec licence et tout

#|---------------------------------------------------------------------------------------|
#| Extract file names and path                                                           |
#|---------------------------------------------------------------------------------------|
PROJ_ASRCS   = $(filter %.s,$(foreach file,$(SOURCES),$(file)))
PROJ_CSRCS   = $(filter %.c,$(foreach file,$(SOURCES),$(file)))
PROJ_CPPSRCS = $(filter %.cpp,$(foreach file,$(SOURCES),$(file)))

#|---------------------------------------------------------------------------------------|
#| Set important path variables                                                          |
#|---------------------------------------------------------------------------------------|
VPATH    = $(foreach path,$(sort $(foreach file,$(SOURCES),$(dir $(file)))),$(path) :)
INC_PATH = $(patsubst %,-I%,$(sort $(foreach file,$(filter %.h,$(SOURCES)),$(dir $(file)))))
INC_PATH += $(INCLUDES)
LIB_PATH =  -L$(dir $(HAL_LIB_PATH))

#|---------------------------------------------------------------------------------------|
#| Options for compiler binaries                                                         |
#|---------------------------------------------------------------------------------------|
COMMON_FLAGS = -Wall -Wchar-subscripts -Wcomment
COMMON_FLAGS += -Werror-implicit-function-declaration -Wmain -Wparentheses
COMMON_FLAGS += -Wsequence-point -Wreturn-type -Wswitch -Wtrigraphs -Wunused
COMMON_FLAGS += -Wuninitialized -Wunknown-pragmas -Wfloat-equal -Wundef
COMMON_FLAGS += -Wshadow -Wpointer-arith -Wwrite-strings
COMMON_FLAGS += -Wsign-compare -Waggregate-return
COMMON_FLAGS += -Wmissing-format-attribute -Wno-deprecated-declarations
COMMON_FLAGS += -Wredundant-decls -Wlong-long
COMMON_FLAGS += -Wunreachable-code -Wcast-align
COMMON_FLAGS += -ffunction-sections -fdata-sections
COMMON_FLAGS += -D$(MCU)
COMMON_FLAGS += -nostdlib $(INC_PATH)


ifeq ($(DEBUG),0)
COMMON_FLAGS += -Os
else
COMMON_FLAGS += -ggdb3 -O0
COMMON_FLAGS += -Wformat=2
endif

ifeq ($(CROSS_COMPILE),avr-)
	COMMON_FLAGS += -mmcu=$(MCU)
	ASFLAGS = -mmcu=$(MCU)
endif
# ifeq ($(CROSS_COMPILE),arm-none-eabi-)
# 	COMMON_FLAGS += -mcpu=$(MCU)
# 	LDFLAGS = -mcpu=$(MCU) -mthumb
# endif
CFLAGS += $(COMMON_FLAGS) -std=gnu99

CPPFLAGS = $(COMMON_FLAGS) -std=gnu++11 -fno-rtti -fno-exceptions

ASFLAGS += -Wa,-adhlns=$(<:.S=.lst),-gstabs -I. -x assembler-with-cpp

LDFLAGS = $(LIB_PATH)
LDFLAGS += -Wl,-v



#|---------------------------------------------------------------------------------------|
#| Define targets                                                                        |
#|---------------------------------------------------------------------------------------|
AOBJS = $(patsubst %.s,%.o,$(addprefix $(OBJ_PATH)/, $(notdir $(PROJ_ASRCS))))
COBJS = $(patsubst %.c,%.o,$(addprefix $(OBJ_PATH)/, $(notdir $(PROJ_CSRCS))))
CPPOBJS = $(patsubst %.cpp,%.o,$(addprefix $(OBJ_PATH)/, $(notdir $(PROJ_CPPSRCS))))

# Declare as phony all rules not based on files
# $(HAL_LIB_PATH) is here also to force checking its dependencies and eventually rebuild
.PHONY: all print_info clean packaging upload_openocd upload_edbg openocd debug $(HAL_LIB_PATH)

all: $(OUTPUT_FILE_PATH).bin

print_info:
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
	@echo CURDIR --------------------------------------------------------------------------------
	@echo $(CURDIR)
	@echo OBJ_PATH ---------------------------------------------------------------------------
	@echo $(OBJ_PATH)
	@echo ---------------------------------------------------------------------------------------

$(OUTPUT_FILE_PATH).bin: $(OUTPUT_FILE_PATH).elf
	$(OBJCOPY) -O binary $(OUTPUT_FILE_PATH).elf $(OUTPUT_FILE_PATH).bin

$(OUTPUT_FILE_PATH).elf: $(OBJ_PATH) $(HAL_LIB_PATH) $(PROJECT_PATH)/Makefile $(AOBJS) $(COBJS) $(CPPOBJS)
	$(CC) $(LDFLAGS) -o "$(OUTPUT_FILE_PATH).elf" $(AOBJS) $(COBJS) $(CPPOBJS) -lm -l$(HAL)
	$(NM) $(OUTPUT_FILE_PATH).elf >$(OUTPUT_FILE_PATH)_symbols.txt
	$(SIZE) --format=sysv -x $(OUTPUT_FILE_PATH).elf

$(HAL_LIB_PATH):
	@echo +++ Checking if library needs to be built [$(notdir $@)]
	make --no-builtin-rules -C $(dir $(HAL_LIB_PATH)) DEBUG=$(DEBUG)

#|---------------------------------------------------------------------------------------|
#| Compile or assemble                                                                   |
#|---------------------------------------------------------------------------------------|
$(AOBJS): $(OBJ_PATH)/%.o: %.s $(OBJ_PATH)
	@echo +++ Assembling [$(notdir $<)]
	$(AS) $(AFLAGS) $< -o $@

$(COBJS): $(OBJ_PATH)/%.o: %.c $(OBJ_PATH)
	@echo +++ Compiling [$(notdir $<)]
	$(CC) $(CFLAGS) -c $< -o $@

$(CPPOBJS): $(OBJ_PATH)/%.o: %.cpp $(OBJ_PATH)
	@echo +++ Compiling [$(notdir $<)]
	$(CC) $(CPPFLAGS) -c $< -o $@

#|---------------------------------------------------------------------------------------|
#| Output folder                                                                         |
#|---------------------------------------------------------------------------------------|
$(OBJ_PATH):
	@echo +++ Creation of [$@]
	@-mkdir $(OBJ_PATH)

#|---------------------------------------------------------------------------------------|
#| Dependencies                                                                          |
#|---------------------------------------------------------------------------------------|
$(OBJ_PATH)/%.d : %.s $(OBJ_PATH)
	@echo +++ Dependencies of [$(notdir $<)]
	@$(CC) $(AFLAGS) -MM -c $< -MT $(basename $@).o -o $@

$(OBJ_PATH)/%.d : %.S $(OBJ_PATH)
	@echo +++ Dependencies of [$(notdir $<)]
	@$(CC) $(AFLAGS) -MM -c $< -MT $(basename $@).o -o $@

$(OBJ_PATH)/%.d : %.c $(OBJ_PATH)
	@echo +++ Dependencies of [$(notdir $<)]
	@$(CC) $(CFLAGS) -MM -c $< -MT $(basename $@).o -o $@

$(OBJ_PATH)/%.d : %.cpp $(OBJ_PATH)
	@echo +++ Dependencies of [$(notdir $<)]
	@$(CC) $(CPPFLAGS) -MM -c $< -MT $(basename $@).o -o $@

#|---------------------------------------------------------------------------------------|
#| Cleanup                                                                               |
#|---------------------------------------------------------------------------------------|
clean:
	-rm -f $(OBJ_PATH)/* $(OBJ_PATH)/*.*
	-rmdir $(OBJ_PATH)
	-rm -f $(OUTPUT_FILE_PATH).elf
	-rm -f $(OUTPUT_FILE_PATH).bin
	-rm -f $(OUTPUT_FILE_PATH).map
	-rm -f $(OUTPUT_FILE_PATH)_symbols.txt

$(OBJ_PATH)/%.o : %.c
	$(CC) $(INCLUDES) $(CFLAGS) -c -o $@ $<


#|---------------------------------------------------------------------------------------|
#| Module packaging for Arduino IDE Board Manager                                        |
#| This rule is added in case of                                                         |
#|---------------------------------------------------------------------------------------|
packaging:

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

