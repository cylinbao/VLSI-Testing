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
		else if (option.retrieve("logicsim")) {
        //logic simulator
        Circuit.InitPattern(option.retrieve("input"));
        Circuit.openOutputFile(option.retrieve("output"));
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
