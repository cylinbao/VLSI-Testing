#include <iostream>
#include <ctime>
#include <string>
#include "circuit.h"
#include "GetLongOpt.h"
#include "pattern.h"
using namespace std;

// All defined in readcircuit.l
extern char* yytext;
extern FILE *yyin;
extern CIRCUIT Circuit;
extern int yyparse (void);
extern bool ParseError;

extern void Interactive();

GetLongOpt option;

int SetupOption(int argc, char ** argv)
{
    option.usage("[options] input_circuit_file");
		// Add options for VLSI-Testing lab1
		option.enroll("print", GetLongOpt::NoValue,
						"print out the needed info of input file", 0);
		option.enroll("info", GetLongOpt::MandatoryValue,
						"specify the needed info.", 0);
		option.enroll("path", GetLongOpt::NoValue, 
						"print all the possible paths from source gate to target gate", 0);
		option.enroll("start",GetLongOpt::MandatoryValue,
						"get the starting gate for the path", 0);
		option.enroll("end",GetLongOpt::MandatoryValue,
						"get the ending gate for the path", 0);
		// --------------------
		// Add options for VLSI-Testing lab2
    option.enroll("pattern", GetLongOpt::NoValue,
            			"Generate random pattern", 0);
		option.enroll("num",GetLongOpt::MandatoryValue,
									"specify the number of the generated pattern", 0);
    option.enroll("unknown", GetLongOpt::NoValue,
            			"Generate random pattern with unknown", 0);
    option.enroll("mod_logicsim", GetLongOpt::NoValue,
            			"use cpu instructions to compute AND, OR and NOT", 0);
		// ---------------------------------
		// Add options for VLSI-Testing lab3
    option.enroll("simulator", GetLongOpt::MandatoryValue,
            "Generate a compiled code simulator", 0);
		// ---------------------------------
		// Add options for VLSI-Testing lab4
    option.enroll("check_point", GetLongOpt::NoValue,
            "Generate fault list by checkpoint theorem", 0);
    option.enroll("bridging", GetLongOpt::NoValue,
            "Generate bridging fault list", 0);
		// ---------------------------------
		// Add options for VLSI-Testing lab5
    option.enroll("bridging_fsim", GetLongOpt::NoValue,
            "run stuck-at bridging fault simulation", 0);
		// ---------------------------------
		// Add options for VLSI-Testing lab6
    option.enroll("check_point_fsim", GetLongOpt::NoValue,
            "run stuck-at check_point fault simulation", 0);
    option.enroll("random_pattern", GetLongOpt::NoValue,
            "Use random patterns first and then atpg", 0);
		// ---------------------------------
    option.enroll("help", GetLongOpt::NoValue,
            "print this help summary", 0);
    option.enroll("logicsim", GetLongOpt::NoValue,
            "run logic simulation", 0);
    option.enroll("plogicsim", GetLongOpt::NoValue,
            "run parallel logic simulation", 0);
    option.enroll("fsim", GetLongOpt::NoValue,
            "run stuck-at fault simulation", 0);
    option.enroll("stfsim", GetLongOpt::NoValue,
            "run single pattern single transition-fault simulation", 0);
    option.enroll("transition", GetLongOpt::NoValue,
            "run transition-fault ATPG", 0);
    option.enroll("input", GetLongOpt::MandatoryValue,
            "set the input pattern file", 0);
    option.enroll("output", GetLongOpt::MandatoryValue,
            "set the output pattern file", 0);
    option.enroll("bt", GetLongOpt::OptionalValue,
            "set the backtrack limit", 0);
    int optind = option.parse(argc, argv);
    if ( optind < 1 ) { exit(0); }
    if ( option.retrieve("help") ) {
        option.usage();
        exit(0);
    }
    return optind;
}

int main(int argc, char ** argv)
{
    int optind = SetupOption(argc, argv);
    clock_t time_init, time_end;
    time_init = clock();
    //Setup File
    if (optind < argc) {
        if ((yyin = fopen(argv[optind], "r")) == NULL) {
            cout << "Can't open circuit file: " << argv[optind] << endl;
            exit( -1);
        }
        else {
            string output_name = argv[optind];
            string::size_type idx = output_name.rfind('/');
            if (idx != string::npos) { output_name = output_name.substr(idx+1); }
            idx = output_name.find(".bench");
            if (idx != string::npos) { output_name = output_name.substr(0,idx); }
            Circuit.SetName(output_name);
        }
    }
    else {
        cout << "Input circuit file missing" << endl;
        option.usage();
        return -1;
    }
    cout << "Start parsing input file\n";
    yyparse();
    if (ParseError) {
        cerr << "Please correct error and try Again.\n";
        return -1;
    }
    fclose(yyin);
    Circuit.FanoutList();
    Circuit.SetupIO_ID();
    Circuit.Levelize();
    Circuit.Check_Levelization();
    Circuit.InitializeQueue();

		// Options operations for Lab1
		if(option.retrieve("path")){
			cout << "Circuit file name: " << Circuit.GetName() << endl;
			const char *start_gate = option.retrieve("start");
			const char *end_gate = option.retrieve("end");

			string src_gate_name(start_gate);
			string dest_gate_name(end_gate);
			Circuit.path(src_gate_name, dest_gate_name);
		}
		// ----------------------------
		// Options operations for lab2
		else if(option.retrieve("pattern")){
			string output_name = (string) option.retrieve("output");
			string pattern_name;
			int number = atoi(option.retrieve("num"));

			string::size_type idx = output_name.rfind('/');
			if (idx != string::npos) { output_name = output_name.substr(idx+1); }
			idx = output_name.find(".output");
			if (idx != string::npos) { pattern_name = output_name.substr(0,idx); }
			pattern_name.append(".input");

			if(option.retrieve("unknown"))	
				Circuit.genRandomPatternUnknown(pattern_name, number);
			else
				Circuit.genRandomPattern(pattern_name, number);
			
			Circuit.openOutputFile(output_name);
			Circuit.LogicSimVectors();
		}
		else if(option.retrieve("mod_logicsim")){
        //logic simulator using CPU instructions directly
        Circuit.InitPattern(option.retrieve("input"));
        Circuit.ModLogicSimVectors();
		}
		// ----------------------------
		else if(option.retrieve("print")){
			const char *info_type = option.retrieve("info");
			cout << "Type of Info.: " << info_type << endl;
			if((string)info_type == "net")
				Circuit.printNetlist();
			else if((string)info_type == "PO")
				Circuit.printPOInputList();
			else if((string)info_type == "gate")
				Circuit.printGateOutput();
		}
		// ---------------------------
		// Options operations for lab3
		else if(option.retrieve("simulator")){
				string file_name = (string) option.retrieve("simulator");
				string output_name;

				string::size_type idx;
				idx = file_name.find(".cc");
				if (idx != string::npos) { output_name = file_name.substr(0,idx); }
				output_name.append(".out");
				cout << "Output name: " << output_name << endl;
				Circuit.setOutputName(output_name);

        Circuit.InitPattern(option.retrieve("input"));
				Circuit.openSimulatorFile(file_name);
        Circuit.genCompiledCodeSimulator();
		}
		// ---------------------------
		// Options operations for lab4
    else if (option.retrieve("check_point")) {
        //single pattern single transition-fault simulation
        Circuit.GenerateAllCPFaultList();
        Circuit.GenerateAllFaultList();
				Circuit.CalculatePercentage();
    }
    else if (option.retrieve("bridging")) {
				string output_name = (string) option.retrieve("output");

				Circuit.PutGateIntoQueueByLevel();
        Circuit.GenerateAllBFaultList();
				Circuit.openOutputFile(output_name);
				Circuit.OutputAllBFaultList();
    }
		// ---------------------------
		// Options operations for lab6
    else if (option.retrieve("check_point_fsim")) {
        //single pattern single transition-fault simulation
        Circuit.GenerateAllCPFaultListForFsim();
				
        //Circuit.GenerateAllFaultList();
        Circuit.SortFaninByLevel();
        Circuit.MarkOutputGate();
        if (option.retrieve("fsim")) {
            //stuck-at fault simulator
            Circuit.InitPattern(option.retrieve("input"));
            Circuit.FaultSimVectors();
        }
        else {
            if (option.retrieve("bt")) {
                Circuit.SetBackTrackLimit(atoi(option.retrieve("bt")));
            }
            //stuck-at fualt ATPG
            Circuit.Atpg();
        }
    }
		else if (option.retrieve("random_pattern")) {
        Circuit.GenerateAllFaultList();
        Circuit.SortFaninByLevel();
        Circuit.MarkOutputGate();
				string pattern_name = (string) option.retrieve("output");
				cout << "pattern name: " << pattern_name << endl;
				Circuit.openFile(pattern_name);
				Circuit.genRandomPattern(pattern_name, 1000);

				unsigned coverage = 0;
				int cnt = 0;

				while (coverage <= 90 && cnt < 1000) {
           //stuck-at fault simulator
           coverage = Circuit.FaultSimRandomPattern();
						
					 cout << "Random Pattern #" << ++cnt << " Coverage: " 
					 			<< coverage << "%\n";
        }
				if (option.retrieve("bt")) {
				Circuit.SetBackTrackLimit(atoi(option.retrieve("bt")));
				}
				//stuck-at fualt ATPG after Random Pattern
				Circuit.AtpgRandomPattern();
		}
		// ---------------------------
		else if (option.retrieve("logicsim")) {
        //logic simulator
        Circuit.InitPattern(option.retrieve("input"));
        Circuit.LogicSimVectors();
    }
    else if (option.retrieve("plogicsim")) {
        //parallel logic simulator
        Circuit.InitPattern(option.retrieve("input"));
        Circuit.ParallelLogicSimVectors();
    }
    else if (option.retrieve("stfsim")) {
        //single pattern single transition-fault simulation
        Circuit.MarkOutputGate();
        Circuit.GenerateAllTFaultList();
        Circuit.InitPattern(option.retrieve("input"));
        Circuit.TFaultSimVectors();
    }
    else if (option.retrieve("transition")) {
        Circuit.MarkOutputGate();
        Circuit.GenerateAllTFaultList();
        Circuit.SortFaninByLevel();
        if (option.retrieve("bt")) {
            Circuit.SetBackTrackLimit(atoi(option.retrieve("bt")));
        }
        Circuit.TFAtpg();
    }
		else if (option.retrieve("bridging_fsim")) {
			Circuit.PutGateIntoQueueByLevel();
			Circuit.GenerateAllBFaultList();
      Circuit.SortFaninByLevel();
      Circuit.MarkOutputGate();

			//stuck-at bridging fault simulator
			Circuit.InitPattern(option.retrieve("input"));
			Circuit.BFaultSimVectors();
		}
    else {
        Circuit.GenerateAllFaultList();
        Circuit.SortFaninByLevel();
        Circuit.MarkOutputGate();
        if (option.retrieve("fsim")) {
            //stuck-at fault simulator
            Circuit.InitPattern(option.retrieve("input"));
            Circuit.FaultSimVectors();
        }
        else {
            if (option.retrieve("bt")) {
                Circuit.SetBackTrackLimit(atoi(option.retrieve("bt")));
            }
            //stuck-at fualt ATPG
            Circuit.Atpg();
        }
    }

    time_end = clock();
    cout << "total CPU time = " << double(time_end - time_init)/CLOCKS_PER_SEC << endl;
    cout << endl;

   return 0;
}
