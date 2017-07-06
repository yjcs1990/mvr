####
#### Mobile Vacuum Robot - MVR
#### Mvri top level makefile
####
####

# Make sure we're using a compiler (we probably don't need to define
# it but we have been so here it is unless it was already set)
ifndef CXX
CXX:=g++
endif

####
#### General vmvriables
####

CFILEEXT:=cpp
# this is set up with the extra layer since the python wrapper needs exceptions
# but I didn't want to have two sets of defines.
# BARECXXFLAGS is used when compiling python and java wrapper code
# CXXFLAGS is used when compiling everything else (libraries, static libraries, examples)
BARECXXFLAGS=-g -Wall -D_REENTRANT  #-pg -fprofile-arcs
CXXFLAGS+=$(BARECXXFLAGS) -fno-exceptions $(EXTRA_CXXFLAGS)
CXXINC:=-Iinclude
CXXLINK=-Llib -lMvria
CXXSTATICLINK:=-Llib -Xlinker -Bstatic -lMvria -Xlinker -Bdynamic 

host:=$(shell uname | cut -d _ -f 1)
ifeq ($(host),MINGW32)
    $(info Building on MinGW)
	#CXXFLAGS+=-mwindows -mms-bitfields -D__MINGW__ -DMINGW
	BARECXXFLAGS+=-mms-bitfields -D__MINGW__ -DMINGW
	CXXLINK+=-lpthreadGC2 -lwinmm -lws2_32 -lstdc++
	CXXSTATICLINK+=-Wl,-Bstatic -lpthread -Wl,-Bdynamic -lwinmm -lws2_32 -lstdc++
	binsuffix:=.exe
else
	BARECXXFLAGS+=-fPIC
	CXXLINK+=-lpthread -ldl -lrt
	CXXSTATICLINK+=-Xlinker -Bdynamic -lpthread -ldl -lrt -Xlinker -Bstatic -lstdc++ -Xlinker -Bdynamic
	binsuffix:=
endif


# ifndef JAVAC
# ifdef JAVA_BIN
# JAVAC:=$(JAVA_BIN)/javac
# else
# JAVAC:=javac
# endif #ifdef JAVA_BIN
# endif #ifndef JAVAC

# ifndef JAR
# ifdef JAVA_BIN
# JAR:=$(JAVA_BIN)/jar
# else
# JAR:=jar
# endif #ifdef JAVA_BIN
# endif #ifndef JAR

# ifndef JAVA_INCLUDE
# JAVA_INCLUDE:=/usr/lib/jvm/default-java/include
# endif

####
#### Mvria build vmvriables
####

# Default targets to build in the default rule:
# TARGETS:=lib/libMvria.so examples/demo$(binsuffix)

# Default static libraries and examples:
# STATIC_TARGETS:=lib/libMvria.a examples/demoStatic$(binsuffix)

# Lots of targets, to build in the everything rule:
# ALL_TARGETS:=lib/libMvria.so utils examples tests docs params lib/libMvrNetworking.so swig arnetworking_docs arnetworking_wrappers clib/libMvrNetworking.a $(STATIC_TARGETS)
# ALL_TARGETS:=lib/libMvria.so utils examples tests docs params lib/libMvrNetworking.so swig arnetworking_docs arnetworking_wrappers clib/libMvrNetworking.a $(STATIC_TARGETS)

CFILES:= \
	MvrAction.cpp \
	MvrArg.cpp \
	MvrArgumentBuilder.cpp \
	MvrArgumentParser.cpp \
	MvrASyncTask.cpp \
	MvrBasePacket.cpp \
	MvrCondition.cpp \
	MvrConfig.cpp \
	MvrConfigArg.cpp \
	MvrConfigGroup.cpp \
	MvrDataLogger.cpp \
	MvrDeviceConnection.cpp \
  MvrFileDeviceConnection.cpp \
	MvrFileParser.cpp \
	MvrFunctorASyncTask.cpp \
	MvrInterpolation.cpp \
	MvrJoyHandler.cpp \
	MvrKeyHandler.cpp \
	MvrLog.cpp \
	MvrLogFileConnection.cpp \
	MvrMode.cpp \
	MvrModes.cpp \
	MvrModule.cpp \
	MvrModuleLoader.cpp \
	MvrMutex.cpp \
	MvrRobot.cpp \
	MvrRobotBatteryPacketReader.cpp \
	MvrRobotConfig.cpp \
	MvrRobotConfigPacketReader.cpp \
	MvrRobotConnector.cpp \
	MvrRobotJoyHandler.cpp \
	MvrRobotPacket.cpp \
	MvrRobotPacketReceiver.cpp \
	MvrRobotPacketReaderThread.cpp \
	MvrRobotPacketSender.cpp \
	MvrRobotParams.cpp \
	MvrRobotTypes.cpp \
	MvrSimpleConnector.cpp \
	MvrSocket.cpp \
	MvrStringInfoGroup.cpp \
	MvrSyncLoop.cpp \
	MvrSyncTask.cpp \
	MvrSystemStatus.cpp \
	MvrThread.cpp \
	MvrTransform.cpp \
	Mvria.cpp \
	mvriaUtil.cpp \
	md5.cpp

# Omit some Linux-only classes, and replace others with Win32 implementations.
# ifeq ($(host),MINGW32)
# 	CFILES+=MvrSocket_WIN.cpp \
# 		MvrJoyHandler_WIN.cpp \
# 		MvrSerialConnection_WIN.cpp \
# 		MvrSignalHandler_WIN.cpp
# else
# 	CFILES+=MvrSocket_LIN.cpp \
# 		MvrJoyHandler_LIN.cpp \
# 		MvrSerialConnection_LIN.cpp \
# 		MvrSignalHandler_LIN.cpp \
# 		MvrVersalogicIO.cpp \
#     MvrMTXIO.cpp
# endif




####
#### Utility vmvriables. No need to touch these.
####

OTFILES:=$(patsubst %.$(CFILEEXT),%.o,$(CFILES))
OFILES:=$(patsubst %,obj/%,$(OTFILES))
EXAMPLES_CPP:=$(shell find examples -name "*.$(CFILEEXT)" | grep -v Mod.cpp | grep -v proprietary)
EXAMPLES:=$(patsubst %.$(CFILEEXT),%$(binsuffix),$(EXAMPLES_CPP))
EXAMPLES_STATIC:=$(patsubst %,%Static$(binsuffix),$(EXAMPLES))
MOD_EXAMPLES_CPP:=$(shell find examples -name "*.$(CFILEEXT)" | grep Mod.cpp)
MOD_EXAMPLES:=$(patsubst %.$(CFILEEXT),%.so,$(MOD_EXAMPLES_CPP))
TESTS_CPP:=$(shell find tests -name "*.$(CFILEEXT)" | grep -v Mod.cpp | grep -v proprietary)
MOD_TESTS_CPP:=$(shell find tests -name "*Mod.$(CFILEEXT)")
MOD_TESTS:=$(patsubst %.$(CFILEEXT),%.so,$(MOD_TESTS_CPP))
TESTS:=$(patsubst %.$(CFILEEXT),%$(binsuffix),$(TESTS_CPP))
TESTS_STATIC:=$(patsubst %,%Static$(binsuffix),$(TESTS))
ADVANCED_CPP:=$(shell find advanced -name "*.$(CFILEEXT)" | grep -v Mod.cpp | grep -v proprietary)
ADVANCED:=$(patsubst %.$(CFILEEXT),%,$(ADVANCED_CPP))
UTILS_CPP:=$(shell find utils -name "*.$(CFILEEXT)")
UTILS:=$(patsubst %.$(CFILEEXT),%$(binsuffix),$(UTILS_CPP))
SRC_FILES:=$(patsubst %,src/%,$(CFILES))
HEADER_FILES:=$(shell find include -type f -name \*.h)




####
#### General rules for user invocation
####

# Default Rule
all: dirs $(TARGETS)

# Build all targets, docs, params, etc. etc.
everything: dirs $(ALL_TARGETS) 

static: dirs $(STATIC_TARGETS)

# Build everything required for distribution packages
ifdef NO_DIST_WRAPPERS
dist-all: lib/libMvria.so params docs CommandLineOptions.txt.in \
  lib/libMvrNetworking.so MvrNetworking/CommandLineOptions.txt.in examples/demo \
  examples/demoStatic MvrNetworking/examples/serverDemo \
  MvrNetworking/examples/serverDemoStatic MvrNetworking/examples/clientDemo \
  MvrNetworking/examples/clientDemoStatic \
  arnetworking_docs
else
dist-all: lib/libMvria.so params docs CommandLineOptions.txt.in \
  lib/libMvrNetworking.so MvrNetworking/CommandLineOptions.txt.in examples/demo \
  examples/demoStatic MvrNetworking/examples/serverDemo \
  MvrNetworking/examples/serverDemoStatic MvrNetworking/examples/clientDemo \
  MvrNetworking/examples/clientDemoStatic java python  arnetworking_wrappers \
  arnetworking_docs
endif

examples: $(EXAMPLES) 

modExamples: $(MOD_EXAMPLES)

tests: $(TESTS) $(MOD_TESTS)

advanced: $(ADVANCED)

utils: $(UTILS)

cleanDep:
	-rm Makefile.dep `find . -name Makefile.dep`

# directories that might not exist:
dirs:
	@mkdir -p -v obj
	@mkdir -p -v lib


# Rules to generate API documentation for local/personal use (rather than part of a
# released package; that uses the packaging shell scripts instead)
DATESTAMP=$(shell date +%Y%m%d)
DATE=$(shell date +'%B %d, %Y')
docs: doc
doc: docs/index.html
docs/index.html: $(SRC_FILES) $(HEADER_FILES) $(EXAMPLES_CPP) docs/overview.dox docs/options/all_options.dox docs/params.dox doxygen.conf.in
	@echo
	@echo Removing old documentation...
	$(MAKE) cleanDoc
	@echo
	@echo Making local documentation with version $(DATESTAMP)
	sed -e "s/@VERSION@/dev$(DATESTAMP)/g" -e "s/@ARIA_VERSION@/dev$(DATESTAMP)/g" -e "s/@DATE@/$(DATE)/g" <doxygen.conf.in >doxygen.conf
	doxygen doxygen.conf
	@echo
	@echo Done autogenerating ARIA API documentation. Open docs/index.html in a web browser.
	@echo Use \"make cleanDocs\" to remove autogenerated documentation.
	@echo Use \"make arnetworking_docs\" or \"make docs\" inside the MvrNetworking directory for MvrNetworking API docs.

arnetworking_wrappers:
	$(MAKE) -C MvrNetworking java python

arnetworking_java:
	$(MAKE) -C MvrNetworking java

arnetworking_python:
	$(MAKE) -C MvrNetworking python

arnetworking_docs:
	$(MAKE) -C MvrNetworking docs

arnetworking_all:
	$(MAKE) -C MvrNetworking all

help:
	@echo To make most things, run \'make\' or \'make all\'
	@echo Some useful targets include: 
	@echo "  allLibs, cleanAllLibs, depAllLibs (do all subdirectories)"
	@echo "  clean"
	@echo "  dep"
	@echo "  cleanDep"
	@echo "  docs"
	@echo "  cleanAll (also cleans MvrNetworking, java, python, etc.)"
	@echo "  examples"
	@echo "  tests"
	@echo "  utils"
	@echo "  python" 
	@echo "  cleanPython"
	@echo "  java" 
	@echo "  cleanJava"
	@echo "  examples/<Some Example>, where examples/<Some Example>.cpp is an example program source file."
	@echo "  tests/<Some Test>, where tests/<Some Test>.cpp is a test program source file."
	@echo "  allLibs (try to build all auxilliary libraries that you have installed by running make in each directory starting with \"Mvr\")"
	@echo "  cleanAllLibs, depAllLibs (do make clean or make dep in the \"Mvr*\" auxilliary libraries)"
	@echo "  install (if this is a source tar.gz package)"

info:
	@echo ARIA=$(ARIA)
	@echo CXX=$(CXX)
	@echo CXXFLAGS=$(CXXFLAGS)
	@echo BARECXXFLAGS=$(BARECXXFLAGS)
	@echo CXXINC=$(CXXINC)
	@echo CXXLINK=$(CXXLINK)
	@echo CXXSTATICLINK=$(CXXSTATICLINK)
	@echo ALL_TARGETS=$(ALL_TARGETS)
	@echo
	@echo JAVAC=$(JAVAC)
	@echo JAR=$(JAR)
	@echo
	@echo Use \'make moreinfo\' for info about individual files, etc.

moreinfo:
	@echo CFILES=$(CFILES)
	@echo
	@echo EXAMPLES=$(EXAMPLES)
	@echo
	@echo MOD_EXAMPLES=$(MOD_EXAMPLES)
	@echo
	@echo TESTS=$(TESTS)
	@echo
	@echo MOD_TESTS=$(MOD_TESTS)
	@echo
	@echo ADVANCED=$(ADVANCED)
	@echo
	@echo UTILS=$(UTILS)
	@echo
	@echo SRC_FILES=$(SRC_FILES)
	@echo
	@echo HEADER_FILES=$(HEADER_FILES)



markdown_format:=markdown #+multiline_tables #+simple_tables
%.rtf: %.md
	pandoc -s --toc --template pandoc_template -f $(markdown_format) -t rtf -o $@ $<

%.txt: %.md
	pandoc -s --toc -f $(markdown_format) -t plain -o $@ $<

%.pdf: %.md
	pandoc -s --toc -f $(markdown_format) -o $@ $<

%.html: %.md
	pandoc -s --toc -f $(markdown_format) -o $@ $<


clean: cleanUtils cleanExamples cleanModules cleanTests cleanAdvanced 
	-rm -f lib/libMvria.a lib/libMvria.so $(OFILES) `find . -name core` `find . -name '*~'` obj/MvriaPy.o obj/MvriaJava.o

cleanUtils:
	-rm -f $(UTILS)

cleanExamples:
	-rm -f $(EXAMPLES) $(EXAMPLES_STATIC)

cleanTests:
	-rm -f $(TESTS) $(TESTS_STATIC)

cleanAdvanced:
	-rm -f $(ADVANCED)

cleanModules:
	-rm -f $(MOD_EXAMPLES)

cleanDoc:
	-rm  docs/*.html docs/*.png docs/doxygen.css

cleanDocs: cleanDoc

cleandoc: cleanDoc

cleandocs: cleanDoc

cleanPython:
	-rm python/_MvriaPy.so
	-rm python/MvriaPy.py
	-rm python/MvriaPy.pyc
	-rm python/MvriaPy_wrap.cpp
	-rm python/MvriaPy_wrap.h
	-rm obj/MvriaPy_wrap.o

cleanpython: cleanPython

# Force rebuild of Makefile.dep:
dep: cleanDep clean
	$(MAKE) Makefile.dep

Makefile.dep:
	if [ -f `echo src/*.cpp | cut -d' ' -f1` ]; then \
	$(CXX) $(CXXFLAGS) $(CXXINC) -MM src/*.cpp | \
	awk '$$1 ~ /:/{printf "obj/%s\n", $$0} $$1 !~ /:/' > Makefile.dep; fi
	if [ -f `echo examples/*.cpp | cut -d' ' -f1` ]; then \
	$(CXX) $(CXXFLAGS) $(CXXINC) -MM examples/*.cpp | \
	awk '$$1 ~ /:/{printf "examples/%s$(binsuffix)\n", $$0} $$1 !~ /:/' | \
	sed 's/\.o//' >> Makefile.dep; fi
	if [ -f `echo utils/*.cpp | cut -d' ' -f1` ]; then \
	$(CXX) $(CXXFLAGS) $(CXXINC) -MM utils/*.cpp | \
	awk '$$1 ~ /:/{printf "utils/%s$(binsuffix)\n", $$0} $$1 !~ /:/' | \
	sed 's/\.o//' >> Makefile.dep; fi
	if [ -f `echo tests/*.cpp | cut -d' ' -f1` ]; then \
	$(CXX) $(CXXFLAGS) $(CXXINC) -MM tests/*.cpp | \
	awk '$$1 ~ /:/{printf "tests/%s$(binsuffix)\n", $$0} $$1 !~ /:/' | \
	sed 's/\.o//' >> Makefile.dep; fi
	if [ -f `echo advanced/*.cpp | cut -d' ' -f1` ]; then \
	$(CXX) $(CXXFLAGS) $(CXXINC) -MM advanced/*.cpp | \
	awk '$$1 ~ /:/{printf "advanced/%s$(binsuffix)\n", $$0} $$1 !~ /:/' | \
	sed 's/\.o//' >> Makefile.dep; fi

depAll: cleanDep
	make dep;
	for dir in `find . -maxdepth 1 -name "Mvr*" -xtype d`; do cd $$dir; make dep; cd ..; done

cleanAll: clean 
	rm -f lib/lib*.so*
	for dir in `find . -maxdepth 1 -name "Mvr*" -xtype d`; do cd $$dir; make clean; cd ..; done


fullCleanAll: cleanAll cleanJava cleanPython 
	rm -f lib/lib*.so*
	rm -f obj/*.o*
	for dir in `find . -maxdepth 1 -name "Mvr*" -xtype d`; do cd $$dir; make fullClean; cd ..; done



params: utils/makeParams$(binsuffix)
	-mkdir params
	utils/makeParams$(binsuffix)

# These files are generated by running a utility that outputs command line
# option information from various MVRIA classes.
CommandLineOptions.txt.in docs/options/all_options.dox: utils/genCommandLineOptionDocs$(binsuffix) 
	-mkdir docs/options
	LD_LIBRARY_PATH=lib:$$LD_LIBRARY_PATH utils/genCommandLineOptionDocs$(binsuffix)

alllibs: allLibs

allLibs: all 
	find . -type d -and -name Mvr\* -maxdepth 1 -exec $(MAKE) -C \{\}  \;

cleanAllLibs: clean cleanJava cleanPython cleanDep
	for dir in `find . -maxdepth 1 -name "Mvr*" -xtype d`; do $(MAKE) -C $$dir clean cleanAll; done

cleanalllibs: cleanAllLibs

distclean: clean cleanUtils cleanExamples cleanAdvanced cleanModules cleanDep
	$(MAKE) -C MvrNetworking clean cleanDep
	-rm python/\*.pyc python/\*.so

	

checkAll:
	$(MAKE) everything > checkAll.log 2>&1
	$(MAKE) allLibs >> checkAll.log 2>&1
	grep -n -v 'Error 1 (ignored)' checkAll.log | grep -C 4 'Error' > checkAll-Errors.log
	@echo
	@echo Errors were:
	@echo
	@cat checkAll-Errors.log
	@echo
	@echo Error log also saved as checkAll-Errors.log.  Full log saved as checkAll.log.

####
#### Swig wrappers
####

ifndef SWIG
SWIG=swig
endif


####
#### Targets to actually build binaries (libraries, programs)
####

lib/libMvria.so: $(OFILES) 
	$(CXX) -shared -o $(@) $(OFILES) -lpthread -ldl -lrt

lib/libMvria.a: $(OFILES) 
	ar -cr $(@) $(OFILES)
	ranlib $(@)

examples/%.so: examples/%.$(CFILEEXT) lib/libMvria.so 
	$(CXX) $(CXXFLAGS) $(CXXINC) -shared $< -o $@

examples/%$(binsuffix): examples/%.$(CFILEEXT) lib/libMvria.so 
	$(CXX) $(CXXFLAGS) $(CXXINC) $< -o $@ $(CXXLINK)

examples/%Static$(binsuffix): examples/%.$(CFILEEXT) lib/libMvria.a 
	$(CXX) $(CXXFLAGS) $(CXXINC) $< -o $@ $(CXXSTATICLINK)
	if test -z "$$NOSTRIP"; then strip $@; fi

tests/%.so: tests/%.$(CFILEEXT) lib/libMvria.so 
	$(CXX) $(CXXFLAGS) $(CXXINC) -shared $< -o $@

tests/%$(binsuffix): tests/%.$(CFILEEXT) lib/libMvria.so 
	$(CXX) $(CXXFLAGS) $(CXXINC) $< -o $@ $(CXXLINK)

tests/%Static$(binsuffix): tests/%.$(CFILEEXT) lib/libMvria.a 
	$(CXX) $(CXXFLAGS) $(CXXINC) $< -o $@ $(CXXSTATICLINK)
	if test -z "$$NOSTRIP"; then strip $@; fi

advanced/%.so: advanced/%.$(CFILEEXT) lib/libMvria.so
	$(CXX) $(CXXFLAGS) $(CXXINC) -shared $< -o $@

advanced/%$(binsuffix): advanced/%.$(CFILEEXT) lib/libMvria.so 
	$(CXX) $(CXXFLAGS) $(CXXINC) $< -o $@ $(CXXLINK)

advanced/%Static$(binsuffix): advanced/%.$(CFILEEXT) lib/libMvria.a 
	$(CXX) $(CXXFLAGS) $(CXXINC) $< -o $@ $(CXXSTATICLINK)
	#strip $@

utils/%$(binsuffix): utils/%.$(CFILEEXT) lib/libMvria.so 
	$(CXX) $(CXXFLAGS) $(CXXINC) $< -o $@ $(CXXLINK)

utils/%Static$(binsuffix): utils/%.$(CFILEEXT) lib/libMvria.a 
	$(CXX) $(CXXFLAGS) $(CXXINC) $< -o $@ $(CXXSTATICLINK)
	#strip $@

# This utility prints information about command line options recognized by
# various classes in ARIA (e.g. MvrRobotConnector, MvrLaserConnector (for various
# laser types), etc.
utils/genCommandLineOptionDocs$(binsuffix): utils/genCommandLineOptionDocs.cpp lib/libMvria.so 
	$(CXX) $(CXXFLAGS) -DFOR_ARIA $(CXXINC) $< -o $@ $(CXXLINK)

ifneq ($(binsuffix),)
examples/%: examples/%$(binsuffix)
utils/%: utils/%$(binsuffix)
advanced/%: advanced/%$(binsuffix)
tests/%: tests/$(binsuffix)
endif

obj/%.o : src/%.cpp 
	@mkdir -p obj
	$(CXX) -c $(CXXFLAGS) $(CXXINC) $< -o $@

obj/%.o : src/%.c 
	@mjdir -p obj
	$(CXX) -c $(CXXFLAGS) $(CXXINC) $< -o $@

include/%.i: include/%.h 
	$(CXX) -E $(CXXFLAGS) $(CXXINC) $< -o $@

src/%.i: src/%.cpp 
	$(CXX) -E $(CXXFLAGS) $(CXXINC) $< -o $@

# Don't build .o files if their library is up to date with respect to source files:
.INTERMEDIATE: $(OFILES)

# But don't delete .o files if we do make them in order to make a library:
.PRECIOUS: $(OFILES)


# To build things in the MvrNetworking subdirectory:

lib/libMvrNetworking.so: FORCE
	$(MAKE) -C MvrNetworking ../$@

lib/libMvrNetworking.a: FORCE 
	$(MAKE) -C MvrNetworking ../$@

MvrNetworking/examples/%: MvrNetworking/examples/%.cpp lib/libMvrNetworking.so
	$(MAKE) -C MvrNetworking examples/$*

MvrNetworking/examples/%Static: MvrNetworking/examples/%.cpp lib/libMvrNetworking.a
	$(MAKE) -C MvrNetworking examples/$*Static

#MvrNetworking/docs/index.html: FORCE
#	$(MAKE) -C MvrNetworking docs/index.html

MvrNetworking/%: FORCE
	$(MAKE) -C MvrNetworking $*

# This rule doesn't actually produce its target, so anything depending on it
# will always get remade.
FORCE:


####
#### Installation and distribution 
####


ifndef INSTALL_DIR
INSTALL_DIR=/usr/local/Mvria
endif

ifndef SYSTEM_ETC_DIR
SYSTEM_ETC_DIR=/etc
endif

# What to put in /etc/Mvria:
ifndef STORED_INSTALL_DIR
STORED_INSTALL_DIR=$(INSTALL_DIR)
endif

# How to run 'install' for the install rule:
ifndef INSTALL
INSTALL:=install --preserve-timestamps
endif


dist: FORCE
	dist/dist.sh

dist-install: install

# Install rule.  This can be used by users or ARIA developers; in the latter
# case it also installs various files needed to make a release distribution.
# Override installation locations with INSTALL_DIR environment vmvriable.
# Things are installed group-writable so as to be hacked upon.
install: all arnetworking_all
	@echo      	--------------------------------------
	@echo		    Installing ARIA in $(DESTDIR)$(INSTALL_DIR)...
	@echo      	--------------------------------------
	$(INSTALL) -m 775 -d $(DESTDIR)$(INSTALL_DIR)
	find    include src tests utils params docs examples advanced maps \
	        MvrNetworking java javaExamples python pythonExamples  obj \
	        \( -name \*.o -or -name core -or -name CVS -or -name .\* -or -name \*~ -or -name tmp -or -name proprietary* -or -name \*.bak -or -name \*.class \) -prune  \
	        -or -type d   -exec $(INSTALL) -d -m 777 $(DESTDIR)$(INSTALL_DIR)/\{\} \; \
	        -or -type l   -exec cp --no-dereference \{\} $(DESTDIR)$(INSTALL_DIR)/\{\} \; \
	        -or -name \*.a -exec $(INSTALL) -D -m 666 \{\}  $(DESTDIR)$(INSTALL_DIR)/\{\} \; \
	        -or -perm /u=x  -exec $(INSTALL) -D --strip -m 777 \{\}  $(DESTDIR)$(INSTALL_DIR)/\{\} \; \
	        -or           -exec $(INSTALL) -D -m 666 \{\} $(DESTDIR)$(INSTALL_DIR)/\{\} \;
	$(INSTALL) -D -m 664 LICENSE.txt INSTALL.txt README.txt Makefile Mvria-Reference.html version.txt Changes.txt CommandLineOptions.txt icon.png $(DESTDIR)$(INSTALL_DIR)/
	$(INSTALL) -D -m 666  Makefile.dep doxygen.conf $(DESTDIR)$(INSTALL_DIR)/
	$(INSTALL) -d -m 777 $(DESTDIR)$(INSTALL_DIR)/lib/
	$(INSTALL) -D --strip -m 666 lib/libMvria.so lib/libMvrNetworking.so $(DESTDIR)$(INSTALL_DIR)/lib/
	if test -f lib/libMvriaJava.so; then $(INSTALL) --strip -m 666 lib/libMvriaJava.so $(DESTDIR)$(INSTALL_DIR)/lib/; else echo Warning: lib/libMvriaJava.so not found. Use \"make java\" to build.; fi
	if test -f lib/libMvrNetworkingJava.so; then $(INSTALL) --strip -m 666 lib/libMvrNetworkingJava.so $(DESTDIR)$(INSTALL_DIR)/lib/; else echo Warning: lib/libMvrNetworkingJava.so not found. Enter the MvrNetworking directory and run \"make java\" to build.; fi
	@if test -z "$(DIST_INSTALL)"; then \
		echo "if test \! -d $(DESTDIR)$(SYSTEM_ETC_DIR); then install -d $(DESTDIR)$(SYSTEM_ETC_DIR); fi" ;\
		if test \! -d $(DESTDIR)$(SYSTEM_ETC_DIR); then install -d $(DESTDIR)$(SYSTEM_ETC_DIR); fi ;\
		echo "echo $(STORED_INSTALL_DIR) > $(DESTDIR)$(SYSTEM_ETC_DIR)/Mvria" ;\
		echo $(STORED_INSTALL_DIR) > $(DESTDIR)$(SYSTEM_ETC_DIR)/Mvria ;\
		echo       ------------------------------------------------------------------------------------ ;\
		echo       ARIA has been installed in $(DESTDIR)$(INSTALL_DIR). ;\
		echo ;\
		echo       To be able to use the ARIA libraries, you must now add $(DESTDIR)$(INSTALL_DIR)/lib ;\
		echo       to your LD_LIBRARY_PATH environment vmvriable, or to the /etc/ld.so.conf system file, ;\
		echo       then run \'ldconfig\';\
		echo     	 ------------------------------------------------------------------------------------ ;\
	fi

#		echo ;\
#		echo       The files are owned by the group \"users\", and all members of that group ;\
#		echo       can enter the directory, read files, and modify the \"examples\" directory. ;\


# Source Code Install rule.  This is used to make source-code only packages.
# It includes files needed for all platforms.
srcdist-install: src-install

src-install:
	$(INSTALL) -m 775 -d $(DESTDIR)$(INSTALL_DIR)
	find    include src tests utils params docs examples advanced maps \
	        MvrNetworking java javaExamples python pythonExamples  \
	        \( -name java/com -or -name \*.jar -or -name \*.a -or -name \*.so -or -name \*.o -or -name core -or -name CVS -or -name .\* -or -name \*~ -or -name tmp -or -name proprietary* -or -name \*.bak -or -name \*.class \) -prune  \
	        -or -type d   -exec $(INSTALL) -d -m 744 $(DESTDIR)$(INSTALL_DIR)/\{\} \; \
	        -or -type l   -exec cp --no-dereference \{\} $(DESTDIR)$(INSTALL_DIR)/\{\} \; \
	        -or -perm /u=x  -prune \
	        -or           -exec $(INSTALL) -D -m 644 \{\} $(DESTDIR)$(INSTALL_DIR)/\{\} \;
	$(INSTALL) -D -m 644 LICENSE.txt INSTALL.txt README.txt Makefile Mvria-Reference.html version.txt Changes.txt CommandLineOptions.txt icon.png $(DESTDIR)$(INSTALL_DIR)/
	$(INSTALL) -D -m 644  Makefile.dep doxygen.conf $(DESTDIR)$(INSTALL_DIR)/
	$(INSTALL) -D -m 644 bin/SIMULATOR_README.txt $(DESTDIR)$(INSTALL_DIR)/bin/SIMULATOR_README.txt


# Make optimization, tell it what rules aren't files:
.PHONY: all everything examples modExamples tests advanced utils cleanDep docs doc dirs help info moreinfo clean cleanUtils cleanExamples cleanTests cleanAdvanced cleanModules cleanDoc cleanPython dep depAll cleanAll params allLibs python python-doc java cleanJava alllibs arnetworking_wrappers arnetworking_docs params swig help info moreinfo py python-doc cleanSwigJava checkAll dirs install srcdist-install dist-install src-install distclean

# Include Autogenerated dependencies, using Makefile.dep rule above to generate
# this file if needed:
include Makefile.dep

