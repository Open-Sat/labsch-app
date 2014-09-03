###############################################################################
## $Id:$
##
## Purpose:
##   Build Lab Scheduler App unit test
##
## Notes:
##   1. See ut-lib's config.mak and rule.mak for what variables must be defined.
##   2. No need to use LOCAL_INC since the tool headers are already
##      included by config.mak
##   3. "-include" used for dependencies because if file doesn't exist it
##      will not report an error.
##
## $Log: $
##
###############################################################################

###############################################################################
# Directory base
#

UT_APPS_BASE = $(shell cd ../..; pwd)
UT_TOOL_BASE = $(shell cd ../../../tools; pwd)
REF_MISSION_BASE = /cygdrive/c/dmccomas/projects/gnc-fsw-proto/user/proto


###############################################################################
# Source, Object, and Executable Definitions
# - OBJS: Object files required to build unit
# - SRC:  Source files required to build unit; used to generate dependencies.
# - 

SUBSYS = labsch_app
BUILD_TYPE = EXE
UNIT = ut_labsch_app

FSW_OBJS := schtbl.o
FSW_OBJS += labsch.o
FSW_OBJS += msgtbl.o
FSW_OBJS += tblmgr.o
FSW_OBJS += cmdmgr.o

UT_OBJS := msgtbl_testcase.o
UT_OBJS += schtbl_testcase.o
UT_OBJS += cmdmgr_testcase.o
UT_OBJS += tblmgr_testcase.o
UT_OBJS += labsch_testrunner.o

SRC := $(OBJS:.o=.c)

UT_TEST_EXE = labsch_testrunner.exe
OBJS :=  $(UT_OBJS) $(FSW_OBJS)
 
###############################################################################
# Local Directories and Options
# - Source and include file
# - Library
#

LOCAL_SRC := ../fsw
LOCAL_SRC += ../ut

LOCAL_INC := ../fsw
LOCAL_INC += ../deploy
LOCAL_INC += $(REF_MISSION_BASE)/build/pc-linux/inc

LOCAL_LIB_DIR  := 
 
LOCAL_LIB_FILE :=

#LOCAL_COPTS := -D

###############################################################################
# Standard configuration and rules
#

#all:$(UT_TEST_EXE)
#$(UT_TEST_EXE): $(UT_OBJS) $(FSW_OBJS)

CONFIG_MAK = $(UT_TOOL_BASE)/ut-lib/build/config.mak

include $(CONFIG_MAK)

include $(RULE_MAK)

###############################################################################
# Include the dependancy list
#
-include $(SUBSYS).d

# eof

