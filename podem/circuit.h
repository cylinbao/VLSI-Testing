#ifndef CIRCUIT_H
#define CIRCUIT_H
#include "fault.h"
#include "tfault.h"
#include "pattern.h"
#include <stdlib.h>
#include <string>
#include <vector>
#include <fstream>
#include <string.h>
#include <bitset>
#include <iostream>

#define xstr(s) str(s)
#define str(s) #s
#define OUTDIR output
#define SIMDIR simulator

typedef GATE* GATEPTR;

class CIRCUIT
{
	private:
		string Name;
		PATTERN Pattern;
		vector<GATE*> Netlist;
		vector<GATE*> PIlist; //store the gate indexes of PI
		vector<GATE*> POlist;
		vector<GATE*> PPIlist;
		vector<GATE*> PPOlist;
		list<FAULT*> Flist; //collapsing fault list
		list<FAULT*> UFlist; //undetected fault list
		list<TFAULT*> TFlist; //collapsing fault list
		list<TFAULT*> UTFlist; //undetected fault list
		list<FAULT*> CPFlist; //collapsing fault list
		list<FAULT*> UCPFlist; //undetected fault list
		list<FAULT*> BFlist; //collapsing fault list
		list<FAULT*> UBFlist; //undetected fault list
		unsigned MaxLevel;
		unsigned BackTrackLimit; //backtrack limit for Podem
		typedef list<GATE*> ListofGate;
		typedef list<GATE*>::iterator ListofGateIte;
		ListofGate* Queue;
		ListofGate GateStack;
		ListofGate PropagateTree;
		ListofGateIte QueueIte;
		//VLSI-Testing Lab1, stack for path
		std::vector<GATE*> path_stack;
		int path_count;
		string dest_gate_name;
		string input_name, output_name;
		ofstream ofs; // for printing logicsim output file
		//VLSI-Testing Lab3
		unsigned int evaluation_count;
		double avg_eval_cnt_pattern;
		double percent_eval_cnt;
		unsigned pattern_num;
		ofstream ofsHeader, ofsMain, ofsEva, ofsPrintIO;

	public:
		//Initialize netlist
		CIRCUIT(): MaxLevel(0), BackTrackLimit(10000) {
			Netlist.reserve(32768);
			PIlist.reserve(128);
			POlist.reserve(512);
			PPIlist.reserve(2048);
			PPOlist.reserve(2048);
			path_stack.clear();
			path_count = 0;
			evaluation_count = 0;
			pattern_num = 0;
		}
		CIRCUIT(unsigned NO_GATE, unsigned NO_PI = 128, unsigned NO_PO = 512,
				unsigned NO_PPI = 2048, unsigned NO_PPO = 2048) {
			Netlist.reserve(NO_GATE);
			PIlist.reserve(NO_PI);
			POlist.reserve(NO_PO);
			PPIlist.reserve(NO_PPI);
			PPOlist.reserve(NO_PPO);
			path_stack.clear();
			path_count = 0;
			evaluation_count = 0;
			pattern_num = 0;
		}
		~CIRCUIT() {
			for (unsigned i = 0;i<Netlist.size();++i) { delete Netlist[i]; }
			list<FAULT*>::iterator fite;
			for (fite = Flist.begin();fite!=Flist.end();++fite) { delete *fite; }
			if(ofs.is_open())
				ofs.close();
			if(ofsHeader.is_open())
				ofsHeader.close();
			if(ofsMain.is_open())
				ofsMain.close();
			if(ofsEva.is_open())
				ofsEva.close();
			if(ofsPrintIO.is_open())
				ofsPrintIO.close();
		}

		void AddGate(GATE* gptr) { Netlist.push_back(gptr); }
		void SetName(string n){ Name = n;}
		string GetName(){ return Name;}
		int GetMaxLevel(){ return MaxLevel;}
		void SetBackTrackLimit(unsigned bt) { BackTrackLimit = bt; }

		//Access the gates by indexes
		GATE* Gate(unsigned index) { return Netlist[index]; }
		GATE* PIGate(unsigned index) { return PIlist[index]; }
		GATE* POGate(unsigned index) { return POlist[index]; }
		GATE* PPIGate(unsigned index) { return PPIlist[index]; }
		GATE* PPOGate(unsigned index) { return PPOlist[index]; }
		unsigned No_Gate() { return Netlist.size(); }
		unsigned No_PI() { return PIlist.size(); }
		unsigned No_PO() { return POlist.size(); }
		unsigned No_PPI() { return PPIlist.size(); }
		unsigned No_PPO() { return PPOlist.size(); }

		void copyPItoPattern(){
			vector<GATE*>::iterator it; 
			for(it = PIlist.begin(); it != PIlist.end(); it++){
				Pattern.addInList(*it);
			}
		}
		void genRandomPattern(string pattern_name, int number){
			Pattern.setPatternName(pattern_name);
			copyPItoPattern();
			Pattern.genRandomPattern(number);
			Pattern.setPatterninput();
		}
		void genRandomPatternUnknown(string pattern_name, int number){
			Pattern.setPatternName(pattern_name);
			copyPItoPattern();
			Pattern.genRandomPatternUnknown(number);
			Pattern.setPatterninput();
		}

		void InitPattern(const char *pattern) {
			Pattern.Initialize(const_cast<char *>(pattern), PIlist.size(), "PI");
		}

		void openOutputFile(string file_name) {
			char str[] = "mkdir ";
			strcat(str, xstr(OUTDIR));
			system(str);

			strcpy(str, "./"); 
			strcat(str, xstr(OUTDIR));
			strcat(str, "/");
			strcat(str, file_name.c_str());
			ofs.open(str, ofstream::out | ofstream::trunc);
			if(!ofs.is_open())
				cout << "Cannot open file!\n";
		}

		void openSimulatorFile(string file_name) {
			char str[] = "mkdir ";
			strcat(str, xstr(SIMDIR));
			system(str);

			strcpy(str, "./"); 
			strcat(str, xstr(SIMDIR));
			strcat(str, "/");
			strcat(str, file_name.c_str());
			ofs.open(str, ofstream::out | ofstream::trunc);
			ofsHeader.open("./simulator/header", ofstream::out|ofstream::trunc);
			ofsMain.open("./simulator/main", ofstream::out|ofstream::trunc);
			ofsEva.open("./simulator/evaluate", ofstream::out|ofstream::trunc);
			ofsPrintIO.open("./simulator/printIO", ofstream::out|ofstream::trunc);
			if(!ofs.is_open())
				cout << "Cannot open output file!\n";
			if(!ofsHeader.is_open())
				cout << "Cannot open header!\n";
			if(!ofsMain.is_open())
				cout << "Cannot open main!\n";
			if(!ofsEva.is_open())
				cout << "Cannot open evaluate!\n";
			if(!ofsPrintIO.is_open())
				cout << "Cannot open printIO!\n";
		}

		void Schedule(GATE* gptr)
		{
			if (!gptr->GetFlag(SCHEDULED)) {
				gptr->SetFlag(SCHEDULED);
				Queue[gptr->GetLevel()].push_back(gptr);
			}
		}

		// VLSI-Testing Lab1
		// defined in path.cc
		void path(string src_name_gate, string dest_gate_name);
		bool findPath();
		void printPath();
		// VLSI-Testing Lab3
		unsigned int getEvaluationCount() {return evaluation_count;}
		void printStatResult();
		void genCompiledCodeSimulator();
		void ccsParallelLogicSim(bool flag);
		void ccsParallelEvaluate(GATEPTR gptr, bool flag);
		void ccsPrintParallelIOs(unsigned idx);
		void genHeader();
		void genMainBegin();
		void genMainEnd();
		void genEvaBegin();
		void genEvaEnd();
		void genPrintIOBegin();
		void genPrintIOEnd();
		void genIniPattern();
		void combineFilesToOutput();
		void setOutputName(string str) { output_name = str;}
		// VLSI-Testing Lab4
		void GenerateAllCPFaultList();
		void GenerateAllBFaultList();
		void CalculatePercentage();
			
		//defined in circuit.cc
		void Levelize();
		void FanoutList();
		void Check_Levelization();
		void SetMaxLevel();
		void SetupIO_ID();
		// print useful infomation
		void printNetlist();
		void printPOInputList();
		void printGateOutput();

		//defined in sim.cc
		void SetPPIZero(); //Initialize PPI state
		void InitializeQueue();
		void ScheduleFanout(GATE*);
		void SchedulePI();
		void SchedulePPI();
		void LogicSimVectors();
		void LogicSim();
		void ModLogicSimVectors();
		void ModLogicSim();
		void PrintIO();
		void PrintModIO();
		VALUE Evaluate(GATEPTR gptr);
		bitset<2> ModEvaluate(GATEPTR gptr);

		//defined in atpg.cc
		void GenerateAllFaultList();
		void GenerateCheckPointFaultList();
		void GenerateFaultList();
		void Atpg();
		void SortFaninByLevel();
		bool CheckTest();
		bool TraceUnknownPath(GATEPTR gptr);
		bool FaultEvaluate(FAULT* fptr);
		ATPG_STATUS Podem(FAULT* fptr, unsigned &total_backtrack_num);
		ATPG_STATUS SetUniqueImpliedValue(FAULT* fptr);
		ATPG_STATUS BackwardImply(GATEPTR gptr, VALUE value);
		GATEPTR FindPropagateGate();
		GATEPTR FindHardestControl(GATEPTR gptr);
		GATEPTR FindEasiestControl(GATEPTR gptr);
		GATEPTR FindPIAssignment(GATEPTR gptr, VALUE value);
		GATEPTR TestPossible(FAULT* fptr);
		void TraceDetectedStemFault(GATEPTR gptr, VALUE val);

		//defined in fsim.cc
		void MarkOutputGate();
		void MarkPropagateTree(GATEPTR gptr);
		void FaultSimVectors();
		void FaultSim();
		void FaultSimEvaluate(GATE* gptr);
		bool CheckFaultyGate(FAULT* fptr);
		void InjectFaultValue(GATEPTR gptr, unsigned idx,VALUE value);

		//defined in psim.cc for parallel logic simulation
		void ParallelLogicSimVectors();
		void ParallelLogicSim();
		void ParallelEvaluate(GATEPTR gptr);
		void PrintParallelIOs(unsigned idx);
		void ScheduleAllPIs();

		//defined in stfsim.cc for single pattern single transition-fault simulation
		void GenerateAllTFaultList();
		void TFaultSimVectors();
		void TFaultSim_t();
		void TFaultSim();
		bool CheckTFaultyGate(TFAULT* fptr);
		bool CheckTFaultyGate_t(TFAULT* fptr);
		VALUE Evaluate_t(GATEPTR gptr);
		void LogicSim_t();
		void PrintTransition();
		void PrintTransition_t();
		void PrintIO_t();

		//defined in tfatpg.cc for transition fault ATPG
		void TFAtpg();
		ATPG_STATUS Initialization(GATEPTR gptr, VALUE target, unsigned &total_backtrack_num);
		ATPG_STATUS BackwardImply_t(GATEPTR gptr, VALUE value);
		GATEPTR FindPIAssignment_t(GATEPTR gptr, VALUE value);
		GATEPTR FindEasiestControl_t(GATEPTR gptr);
		GATEPTR FindHardestControl_t(GATEPTR gptr);
};
#endif
