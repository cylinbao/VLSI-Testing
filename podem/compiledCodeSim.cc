#include <iostream>
#include "circuit.h"
using namespace std;

// Event-driven Parallel Pattern Logic simulation
void CIRCUIT::genCompiledCodeSimulator()
{
  cout << "Generate Parallel Logic simulator" << endl;

	genHeader();
	genMainBegin();
	genEvaBegin();
	genPrintIOBegin();

  unsigned pattern_idx(0);
	bool flag(true);
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
		ccsParallelLogicSim(flag);

		if(flag == true)
			flag = false;
  }
	ccsPrintParallelIOs(pattern_idx);

	genMainEnd();
	genEvaEnd();
	genPrintIOEnd();

	combineFilesToOutput();

	printStatResult();
}

//Simulate PatternNum vectors
void CIRCUIT::ccsParallelLogicSim(bool flag)
{
    GATE* gptr;
    for (unsigned i = 0;i <= MaxLevel;i++) {
        while (!Queue[i].empty()) {
            gptr = Queue[i].front();
            Queue[i].pop_front();
            gptr->ResetFlag(SCHEDULED);
            ccsParallelEvaluate(gptr, flag);
        }
    }
    return;
}

//Evaluate parallel value of gptr
void CIRCUIT::ccsParallelEvaluate(GATEPTR gptr, bool flag)
{
    register unsigned i;
    bitset<PatternNum> new_value1(gptr->Fanin(0)->GetValue1());
    bitset<PatternNum> new_value2(gptr->Fanin(0)->GetValue2());

		if(flag == true){
		ofsEva << "G_" << gptr->GetName() << "[0]" << " = " 
					 << "G_" << gptr->Fanin(0)->GetName() << "[0];\n";
		ofsEva << "G_" << gptr->GetName() << "[1]" << " = " 
					 << "G_" << gptr->Fanin(0)->GetName() << "[1];\n";
		}

		evaluation_count += gptr->No_Fanin();
    switch(gptr->GetFunction()) {
        case G_AND:
        case G_NAND:
            for (i = 1; i < gptr->No_Fanin(); ++i) {
                new_value1 &= gptr->Fanin(i)->GetValue1();
								new_value2 &= gptr->Fanin(i)->GetValue2();

								if(flag == true){
								ofsEva << "G_" << gptr->GetName() << "[0]" << " &= " 
											 << "G_" << gptr->Fanin(i)->GetName() << "[0];\n";
								ofsEva << "G_" << gptr->GetName() << "[1]" << " &= " 
											 << "G_" << gptr->Fanin(i)->GetName() << "[1];\n";
								}
            }
            break;
        case G_OR:
        case G_NOR:
            for (i = 1; i < gptr->No_Fanin(); ++i) {
                new_value1 |= gptr->Fanin(i)->GetValue1();
                new_value2 |= gptr->Fanin(i)->GetValue2();

								if(flag == true){
								ofsEva << "G_" << gptr->GetName() << "[0]" << " |= " 
											 << "G_" << gptr->Fanin(i)->GetName() << "[0];\n";
								ofsEva << "G_" << gptr->GetName() << "[1]" << " |= " 
											 << "G_" << gptr->Fanin(i)->GetName() << "[1];\n";
								}
            }
            break;
        default: break;
    } 
    //swap new_value1 and new_value2 to avoid unknown value masked
    if (gptr->Is_Inversion()) {
        new_value1.flip(); new_value2.flip();
        bitset<PatternNum> value(new_value1);
				new_value1 = new_value2; new_value2 = value;

				if(flag == true){
					ofsEva << "temp = G_" << gptr->GetName() << "[0];\n";
					ofsEva << "G_" << gptr->GetName() << "[0]" << " = ~" 
								 << "G_" << gptr->GetName() << "[1];\n";
					ofsEva << "G_" << gptr->GetName() << "[1]" << " = ~temp;\n";
				}
    }
    if (gptr->GetValue1() != new_value1 || gptr->GetValue2() != new_value2) {
        gptr->SetValue1(new_value1);
        gptr->SetValue2(new_value2);
        ScheduleFanout(gptr);
    }
    return;
}

void CIRCUIT::genHeader()
{
	ofsHeader << "#include <iostream>" << endl;
	ofsHeader << "#include <ctime>" << endl;
	ofsHeader << "#include <bitset>" << endl;
	ofsHeader << "#include <string>" << endl;
	ofsHeader << "#include <fstream>" << endl;
	ofsHeader << "#include <stdlib.h>" << endl;
	ofsHeader << "using namespace std;" << endl << endl;
	ofsHeader << "const unsigned PatternNum = 16;" << endl << endl;
	ofsHeader << "void evaluate();" << endl;
	ofsHeader << "void printIO(unsigned idx);" << endl << endl;

	ofsHeader << "bitset<PatternNum> temp;\n";
	for(unsigned i=0; i < No_Gate(); i++) {
		ofsHeader << "bitset<PatternNum> G_" << Gate(i)->GetName() << "[2];\n";
	}

	ofsHeader << "ofstream ofs(\"" << output_name
						<< "\", ofstream::out | ofstream::trunc);\n";

	ofsHeader << endl;
}

void CIRCUIT::genMainBegin()
{
	ofsMain << "int main()\n{\n";
	ofsMain << "clock_t time_init, time_end;\n";
	ofsMain << "time_init = clock();\n";
}

void CIRCUIT::genMainEnd()
{
	ofsMain << "time_end = clock();\n";
	ofsMain << "cout << \"Total CPU Time = \" << double(time_end - \
							time_init)/CLOCKS_PER_SEC << endl;\n";
	ofsMain << "return 0;\n}\n";
}

void CIRCUIT::genEvaBegin()
{
	ofsEva << "void evaluate()\n{\n";
}

void CIRCUIT::genEvaEnd()
{
	ofsEva << "}\n";
}

void CIRCUIT::genIniPattern()
{
	vector<GATE*>::iterator it;

	for(it=Pattern.getInlistPtr()->begin(); it!=Pattern.getInlistPtr()->end(); it++) {
		ofsMain << "G_" << (*it)->GetName() << "[0] = 0b" << (*it)->getWireValue()[0] 
						<< ";" << endl;
		ofsMain << "G_" << (*it)->GetName() << "[1] = 0b" << (*it)->getWireValue()[1] 
						<< ";" << endl;
	}
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

	system("rm -f ./simulator/header");
	system("rm -f ./simulator/main");
	system("rm -f ./simulator/evaluate");
	system("rm -f ./simulator/printIO");
}

void CIRCUIT::genPrintIOBegin()
{
	ofsPrintIO << "void printIO(unsigned idx)\n{\n";
}

void CIRCUIT::genPrintIOEnd()
{
	ofsPrintIO << "}\n";
}

void CIRCUIT::ccsPrintParallelIOs(unsigned idx)
{
	ofsPrintIO << "for(unsigned i=0; i < idx; i++){\n";
	for(unsigned i=0; i < No_PI(); i++){
		ofsPrintIO << "\tif(G_" << PIGate(i)->GetName() << "[0][i] == 0){\n";

		ofsPrintIO << "\tif(G_" << PIGate(i)->GetName() << "[1][i] == 1)\n";
		ofsPrintIO << "\tofs << \'F\';\n";
		ofsPrintIO << "\telse\n\tofs << \'0\';\n";

		ofsPrintIO << "\t}\n\telse{\n";

		ofsPrintIO << "\tif(G_" << PIGate(i)->GetName() << "[1][i] == 1)\n";
		ofsPrintIO << "\tofs << \'1\';\n";
		ofsPrintIO << "\telse\n\tofs << \'2\';\n";

		ofsPrintIO << "\t}\n";
	}
	
	ofsPrintIO << "\tofs << \" \";\n";

	for(unsigned i=0; i < No_PO(); i++){
		ofsPrintIO << "\tif(G_" << POGate(i)->GetName() << "[0][i] == 0){\n";

		ofsPrintIO << "\tif(G_" << POGate(i)->GetName() << "[1][i] == 1)\n";
	ofsPrintIO << "\tofs << \'F\';\n";
	ofsPrintIO << "\telse\n\tofs << \'0\';\n";

		ofsPrintIO << "\t}\n\telse{\n";

		ofsPrintIO << "\tif(G_" << POGate(i)->GetName() << "[1][i] == 1)\n";
		ofsPrintIO << "\tofs << \'1\';\n";
		ofsPrintIO << "\telse\n\tofs << \'2\';\n";

		ofsPrintIO << "\t}\n";
	}
	ofsPrintIO << "\tofs << endl;\n";

	ofsPrintIO << "}\n";
}
