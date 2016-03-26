/* Logic Simulator
 * Last update: 2006/09/20 */
#include <iostream>
#include <bitset>
#include "gate.h"
#include "circuit.h"
#include "pattern.h"
#include "GetLongOpt.h"
using namespace std;

extern GetLongOpt option;

//do logic simulation for test patterns
void CIRCUIT::LogicSimVectors()
{
    cout << "Run logic simulation" << endl;
    //read test patterns
    while (!Pattern.eof()) {
        Pattern.ReadNextPattern();
        SchedulePI();
        LogicSim();
        PrintIO();
    }
    return;
}

//do logic simulation for test patterns
void CIRCUIT::ModLogicSimVectors()
{
    cout << "Run logic simulation" << endl;
    //read test patterns
    while (!Pattern.eof()) {
        Pattern.ReadNextModPattern();
        SchedulePI();
        ModLogicSim();
        PrintModIO();
    }
    return;
}

//do event-driven logic simulation
void CIRCUIT::LogicSim()
{
    GATE* gptr;
    VALUE new_value;
    for (unsigned i = 0;i <= MaxLevel;i++) {
        while (!Queue[i].empty()) {
            gptr = Queue[i].front();
            Queue[i].pop_front();
            gptr->ResetFlag(SCHEDULED);
            new_value = Evaluate(gptr);
            if (new_value != gptr->GetValue()) {
                gptr->SetValue(new_value);
                ScheduleFanout(gptr);
            }
        }
    }
    return;
}

//do event-driven logic simulation
void CIRCUIT::ModLogicSim()
{
    GATE* gptr;
    bitset<2> new_value;
    for (unsigned i = 0;i <= MaxLevel;i++) {
        while (!Queue[i].empty()) {
            gptr = Queue[i].front();
            Queue[i].pop_front();
            gptr->ResetFlag(SCHEDULED);
            new_value = ModEvaluate(gptr);
            if (new_value != gptr->GetValue()) {
                gptr->SetModValue(new_value);
                ScheduleFanout(gptr);
            }
        }
    }
    return;
}

//Used only in the first pattern
void CIRCUIT::SchedulePI()
{
    for (unsigned i = 0;i < No_PI();i++) {
        if (PIGate(i)->GetFlag(SCHEDULED)) {
            PIGate(i)->ResetFlag(SCHEDULED);
            ScheduleFanout(PIGate(i));
        }
    }
    return;
}

//schedule all fanouts of PPIs to Queue
void CIRCUIT::SchedulePPI()
{
#include <bitset>
    for (unsigned i = 0;i < No_PPI();i++) {
        if (PPIGate(i)->GetFlag(SCHEDULED)) {
            PPIGate(i)->ResetFlag(SCHEDULED);
            ScheduleFanout(PPIGate(i));
        }
    }
    return;
}

//set all PPI as 0
void CIRCUIT::SetPPIZero()
{
#include <bitset>
    GATE* gptr;
    for (unsigned i = 0;i < No_PPI();i++) {
        gptr = PPIGate(i);
        if (gptr->GetValue() != S0) {
            gptr->SetFlag(SCHEDULED);
            gptr->SetValue(S0);
        }
    }
    return;
}

//schedule all fanouts of gate to Queue
void CIRCUIT::ScheduleFanout(GATE* gptr)
{
    for (unsigned j = 0;j < gptr->No_Fanout();j++) {
        Schedule(gptr->Fanout(j));
    }
    return;
}

//initial Queue for logic simulation
void CIRCUIT::InitializeQueue()
{
    SetMaxLevel();
    Queue = new ListofGate[MaxLevel + 1];
    return;
}

//evaluate the output value of gate
VALUE CIRCUIT::Evaluate(GATEPTR gptr)
{
    GATEFUNC fun(gptr->GetFunction());
    VALUE cv(CV[fun]); //controling value
    VALUE value(gptr->Fanin(0)->GetValue());
    switch (fun) {
        case G_AND:
        case G_NAND:
            for (unsigned i = 1;i<gptr->No_Fanin() && value != cv;++i) {
                value = AndTable[value][gptr->Fanin(i)->GetValue()];
            }
            break;
        case G_OR:
        case G_NOR:
            for (unsigned i = 1;i<gptr->No_Fanin() && value != cv;++i) {
                value = OrTable[value][gptr->Fanin(i)->GetValue()];
            }
            break;
        default: break;
    }
    //NAND, NOR and NOT
    if (gptr->Is_Inversion()) { value = NotTable[value]; }

    return value;
}

// Using CPU instructions directly to evaluate the output value of gate
bitset<2> CIRCUIT::ModEvaluate(GATEPTR gptr)
{
    GATEFUNC fun(gptr->GetFunction());
    VALUE cv(CV[fun]); //controling value
    bitset<2> value(gptr->Fanin(0)->GetModValue());
    switch (fun) {
        case G_AND:
        case G_NAND:
            for (unsigned i = 1;i<gptr->No_Fanin() && (VALUE) value.to_ulong() 
								!= cv;++i){
    						value = value & gptr->Fanin(i)->GetModValue();
						}
            break;
        case G_OR:
        case G_NOR:
            for (unsigned i = 1;i<gptr->No_Fanin() && (VALUE) value.to_ulong() 
								!= cv;++i){
    						value = value | gptr->Fanin(i)->GetModValue();
						}
            break;
        default: break;
    }
    //NAND, NOR and NOT
    if (gptr->Is_Inversion()){ 
			if(value.to_string() != "01")
				value = ~value;
		}
		
    return value;
}

extern GATE* NameToGate(string);

void PATTERN::Initialize(char* InFileName, int no_pi, string TAG)
{
    patterninput.open(InFileName, ios::in);
    if (!patterninput) {
        cout << "Unable to open input pattern file: " << InFileName << endl;
        exit( -1);
    }
    string piname;
    while (no_pi_infile < no_pi) {
        patterninput >> piname;
        if (piname == TAG) {
            patterninput >> piname;
            inlist.push_back(NameToGate(piname));
            no_pi_infile++;
        }
        else {
            cout << "Error in input pattern file at line "
                << no_pi_infile << endl;
            cout << "Maybe insufficient number of input\n";
            exit( -1);
        }
    }
    return;
}

//Assign next input pattern to PI
void PATTERN::ReadNextPattern()
{
    char V;
    for (int i = 0;i < no_pi_infile;i++) {
        patterninput >> V;
        if (V == '0') {
            if (inlist[i]->GetValue() != S0) {
                inlist[i]->SetFlag(SCHEDULED);
                inlist[i]->SetValue(S0);
            }
        }
        else if (V == '1') {
            if (inlist[i]->GetValue() != S1) {
                inlist[i]->SetFlag(SCHEDULED);
                inlist[i]->SetValue(S1);
            }
        }
        else if (V == 'X') {
            if (inlist[i]->GetValue() != X) {
                inlist[i]->SetFlag(SCHEDULED);
                inlist[i]->SetValue(X);
            }
        }
    }
    //Take care of newline to force eof() function correctly
    patterninput >> V;
    if (!patterninput.eof()) patterninput.unget();

    return;
}

void PATTERN::ReadNextModPattern()
{
    char V;
    for (int i = 0;i < no_pi_infile;i++) {
        patterninput >> V;
        if (V == '0') {
            if (inlist[i]->GetModValue().to_string() != "00") {
                inlist[i]->SetFlag(SCHEDULED);
								bitset<2> bits(0x0);
                inlist[i]->SetModValue(bits);
            }
        }
        else if (V == '1') {
            if (inlist[i]->GetModValue().to_string() != "11") {
                inlist[i]->SetFlag(SCHEDULED);
								bitset<2> bits(0x3);
                inlist[i]->SetModValue(bits);
            }
        }
        else if (V == 'X') {
            if (inlist[i]->GetModValue().to_string() != "01") {
                inlist[i]->SetFlag(SCHEDULED);
								bitset<2> bits(0x1);
                inlist[i]->SetModValue(bits);
            }
        }
    }
    //Take care of newline to force eof() function correctly
    patterninput >> V;
    if (!patterninput.eof()) patterninput.unget();

    return;
}

void CIRCUIT::PrintIO()
{
    register unsigned i;
		VALUE temp;
		cout << "PI: ";
		ofs << "PI: ";
    for (i = 0;i<No_PI();++i){
			temp = PIGate(i)->GetValue();
			if(temp == X){
				cout << 'X';
				ofs << 'X';
			}
			else{
				cout << temp; 
				ofs << temp;
			}
		}
    cout << " ";
    ofs << " ";
		cout << "PO: ";
		ofs << "PO: ";
    for (i = 0;i<No_PO();++i){
			temp = POGate(i)->GetValue();
			if(temp == X){
				cout << 'X';
				ofs << 'X';
			}
			else{
				cout << temp; 
				ofs << temp;
			}
		}
    cout << endl;
    ofs << endl;
    return;
}

void CIRCUIT::PrintModIO()
{
    register unsigned i;
		bitset<2> temp;
		cout << "PI: ";
    for (i = 0;i<No_PI();++i){
			temp = PIGate(i)->GetModValue();
			if(temp.to_string() == "00")
				cout << '0';
			else if(temp.to_string() == "11")
				cout << '1';
			else
				cout << 'X'; 
		}
    cout << " ";
		cout << "PO: ";
    for (i = 0;i<No_PO();++i){
			temp = POGate(i)->GetModValue();
			if(temp.to_string() == "00")
				cout << '0';
			else if(temp.to_string() == "11")
				cout << '1';
			else
				cout << 'X'; 
		}
    cout << endl;
    return;
}
