#TODO faire une entête avec licence et tout

SHELL = /bin/sh

.SUFFIXES: .tar.bz2

ROOT_PATH := .
EXAMPLES_PATH := $(ROOT_PATH)/extra/examples
HAL_PATH := $(ROOT_PATH)/robus/hal

ifeq ($(OS),Windows_NT)
$(error no gnu ls on your os)
endif

ifeq ($(shell uname -s),Linux)
# HAL list, can be overriden via command line or ENV
HAL_LIST?=$(shell ls --hide=*.mk $(HAL_PATH))
# Examples list, can be overriden via command line or ENV
EXAMPLES?=$(shell ls --hide=*.mk --hide=template --hide=.* --hide=sketches.* $(EXAMPLES_PATH))
endif

ifeq ($(shell uname -s),Darwin)
# HAL list, can be overriden via command line or ENV
HAL_LIST?=$(shell gls --hide=*.mk $(HAL_PATH))
# Examples list, can be overriden via command line or ENV
EXAMPLES?=$(shell gls --hide=*.mk --hide=template --hide=.* --hide=sketches.* $(EXAMPLES_PATH))
endif

# HAL name, can be overriden via command line or ENV
HAL ?= stub

CORE_VERSION := $(shell grep version= $(ROOT_PATH)/robus/platform.txt | sed 's/version=//g')
PACKAGE_NAME := $(basename $(notdir $(CURDIR)))
PACKAGE_FOLDER := robus

ifeq ($(TRAVIS),true)
PRINT_INFO_TRAVIS=print_info_travis
else
PRINT_INFO_TRAVIS=
endif

# -----------------------------------------------------------------------------
# packaging specific

# these words match the folder names in module/tools
PACKAGE_OS_VIRTUAL := all
PACKAGE_OS_REAL    := win32 win64 linux32 linux64 osx
PACKAGE_OS_VALID   := $(PACKAGE_OS_VIRTUAL) $(PACKAGE_OS_REAL)

#
# We will define a convenient macro function to handle correctly all cases
# PACKAGE_PROCESS must be called with one parameter
# first parameter is one value from PACKAGE_OS_VALID
#

# handle non-packaging request (normal build)
ifeq ($(PACKAGE_OS),)
# we are not requested to build the release package
# specify default variant, if not provided
HAL ?= stub
$(warning, "Packaging won't be fully processed.")
else
#
# handle case where PACKAGE_OS is wrong
#
  ifeq (,$(findstring $(PACKAGE_OS), $(PACKAGE_OS_VALID)))
$(error, "Wrong OS specified for Packaging ($(PACKAGE_OS)).")
  else
#
# handle full packaging for given variant(s)
# if no specific variant is requested, all present will be integrated
#
    PACKAGE_TO_BE_PROCESSED := $(PACKAGE_NAME)-$(CORE_VERSION)-$(PACKAGE_OS).tar.bz2
  endif
endif

ifeq (postpackaging,$(findstring $(MAKECMDGOALS),postpackaging))
	PACKAGE_OS_FILENAME=$(PACKAGE_NAME)-$(CORE_VERSION)-$(PACKAGE_OS).tar.bz2
  PACKAGE_CHKSUM := $(firstword $(shell sha256sum $(PACKAGE_OS_FILENAME)))
	PACKAGE_SIZE := $(firstword $(shell wc -c $(PACKAGE_OS_FILENAME)))
endif

# end of packaging specific
# -----------------------------------------------------------------------------

.PHONY: all clean print_info clean print_info_travis packaging packaging_clean postpackaging help $(PACKAGE_NAME)-$(CORE_VERSION)-all.tar.bz2

all: help print_info $(PRINT_INFO_TRAVIS)
	@echo ----------------------------------------------------------
	@echo  Build examples: $(EXAMPLES)
	@echo    for hal: $(HAL)
	$(foreach example,$(EXAMPLES),$(MAKE) --no-builtin-rules HAL=$(HAL) all -C $(EXAMPLES_PATH)/$(example) ; )
	@echo ----------------------------------------------------------

clean:
	@echo ----------------------------------------------------------
	@echo  Clean examples: $(EXAMPLES)
	@echo    for hal: $(HAL)
	$(foreach example,$(EXAMPLES),$(MAKE) --no-builtin-rules HAL=$(HAL) clean -C $(EXAMPLES_PATH)/$(example) ; )
	@echo ----------------------------------------------------------

help:
	@echo "----------------------------------------------------------"
	@echo "Available targets are:"
	@echo "- hals = one of [$(HAL_LIST)] all"
	@echo "- hals = one of [$(HAL_LIST)] clean"
	@echo "- PACKAGE_OS = one of [$(PACKAGE_OS_VALID)] packaging"
	@echo " "
	@echo "example:"
	@echo "> make HAL=$(firstword $(HAL_LIST)) all"
	@echo "or"
	@echo "> make PACKAGE_OS=$(firstword $(PACKAGE_OS_VALID)) packaging"
	@echo " "
	@echo "----------------------------------------------------------"

print_info:
	@echo ----------------------------------------------------------
	@echo Building Poppy-com using
	@echo CURDIR                  = $(CURDIR)
	@echo OS                      = $(OS)
	@echo SHELL                   = $(SHELL)
	@echo TERM                    = $(TERM)
	@echo HAL_PATH 		          = $(HAL_PATH)
	@echo HAL_LIST                = $(HAL_LIST)
	@echo HAL  			      	  = $(HAL)
	@echo EXAMPLES_PATH           = $(EXAMPLES_PATH)
	@echo EXAMPLES_LIST           = $(EXAMPLES)
	@echo CORE_VERSION            = $(CORE_VERSION)
	@echo PACKAGE_NAME            = $(PACKAGE_NAME)
	@echo PACKAGE_OS              = $(PACKAGE_OS)
	@echo PACKAGE_OS_VALID        = $(PACKAGE_OS_VALID)
	@echo PACKAGE_OS_REAL         = $(PACKAGE_OS_REAL)
#	@echo PACKAGE_PROCESS         = $(PACKAGE_PROCESS)
	@echo PACKAGE_TO_BE_PROCESSED = $(PACKAGE_TO_BE_PROCESSED)
#	"$(CC)" -v
#	env

print_info_travis:
	@echo ----------------------------------------------------------
	@echo Travis-CI envvars
	@echo TRAVIS_OS_NAME      = $(TRAVIS_OS_NAME)
	@echo TRAVIS_LANGUAGE     = $(TRAVIS_LANGUAGE)
	@echo TRAVIS_REPO_SLUG    = $(TRAVIS_REPO_SLUG)
	@echo TRAVIS_BRANCH       = $(TRAVIS_BRANCH)
	@echo TRAVIS_TAG          = $(TRAVIS_TAG)
	@echo TRAVIS_PULL_REQUEST = $(TRAVIS_PULL_REQUEST)
	@echo TRAVIS_COMMIT       = $(TRAVIS_COMMIT)
	@echo TRAVIS_COMMIT_RANGE = $(TRAVIS_COMMIT_RANGE)
	@echo TRAVIS_JOB_ID       = $(TRAVIS_JOB_ID)
	@echo TRAVIS_JOB_NUMBER   = $(TRAVIS_JOB_NUMBER)
	@echo TRAVIS_BUILD_DIR    = $(TRAVIS_BUILD_DIR)
	@echo TRAVIS_BUILD_ID     = $(TRAVIS_BUILD_ID)
	@echo TRAVIS_BUILD_NUMBER = $(TRAVIS_BUILD_NUMBER)

packaging: packaging_clean build_hals $(PACKAGE_TO_BE_PROCESSED)

build_hals:
	@echo ----------------------------------------------------------
	@echo  Build hal list: $(HAL_LIST)
	$(foreach hal,$(HAL_LIST),$(MAKE) --no-builtin-rules HAL=$(HAL) clean -C $(HAL_PATH)/$(hal) ; )
	$(foreach hal,$(HAL_LIST),$(MAKE) --no-builtin-rules HAL=$(HAL) all -C $(HAL_PATH)/$(hal) ; )
	@echo ----------------------------------------------------------

# fake phony rule for 'all' virtual os target
$(PACKAGE_NAME)-$(CORE_VERSION)-all.tar.bz2: $(patsubst %,$(PACKAGE_NAME)-$(CORE_VERSION)-%.tar.bz2,$(PACKAGE_OS_REAL))

define PACKAGE_OS_PROCESS=
# Arduino module packaging:
#   - exclude version control system files, here git files and folders .git, .gitattributes and .gitignore
#   - exclude 'extras' folder
#   - exclude 'obj' folder from variants
$(PACKAGE_NAME)-$(CORE_VERSION)-$(1).tar.bz2:
	@echo ----------------------------------------------------------
	@echo "Packaging for OS $(1)."
	@echo "Removing OSs $(filter-out $(1),$(PACKAGE_OS_REAL))"

	tar --transform "s|module|$(PACKAGE_NAME)-$(CORE_VERSION)-$(1)|g" --transform "s|tools/$(1)|tools|g" --exclude=.gitattributes --exclude=.travis.yml --exclude-vcs \
--exclude-vcs-ignores --exclude=.clang --exclude=.codelite --exclude=obj --exclude=*.bat --exclude=*.map --exclude=*.elf --exclude=*.bin --exclude=*_symbols.txt --exclude=CMSIS_SVD $(patsubst %,--exclude=%,$(filter-out $(1),$(PACKAGE_OS_REAL)))\
-cjf $(PACKAGE_NAME)-$(CORE_VERSION)-$(1).tar.bz2 "$(PACKAGE_FOLDER)"
	$(MAKE) --no-builtin-rules PACKAGE_OS=$(1) postpackaging -C .
	@echo ----------------------------------------------------------
endef

$(foreach OS_VALUE,$(PACKAGE_OS_REAL), $(eval $(call PACKAGE_OS_PROCESS,$(OS_VALUE))))

postpackaging:
	@echo "PACKAGE_CHKSUM      = $(PACKAGE_CHKSUM)"
	@echo "PACKAGE_SIZE        = $(PACKAGE_SIZE)"
	@echo "TRAVIS_JOB_NUMBER   = $(TRAVIS_JOB_NUMBER)"
	@echo "TRAVIS_BUILD_NUMBER = $(TRAVIS_BUILD_NUMBER)"
	@echo "PACKAGE_OS_FILENAME = $(PACKAGE_OS_FILENAME)"
	cat extras/package_index.json.template | sed s/%%PR_NUMBER%%/$(TRAVIS_JOB_NUMBER)/ | sed s/%%BUILD_NUMBER%%/$(TRAVIS_BUILD_NUMBER)/ | sed s/%%VERSION%%/$(CORE_VERSION)-build-$(TRAVIS_BUILD_NUMBER)/ | \
sed s/%%FILENAME%%/$(PACKAGE_OS_FILENAME)/ | sed s/%%CHECKSUM%%/$(PACKAGE_CHKSUM)/ | sed s/%%SIZE%%/$(PACKAGE_SIZE)/ > package_$(PACKAGE_NAME)_$(CORE_VERSION)_$(PACKAGE_OS)_b$(TRAVIS_BUILD_NUMBER).json
	@echo "package_$(PACKAGE_NAME)_$(CORE_VERSION)_$(PACKAGE_OS)_b$(TRAVIS_BUILD_NUMBER).json created"

packaging_clean:
	-$(RM) $(PACKAGE_NAME)-*.tar.bz2 package_$(PACKAGE_NAME)_*.json
