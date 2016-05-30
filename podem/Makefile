CXX      = g++
LINK     = g++
# Remove -DDEBUG flag for VLST-Testing Lab0
CXXFLAGS = -O2 -Wall -g
#CXXFLAGS = -O2 -Wall -DDEBUG
#CXXFLAGS = -g -DDEBUG_ATPG -Wall #Turn on debugging
INCPATH  =
LIBS     = -lreadline -lcurses
DEL_FILE = rm -f
DEL_DIR  = rm -fr
MEMUSG	 = /usr/bin/time -f "Average memory usage: %K \nMaximum memory usage: %M\n"

####### Files

HEADERS = gate.h fault.h circuit.h GetLongOpt.h typeemu.h readcircuit.tab.h pattern.h hash.h tfault.h

# Add source file assignment.c to be compiled
SOURCES = readcircuit.tab.cc lex.yy.cc circuit.cc main.cc GetLongOpt.cc atpg.cc \
					fsim.cc sim.cc psim.cc stfsim.cc tfatpg.cc path.cc pattern.cc \
					compiledCodeSim.cc checkpoint.cc bridging.cc bfsim.cc

# Add object file assignment.o to be linked
OBJECTS = readcircuit.tab.o lex.yy.o circuit.o main.o GetLongOpt.o atpg.o \
					fsim.o sim.o psim.o stfsim.o tfatpg.o path.o pattern.o \
					compiledCodeSim.o checkpoint.o bridging.o bfsim.o

TARGET  = atpg

####### Implicit rules

.SUFFIXES: .o .cpp .cc .cxx

.cpp.o:
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $@ $<

.cc.o:
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $@ $<

####### Build rules
all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(LINK) $(CXXFLAGS) -o $(TARGET) $(OBJECTS) $(LIBS) $(INCPATH)

$(OBJECTS): $(HEADERS) Makefile

readcircuit.tab.cc: readcircuit.y gate.h circuit.h
	bison -d -t readcircuit.y
	mv readcircuit.tab.c readcircuit.tab.cc

lex.yy.cc: readcircuit.l readcircuit.y
	flex readcircuit.l 
	mv lex.yy.c lex.yy.cc

.PHONY: clean c17 c880 c7552 s35 s384 s385 b17
clean:
	@$(DEL_FILE) $(OBJECTS) $(TARGET)
	@$(DEL_FILE) readcircuit.tab.cc readcircuit.tab.h lex.yy.cc
	@$(DEL_DIR)	 output input simulator

c17:
	./atpg -bridging_fsim -input ../input/c17.input ../circuits/iscas85/c17.bench

c880:
	./atpg -bridging_fsim -input ../input/c880.input ../circuits/iscas85/c880.bench

c7552:
	./atpg -bridging_fsim -input ../input/c7552.input ../circuits/iscas85/c7552.bench

s35:
	rm -f ./input/s35932_com-rp.input
	./atpg -bt 1000 -random_pattern -output ./input/s35932_com-rp.input ../circuits/iscas89_com/s35932_com.bench

s384:
	rm -f ./input/s38417_com-rp.input
	./atpg -bt 1000 -random_pattern -output ./input/s38417_com-rp.input ../circuits/iscas89_com/s38417_com.bench

s385:
	rm -f ./input/s38584_com-rp.input
	./atpg -bt 1000 -random_pattern -output ./input/s38584_com-rp.input ../circuits/iscas89_com/s38584_com.bench

b17:
	rm -f ./input/b17-rp.input
	./atpg -bt 1000 -random_pattern -output ./input/b17-rp.input ../circuits/b17.bench
