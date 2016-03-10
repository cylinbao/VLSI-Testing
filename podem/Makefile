CXX      = g++
LINK     = g++
# Remove -DDEBUG flag for VLST-Testing Lab0
CXXFLAGS = -O2 -Wall -g
#CXXFLAGS = -O2 -Wall -DDEBUG
#CXXFLAGS = -g -DDEBUG_ATPG -Wall #Turn on debugging
INCPATH  =
LIBS     = -lreadline -lcurses
DEL_FILE = rm -f

####### Files

HEADERS = gate.h fault.h circuit.h GetLongOpt.h typeemu.h readcircuit.tab.h ReadPattern.h hash.h tfault.h

# Add source file assignment.c to be compiled
SOURCES = readcircuit.tab.cc lex.yy.cc circuit.cc main.cc GetLongOpt.cc atpg.cc fsim.cc sim.cc psim.cc stfsim.cc tfatpg.cc path.cc

# Add object file assignment.o to be linked
OBJECTS = readcircuit.tab.o lex.yy.o circuit.o main.o GetLongOpt.o atpg.o fsim.o sim.o psim.o stfsim.o tfatpg.o path.o

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

