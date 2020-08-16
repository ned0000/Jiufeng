#
#  @file lnxobjdef.mak
#
#  @brief The object definition file for Linux system.
#
#  @author Min Zhang
#
#  @note
#

#---------------------------------------------------------------------------------------------------

OBJECTS = $(addsuffix .o,$(basename ${SOURCES}))

#---------------------------------------------------------------------------------------------------

