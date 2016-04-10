#include <iostream>
#include "circuit.h"
using namespace std;

// Event-driven Parallel Pattern Logic simulation
void CIRCUIT::genCompiledCodeSimulator()
{
  cout << "Generate Parallel Logic simulator" << endl;

	genHeader();
	genMainBegin();

  unsigned pattern_idx(0);
  while(!Pattern.eof()){ 
		for(pattern_idx=0; pattern_idx<PatternNum; pattern_idx++){
			if(!Pattern.eof()){ 
				++pattern_num;
				Pattern.ReadNextPattern(pattern_idx);
			}
			else break;
		}
		genIniPattern();
		
		ofsMain << endl << "evaluate();" << endl;
		ofsMain << "printIO(" << pattern_idx << ");\n\n"; 

		ScheduleAllPIs();
		ParallelLogicSim();
		//ccsPrintParallelIOs(pattern_idx);
  }
	genMainEnd();
	combineFilesToOutput();

	printStatResult();
}

void CIRCUIT::genHeader()
{
	ofsHeader << "#include <iostream>" << endl;
	ofsHeader << "#include <ctime>" << endl;
	ofsHeader << "#include <bitset>" << endl;
	ofsHeader << "#include <string>" << endl;
	ofsHeader << "#include <fstream>" << endl;
	ofsHeader << "using namespace std;" << endl << endl;
	ofsHeader << "const unsigned PatternNum = 16;" << endl << endl;
	ofsHeader << "void evaluate();" << endl;
	ofsHeader << "void printIO(unsigned idx);" << endl;
}

void CIRCUIT::genMainBegin()
{
	ofsMain << "int main()\n{\n";
}

void CIRCUIT::genIniPattern()
{
	vector<GATE*>::iterator it;

	for(it=Pattern.getInlistPtr()->begin(); it!=Pattern.getInlistPtr()->end(); it++) {
		ofsMain << (*it)->GetName() << "[0]=" << (*it)->getWireValue()[0] << ";" << endl;
		ofsMain << (*it)->GetName() << "[1]=" << (*it)->getWireValue()[1] << ";" << endl;			
	}
}

void CIRCUIT::genMainEnd()
{
	ofsMain << "return 0;\n}\n";
}

void CIRCUIT::combineFilesToOutput()
{
	ofsHeader.close(); ofsMain.close(); ofsEva.close(); ofsPrintIO.close();
	ifstream ifs;

	ifs.open("./simulator/header");
	if(ifs.is_open()){
		ofs << ifs.rdbuf();
		ifs.close();
	}
	ifs.open("./simulator/main");
	if(ifs.is_open()){
		ofs << ifs.rdbuf();
		ifs.close();
	}
	ifs.open("./simulator/evaluate");
	if(ifs.is_open()){
		ofs << ifs.rdbuf();
		ifs.close();
	}
	ifs.open("./simulator/printIO");
	if(ifs.is_open()){
		ofs << ifs.rdbuf();
		ifs.close();
	}
}

void CIRCUIT::ccsPrintParallelIOs(unsigned idx)
{
    register unsigned i;
    for (unsigned j=0; j<idx; j++){
	    for (i = 0;i<No_PI();++i) { 
		    if(PIGate(i)->GetWireValue(0, j)==0){ 
			   if(PIGate(i)->GetWireValue(1, j)==1){
	    			ofs << "F";
			   }
			   else ofs << "0";
		    }
		    else{
			   if(PIGate(i)->GetWireValue(1, j)==1){
	    			ofs << "1";
			   }
			   else ofs << "2";
		    }

	    }
	    ofs << " ";
	    for (i = 0;i<No_PO();++i) { 
		    if(POGate(i)->GetWireValue(0, j)==0){ 
			   if(POGate(i)->GetWireValue(1, j)==1){
	    			ofs << "F";
			   }
			   else ofs << "0";
		    }
		    else{
			   if(POGate(i)->GetWireValue(1, j)==1){
	    			ofs << "1";
			   }
			   else ofs << "2";
		    }
	    }
	    ofs << endl;
    }
    return;
}
