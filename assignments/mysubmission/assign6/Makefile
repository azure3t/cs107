#
# A simple makefile for building project composed of C source files.
#
# Julie Zelenski, for CS107, Nov 2016
#

# It is likely that default C compiler is already gcc, but be explicit anyway
CC = gcc

# The CFLAGS variable sets the flags for the compiler.  CS107 adds these flags:
#  -g          compile with debug information
#  -Og         optimize, but keep code debuggable
#  -std=gnu99  use the C99 standard language definition with GNU extensions
#  -Wall       turn on optional warnings (warnflags configures specific diagnostic warnings)
CFLAGS = -g -O0 -std=gnu99 -Wall -Werror $$warnflags 
export warnflags = -Wfloat-equal -Wtype-limits -Wpointer-arith -Wlogical-op -Wshadow -fno-diagnostics-show-option -fno-stack-protector

# The LDFLAGS variable sets flags for the linker and the LDLIBS variable lists
# additional libraries being linked. The standard libc is linked by default
# and we require no additional libraries or flags, so these are empty.
LDFLAGS = -L.
LDLIBS = -ldatabase -lcvecmap

# The line below defines the variable 'PROGRAMS' to name all of the executables
# to be built by this makefile
PROGRAMS = runexploits 

# The line below defines a target named 'all', configured to trigger the
# build of everything named in the 'PROGRAMS' variable. The first target
# defined in the makefile becomes the default target. When make is invoked
# without any arguments, it builds the default target.
all:: $(PROGRAMS)

# The entry below is a pattern rule. It defines the general recipe to make
# the 'name.o' object file by compiling the 'name.c' source file.
# Add Makefile as a prerequisite so changing flags will trigger rebuild
%.o: %.c Makefile database.h
	$(COMPILE.c) $< -o $@

# This pattern rule defines the general recipe to make the executable 'name'
# by linking the 'name.o' object file and any other .o prerequisites. The
# rule is used for all executables listed in the PROGRAMS definition above.
$(PROGRAMS): %:%.o exploits.o libdatabase.a libcvecmap.a
	$(LINK.o) $(filter %.o,$^) $(LDLIBS) -o $@

# Specific per-target customizations and prerequisites can be listed here

ARFLAGS = rvD
libdatabase.a: database.o
	$(AR) $(ARFLAGS) $@ $?
.INTERMEDIATE: database.o 

# The line below defines the clean target to remove any previous build results
clean::
	rm -f $(PROGRAMS) libdatabase.a core *.o 

# PHONY is used to mark targets that don't represent actual files/build products
.PHONY: clean all

# The line below tries to include our master Makefile, which we use internally.
# The - means that it is not an error if this file can't be found (which will
# normally be the case). You can just ignore this line.

-include Makefile.grading
