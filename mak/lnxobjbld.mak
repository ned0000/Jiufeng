#
#  @file
#
#  @brief The Makefile for object rules on Linux platform
#
#  @author Min Zhang
#
#  @note
#

#---------------------------------------------------------------------------------------------------

%.o: %.c
	$(CC) -c -o $@ $(CFLAGS) $(EXTRA_CFLAGS) $(INCLUDES) $(EXTRA_INC_DIR) $<

%.o : %.cpp
	$(CXX) -c -o $@ ${CXXFLAGS} $(EXTRA_CXXFLAGS) ${INCLUDES} $(EXTRA_INC_DIR) $<

#---------------------------------------------------------------------------------------------------

