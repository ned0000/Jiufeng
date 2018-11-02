#
#  @file
#
#  @brief The Makefile for object rules on Windows platform
#
#  @author Min Zhang
#
#  @note
#

#-----------------------------------------------------------------------------

{$S}.c{$O}.obj::
	$(CC) $(CFLAGS) $(EXTRA_CFLAGS) $(INCLUDES) $(EXTRA_INC_DIR) -Fd$O\ -c $<
	
{$S}.cpp{$O}.obj::
	$(CXX) $(CXXFLAGS) $(EXTRA_CXXFLAGS) $(INCLUDES) $(EXTRA_INC_DIR) -Fd$O\ -c $<

#-----------------------------------------------------------------------------

