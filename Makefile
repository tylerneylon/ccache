# CCache Makefile
#
# The primary rules are:
#
# * all -- Builds everything in the out/ directory.
# * test -- Builds and runs all tests, printing out the results.
# * clean -- Deletes everything this makefile may have created.
#

#################################################################################
# Variables for targets.

# Target lists.
tests = out/cachetest
common_obj = $(addprefix out/,CArray.o CList.o CMap.o memprofile.o ctest.o)
test_obj = $(common_obj) out/ccache_test.o
obj = $(common_obj) out/ccache.o

# Variables for build settings.
includes = -Icstructs -I.
cflags = $(includes)
cc = clang $(cflags)

# Test-running environment.
testenv = DYLD_INSERT_LIBRARIES=/usr/lib/libgmalloc.dylib MALLOC_LOG_FILE=/dev/null

#################################################################################
# Primary rules; meant to be used directly.

# Build everything.
all: $(obj) $(tests)

# Build all tests.
test: $(tests)
	@echo Running tests:
	@echo -
	@for test in $(tests); do $(testenv) $$test || exit 1; done
	@echo -
	@echo All tests passed!

clean:
	rm -rf out

#################################################################################
# Internal rules; meant to only be used indirectly by the above rules.

out:
	mkdir -p out

out/ctest.o: test/ctest.c test/ctest.h | out
	$(cc) -o $@ -c test/ctest.c

out/ccache.o: ccache.c ccache.h | out
	$(cc) -o $@ -c ccache.c

out/ccache_test.o: ccache.c ccache.h | out
	$(cc) -o $@ -c ccache.c -D CCACHE_TESTING

out/%.o : cstructs/%.c cstructs/%.h | out
	$(cc) -o $@ -c $<

$(tests) : out/% : test/%.c $(test_obj)
	$(cc) -o $@ $^

# Listing this special-name rule prevents the deletion of intermediate files.
.SECONDARY:

# The PHONY rule tells the makefile to ignore directories with the same name as a rule.
.PHONY : test
