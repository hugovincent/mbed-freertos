# Integration of uSTL v1.4 into this project (couldn't make the uSTL configuration
# system cooperate, so I deleted it and rewrote the build system....)
#
# Hugo Vincent, 10 May 2010.

THUMB_CXX_SOURCE+= \
		lib/ustl/cmemlink.cpp \
		lib/ustl/fstream.cpp \
		lib/ustl/memblock.cpp \
		lib/ustl/memlink.cpp \
		lib/ustl/mistream.cpp \
		lib/ustl/ofstream.cpp \
		lib/ustl/sistream.cpp \
		lib/ustl/sostream.cpp \
		lib/ustl/ualgobase.cpp \
		lib/ustl/ubitset.cpp \
		lib/ustl/uexception.cpp \
		lib/ustl/unew.cpp \
		lib/ustl/ustdxept.cpp \
		lib/ustl/ustring.cpp

CXXFLAGS+=\
		-fvisibility-inlines-hidden -fno-threadsafe-statics -finline-limit=1024

