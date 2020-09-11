TOP=../..

include $(TOP)/configure/CONFIG
#----------------------------------------
#  ADD MACRO DEFINITIONS AFTER THIS LINE
#=============================

#==================================================
# build a support library

LIBRARY_IOC += devTextFile

# xxxRecord.h will be created from xxxRecord.dbd
#DBDINC += xxxRecord

# install devTextFile.dbd into <top>/dbd
DBD += devWaveformTextFile.dbd

# specify all source files to be compiled and added to the library
devTextFile_SRCS += devWaveformTextFile.c

devTextFile_LIBS += $(EPICS_BASE_IOC_LIBS)

#USR_CFLAGS += -Werror
#===========================

include $(TOP)/configure/RULES
#----------------------------------------
#  ADD RULES AFTER THIS LINE

