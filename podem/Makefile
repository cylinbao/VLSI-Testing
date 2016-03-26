CXX      = g++
LINK     = g++
# Remove -DDEBUG flag for VLST-Testing Lab0
CXXFLAGS = -O2 -Wall
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
SOURCES = readcircuit.tab.cc lex.yy.cc circuit.cc main.cc GetLongOpt.cc atpg.cc fsim.cc sim.cc psim.cc stfsim.cc tfatpg.cc path.cc pattern.cc

# Add object file assignment.o to be linked
OBJECTS = readcircuit.tab.o lex.yy.o circuit.o main.o GetLongOpt.o atpg.o fsim.o sim.o psim.o stfsim.o tfatpg.o path.o pattern.o

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

clean:
	@$(DEL_FILE) $(OBJECTS) $(TARGET)
	@$(DEL_FILE) readcircuit.tab.cc readcircuit.tab.h lex.yy.cc
	@$(DEL_DIR)	 output input

lab2.1-17-1:
	$(MEMUSG) ./$(TARGET) -pattern -num 100 -output c17.output ../circuits/iscas85/c17.bench

lab2.1-17U-1:
	$(MEMUSG) ./$(TARGET) -pattern -num 100 -unknown -output c17.output ../circuits/iscas85/c17.bench

lab2.1-17-2:
	$(MEMUSG) ./$(TARGET) -pattern -num 1000 -output c17.output ../circuits/iscas85/c17.bench

lab2.1-17U-2:
	$(MEMUSG) ./$(TARGET) -pattern -num 1000 -unknown -output c17.output ../circuits/iscas85/c17.bench

lab2.1-17-3:
	$(MEMUSG) ./$(TARGET) -pattern -num 10000 -output c17.output ../circuits/iscas85/c17.bench

lab2.1-17U-3:
	$(MEMUSG) ./$(TARGET) -pattern -num 10000 -unknown -output c17.output ../circuits/iscas85/c17.bench

lab2.1-7552-1:
	$(MEMUSG) ./$(TARGET) -pattern -num 100 -output c7552.output ../circuits/iscas85/c7552.bench

lab2.1-7552U-1:
	$(MEMUSG) ./$(TARGET) -pattern -num 100 -unknown -output c7552.output ../circuits/iscas85/c7552.bench

lab2.1-7552-2:
	$(MEMUSG) ./$(TARGET) -pattern -num 1000 -output c7552.output ../circuits/iscas85/c7552.bench

lab2.1-7552U-2:
	$(MEMUSG) ./$(TARGET) -pattern -num 1000 -unknown -output c7552.output ../circuits/iscas85/c7552.bench

lab2.1-7552-3:
	$(MEMUSG) ./$(TARGET) -pattern -num 10000 -output c7552.output ../circuits/iscas85/c7552.bench

lab2.1-7552U-3:
	$(MEMUSG) ./$(TARGET) -pattern -num 10000 -unknown -output c7552.output ../circuits/iscas85/c7552.bench

lab2.2-17:
	$(MEMUSG) ./$(TARGET) -mod_logicsim -input ./input/c17.input ../circuits/iscas85/c17.bench

lab2.2-7552:
	$(MEMUSG) ./$(TARGET) -mod_logicsim -input ./input/c7552.input ../circuits/iscas85/c7552.bench

lab2-17:
	$(MEMUSG) ./$(TARGET) -logicsim -input ./input/c17.input ../circuits/iscas85/c17.bench

lab2-7552:
	$(MEMUSG) ./$(TARGET) -logicsim -input ./input/c7552.input ../circuits/iscas85/c7552.bench
