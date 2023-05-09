TOP=..

include $(TOP)/configure/CONFIG
#----------------------------------------
#  ADD MACRO DEFINITIONS AFTER THIS LINE
#=============================

#USR_CFLAGS += -Werror
USR_CPPFLAGS += -DUSE_TYPED_RSET -DUSE_TYPED_DSET

#==================================================
# build a support library

LIBRARY_IOC += devTextFile

# xxxRecord.h will be created from xxxRecord.dbd
#DBDINC += xxxRecord

# install devTextFile.dbd into <top>/dbd
DBD += devTextFile.dbd

# specify all source files to be compiled and added to the library
devTextFile_SRCS += devTextFileLi.c
devTextFile_SRCS += devTextFileAi.c
devTextFile_SRCS += devTextFileSi.c
devTextFile_SRCS += devTextFileWf.c

devTextFile_LIBS += $(EPICS_BASE_IOC_LIBS)

#===========================

include $(TOP)/configure/RULES
#----------------------------------------
#  ADD RULES AFTER THIS LINE
