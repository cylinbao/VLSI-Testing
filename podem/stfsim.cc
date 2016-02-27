/* sequential transition fault simulator for combinational circuit
 * Last update: 2007/08/19 */
#include <iostream>
#include "circuit.h"
#include "GetLongOpt.h"
using namespace std;

extern GetLongOpt option;

//generate a list for all transition faults 
void CIRCUIT::GenerateAllTFaultList()
{
    cout << "Generate transition fault list" << endl;
    register unsigned i, j;
    GATEFUNC fun;
    GATEPTR gptr, fanout;
    TFAULT *fptr;
    for (i = 0;i<No_Gate();++i) {
        gptr = Netlist[i]; fun = gptr->GetFunction();
        if (fun == G_PO) { continue; } //skip PO
        //add stem stuck-at 0 fault to TFlist
        fptr = new TFAULT(gptr, gptr, S0);
        TFlist.push_front(fptr);
        //add stem stuck-at 1 fault to TFlist
        fptr = new TFAULT(gptr, gptr, S1);
        TFlist.push_front(fptr);

        if (gptr->No_Fanout() == 1) { continue; } //no branch faults

        //add branch fault
        for (j = 0;j< gptr->No_Fanout();++j) {
            fanout = gptr->Fanout(j);
            fptr = new TFAULT(gptr, fanout, S0);
            fptr->SetBranch(true);
            TFlist.push_front(fptr);
            fptr = new TFAULT(gptr, fanout, S1);
            fptr->SetBranch(true);
            TFlist.push_front(fptr);
        } //end all fanouts
    } //end all gates
    //copy TFlist to undetected UTFlist (for fault simulation)
    UTFlist = TFlist;
    return;
}

// fault simulation test patterns
// Note all functions with _t are the same as the one without _t except
// they perform simulation on Value_t on odd vectors
void CIRCUIT::TFaultSimVectors()
{
    cout << "Run transition fault simulation" << endl;
    unsigned pattern_num(0);
    if(!Pattern.eof()){ // Readin the first vector
            ++pattern_num;
            Pattern.ReadNextPattern();
            //fault-free simulation
            SchedulePI();
            LogicSim();
#ifdef DEBUG_TFSIM
	    cout << "Pattern #1"<< endl;
	    PrintIO();
#endif
    }
    if(!Pattern.eof()){ // For the following vectors, perform fault simulation
        while(!Pattern.eof()){
            ++pattern_num;
	    if(pattern_num%2==1){
		    Pattern.ReadNextPattern();
		    //fault-free simulation
		    SchedulePI();
		    LogicSim();
#ifdef DEBUG_TFSIM
	            cout << "Pattern #" << pattern_num<< endl; //debug
	            PrintIO(); //debug
		    PrintTransition(); //debug
#endif
		    //single pattern single fault simulation
		    TFaultSim();
	    }
	    else{
		    Pattern.ReadNextPattern_t();
		    //fault-free simulation
		    SchedulePI();
		    LogicSim_t();
#ifdef DEBUG_TFSIM
	            cout << "Pattern #" << pattern_num<< endl; //debug
	            PrintIO_t(); //debug
		    PrintTransition_t(); //debug
#endif
		    //single pattern single fault simulation
		    TFaultSim_t();
	    }
        }
    }

    //compute fault coverage
    unsigned total_num(0);
    unsigned undetected_num(0), detected_num(0);
    unsigned eqv_undetected_num(0), eqv_detected_num(0);
    TFAULT* fptr;
    list<TFAULT*>::iterator fite;
    for (fite = TFlist.begin();fite!=TFlist.end();++fite) {
        fptr = *fite;
        switch (fptr->GetStatus()) {
            case DETECTED:
                ++eqv_detected_num;
                detected_num += fptr->GetEqvFaultNum();
                break;
            default:
                ++eqv_undetected_num;
                undetected_num += fptr->GetEqvFaultNum();
                break;
        }
    }
    //Note that we do not perform any fault collapsing for transition faults
    total_num = detected_num + undetected_num;
    cout.setf(ios::fixed);
    cout.precision(2);
    cout << "---------------------------------------" << endl;
    cout << "Test pattern number = " << pattern_num << endl;
    cout << "---------------------------------------" << endl;
    cout << "Total fault number = " << total_num << endl;
    cout << "Detected fault number = " << detected_num << endl;
    cout << "Undetected fault number = " << undetected_num << endl;
    cout << "---------------------------------------" << endl;
    //cout << "Equivalent fault number = " << Flist.size() << endl;
    //cout << "Equivalent detected fault number = " << eqv_detected_num << endl; 
    //cout << "Equivalent undetected fault number = " << eqv_undetected_num << endl; 
    //cout << "---------------------------------------" << endl;
    cout << "Fault Coverge = " << 100*detected_num/double(total_num) << "%" << endl;
    //cout << "Equivalent FC = " << 100*eqv_detected_num/double(Flist.size()) << "%" << endl;
    cout << "---------------------------------------" << endl;
    return;
}

//Assign next input pattern to PI with Value_t
void PATTERN::ReadNextPattern_t()
{
    char V;
    for (int i = 0;i < no_pi_infile;i++) {
        patterninput >> V;
        if (V == '0') {
            if (inlist[i]->GetValue_t() != S0) {
                inlist[i]->SetFlag(SCHEDULED);
                inlist[i]->SetValue_t(S0);
            }
        }
        else if (V == '1') {
            if (inlist[i]->GetValue_t() != S1) {
                inlist[i]->SetFlag(SCHEDULED);
                inlist[i]->SetValue_t(S1);
            }
        }
        else if (V == 'X') {
            if (inlist[i]->GetValue_t() != X) {
                inlist[i]->SetFlag(SCHEDULED);
                inlist[i]->SetValue_t(X);
            }
        }
    }
    //Take care of newline to force eof() function correctly
    patterninput >> V;
    if (!patterninput.eof()) patterninput.unget();
    return;
}
//
//single pattern single fault simulation
//Assume current logic values of gates stored at Value_t
//Assume previous logic values of gates stored at Value
void CIRCUIT::TFaultSim_t()
{
    register unsigned i;
    GATEPTR gptr;
    TFAULT *fptr;
    
    //for all undetected faults
    list<TFAULT*>::iterator fite;
    for (fite = UTFlist.begin();fite!=UTFlist.end();++fite) {
        fptr = *fite;
#ifdef DEBUG_TFSIM
	cout << "Checking: " << fptr->GetInputGate()->GetName() << "->" << fptr->GetOutputGate()->GetName() << ":" << fptr->GetValue() << endl;
	//debug
#endif
        //skip redundant and detected faults
        if (fptr->GetStatus() == REDUNDANT || fptr->GetStatus() == DETECTED) { continue; }
        //the fault is not active
	gptr=fptr->GetInputGate();//Input gate
        if (fptr->GetValue() == S0) { //Slow to fall; need a falling transition
            if (gptr->GetValue()==S0||gptr->GetValue_t()==S1) { 
		continue; 
	    }
	}
	else{//Slow to rise
            if (gptr->GetValue()==S1||gptr->GetValue_t()==S0) { 
		continue; 
	    }
	}
#ifdef DEBUG_TFSIM
	//debug
	cout << "Activated" << endl;
#endif
        //the fault can be directly seen at PO
        if (gptr->GetFlag(OUTPUT) && (!fptr->Is_Branch() || fptr->GetOutputGate()->GetFunction() == G_PO)) {
            fptr->SetStatus(DETECTED);
#ifdef DEBUG_TFSIM
		cout << "DETECTED 1: " << fptr->GetInputGate()->GetName() << "->" << fptr->GetOutputGate()->GetName() << ":" << fptr->GetValue() << endl;
		//debug
#endif
            continue;
        }
	if (!fptr->Is_Branch()) { //stem
	    gptr->SetValue_t(NotTable[fptr->GetValue()]); //inject fault
            ScheduleFanout(gptr);
        }
        else { //branch; Only inject faults at gate outputs, 
	       //so we need to propagate branch faults one level further
            if (!CheckTFaultyGate_t(fptr)) { continue; }
            gptr = fptr->GetOutputGate();
            //if the fault is shown at an output, it is detected
            if (gptr->GetFlag(OUTPUT)) {
                fptr->SetStatus(DETECTED);
#ifdef DEBUG_TFSIM
		cout << "DETECTED 2: " << fptr->GetInputGate()->GetName() << "->" << fptr->GetOutputGate()->GetName() << ":" << fptr->GetValue() << endl;
		//debug
#endif
                continue;
            }
            // add the fault to the simulated list and inject it
            VALUE fault_type = gptr->Is_Inversion()? fptr->GetValue(): NotTable[fptr->GetValue()];
	    gptr->SetValue_t(fault_type);//inject fault
            ScheduleFanout(gptr);
        }
	//Perform simulation
	VALUE new_value;
	bool detected(0);
        for (i = 0;i<= MaxLevel&&!detected; ++i) {
            while (!Queue[i].empty()) {
                gptr = Queue[i].front(); 
		Queue[i].pop_front();
                gptr->ResetFlag(SCHEDULED);
		new_value = Evaluate_t(gptr);
		if (new_value != gptr->GetValue_t()) {
		    gptr->SetValue_t(new_value);
		    ScheduleFanout(gptr);
		    if(gptr->GetFlag(OUTPUT)){
			fptr->SetStatus(DETECTED);
			detected=1;
#ifdef DEBUG_TFSIM
			cout << "DETECTED 3: " << fptr->GetInputGate()->GetName() << "->" << fptr->GetOutputGate()->GetName() << ":" << fptr->GetValue() << endl;
			//debug
#endif
			break; //Jump out of the for loop
		    }
		}
            }
        }
        for (i = 0;i<= MaxLevel; ++i) {
            while (!Queue[i].empty()) {
                //clean up the queues
                gptr = Queue[i].front(); 
		Queue[i].pop_front();
                gptr->ResetFlag(SCHEDULED);
	    }
	}

	//Simulate inverse of the faulty value to restore fault-free values from faulty sites
	if (!fptr->Is_Branch()) { //stem
            gptr = fptr->GetOutputGate();
	    gptr->SetValue_t(fptr->GetValue()); //remove fault
            ScheduleFanout(gptr);
        }
        else { //branch
            gptr = fptr->GetOutputGate();
            VALUE nofault_type = gptr->Is_Inversion()? NotTable[fptr->GetValue()] : fptr->GetValue();
	    gptr->SetValue_t(nofault_type);//remove fault
            ScheduleFanout(gptr);
        }
	//Perform simulation
	LogicSim_t();
#ifdef DEBUG_TFSIM
	PrintTransition_t(); //debug
#endif

    }
    for (fite = UTFlist.begin();fite != UTFlist.end();) {
        fptr = *fite;
        if (fptr->GetStatus() == DETECTED || fptr->GetStatus() == REDUNDANT) {
            fite = UTFlist.erase(fite);
        }
        else { ++fite; }
    }

}
//
//single pattern single fault simulation
//Assume current logic values of gates stored at Value
//Assume previous logic values of gates stored at Value_t
void CIRCUIT::TFaultSim()
{
    register unsigned i;
    GATEPTR gptr;
    TFAULT *fptr;
    
    //for all undetected faults
    list<TFAULT*>::iterator fite;
    for (fite = UTFlist.begin();fite!=UTFlist.end();++fite) {
        fptr = *fite;
#ifdef DEBUG_TFSIM
	cout << "Checking: " << fptr->GetInputGate()->GetName() << "->" << fptr->GetOutputGate()->GetName() << ":" << fptr->GetValue() << endl;
	//debug
#endif
        //skip redundant and detected faults
        if (fptr->GetStatus() == REDUNDANT || fptr->GetStatus() == DETECTED) { continue; }
        //the fault is not active
	gptr=fptr->GetInputGate();//Input gate
        if (fptr->GetValue() == S0) { //Slow to fall; need a falling transition
            if (gptr->GetValue_t()==S0||gptr->GetValue()==S1) { 
		continue; 
	    }
	}
	else{//Slow to rise
            if (gptr->GetValue_t()==S1||gptr->GetValue()==S0) { 
		continue; 
	    }
	}
#ifdef DEBUG_TFSIM
	//debug
	cout << "Activated" << endl;
#endif
        //the fault can be directly seen at PO
        if (gptr->GetFlag(OUTPUT) && (!fptr->Is_Branch() || fptr->GetOutputGate()->GetFunction() == G_PO)) {
            fptr->SetStatus(DETECTED);
#ifdef DEBUG_TFSIM
		cout << "DETECTED 1: " << fptr->GetInputGate()->GetName() << "->" << fptr->GetOutputGate()->GetName() << ":" << fptr->GetValue() << endl;
		//debug
#endif
            continue;
        }
	if (!fptr->Is_Branch()) { //stem
	    gptr->SetValue(NotTable[fptr->GetValue()]); //inject fault
            ScheduleFanout(gptr);
        }
        else { //branch; Only inject faults at gate outputs, 
	       //so we need to propagate branch faults one level further
            if (!CheckTFaultyGate(fptr)) { continue; }
            gptr = fptr->GetOutputGate();
            //if the fault is shown at an output, it is detected
            if (gptr->GetFlag(OUTPUT)) {
                fptr->SetStatus(DETECTED);
#ifdef DEBUG_TFSIM
		cout << "DETECTED 2: " << fptr->GetInputGate()->GetName() << "->" << fptr->GetOutputGate()->GetName() << ":" << fptr->GetValue() << endl;
		//debug
#endif
                continue;
            }
            // add the fault to the simulated list and inject it
            VALUE fault_type = gptr->Is_Inversion()? fptr->GetValue() : NotTable[fptr->GetValue()];
	    gptr->SetValue(fault_type);//inject fault
            ScheduleFanout(gptr);
        }
	//Perform simulation
	VALUE new_value;
	bool detected(0);
        for (i = 0;i<= MaxLevel&&!detected; ++i) {
            while (!Queue[i].empty()) {
                gptr = Queue[i].front(); 
		Queue[i].pop_front();
                gptr->ResetFlag(SCHEDULED);
		new_value = Evaluate(gptr);
		if (new_value != gptr->GetValue()) {
		    gptr->SetValue(new_value);
		    ScheduleFanout(gptr);
		    if(gptr->GetFlag(OUTPUT)){
			fptr->SetStatus(DETECTED);
			detected=1;
#ifdef DEBUG_TFSIM
		cout << "DETECTED 3: " << fptr->GetInputGate()->GetName() << "->" << fptr->GetOutputGate()->GetName() << ":" << fptr->GetValue() << endl;
		//debug
#endif
			break; //Jump out of the for loop
		    }
		}
            }
        }
        for (i = 0;i<= MaxLevel; ++i) {
            while (!Queue[i].empty()) {
                //clean up the queues
                gptr = Queue[i].front(); 
		Queue[i].pop_front();
                gptr->ResetFlag(SCHEDULED);
	    }
	}

	//Simulate inverse of the faulty value to restore fault-free values from faulty sites
	if (!fptr->Is_Branch()) { //stem
            gptr = fptr->GetOutputGate();
	    gptr->SetValue(fptr->GetValue()); //remove fault
            ScheduleFanout(gptr);
        }
        else { //branch
            gptr = fptr->GetOutputGate();
            VALUE nofault_type = gptr->Is_Inversion()? NotTable[fptr->GetValue()] : fptr->GetValue();
	    gptr->SetValue(nofault_type);//remove fault
            ScheduleFanout(gptr);
        }
	//Perform simulation
	LogicSim();
#ifdef DEBUG_TFSIM
	PrintTransition(); //debug
#endif

    }
    for (fite = UTFlist.begin();fite != UTFlist.end();) {
        fptr = *fite;
        if (fptr->GetStatus() == DETECTED || fptr->GetStatus() == REDUNDANT) {
            fite = UTFlist.erase(fite);
        }
        else { ++fite; }
    }

}


//check if the transition fault can be propagated 
bool CIRCUIT::CheckTFaultyGate(TFAULT* fptr)
{
    register unsigned i;
    GATEPTR ogptr(fptr->GetOutputGate());
    VALUE ncv(NCV[ogptr->GetFunction()]);
    GATEPTR fanin, igptr(fptr->GetInputGate());
    for (i = 0;i< ogptr->No_Fanin();++i) {
        fanin = ogptr->Fanin(i);
        if (fanin == igptr) { continue; }
        if (fanin->GetValue() != ncv) { return false; }
    }
    return true;
}

//check if the transition fault can be propagated 
bool CIRCUIT::CheckTFaultyGate_t(TFAULT* fptr)
{
    register unsigned i;
    GATEPTR ogptr(fptr->GetOutputGate());
    VALUE ncv(NCV[ogptr->GetFunction()]);
    GATEPTR fanin, igptr(fptr->GetInputGate());
    for (i = 0;i< ogptr->No_Fanin();++i) {
        fanin = ogptr->Fanin(i);
        if (fanin == igptr) { continue; }
        if (fanin->GetValue_t() != ncv) { return false; }
    }
    return true;
}

//do event-driven logic simulation
void CIRCUIT::LogicSim_t()
{
    GATE* gptr;
    VALUE new_value;
    for (unsigned i = 0;i <= MaxLevel;i++) {
        while (!Queue[i].empty()) {
            gptr = Queue[i].front();
            Queue[i].pop_front();
            gptr->ResetFlag(SCHEDULED);
            new_value = Evaluate_t(gptr);
            if (new_value != gptr->GetValue_t()) {
                gptr->SetValue_t(new_value);
                ScheduleFanout(gptr);
            }
        }
    }
    return;
}

//evaluate the output value of gate
VALUE CIRCUIT::Evaluate_t(GATEPTR gptr)
{
    GATEFUNC fun(gptr->GetFunction());
    VALUE cv(CV[fun]); //controling value
    VALUE value(gptr->Fanin(0)->GetValue_t());
    switch (fun) {
        case G_AND:
        case G_NAND:
            for (unsigned i = 1;i<gptr->No_Fanin() && value != cv;++i) {
                value = AndTable[value][gptr->Fanin(i)->GetValue_t()];
            }
            break;
        case G_OR:
        case G_NOR:
            for (unsigned i = 1;i<gptr->No_Fanin() && value != cv;++i) {
                value = OrTable[value][gptr->Fanin(i)->GetValue_t()];
            }
            break;
        default: break;
    }
    //NAND, NOR and NOT
    if (gptr->Is_Inversion()) { value = NotTable[value]; }
    return value;
}

void CIRCUIT::PrintTransition_t()
{
    register unsigned i;
    GATEPTR gptr;
    for (i = 0;i<No_Gate();++i) { 
	    gptr=Gate(i);
	    cout << gptr->GetName() << ":" << gptr->GetTransition_t() << " "; 
    }
    cout << endl;
    cout << "--------------------";
    cout << endl;
    return;
}

void CIRCUIT::PrintTransition()
{
    register unsigned i;
    GATEPTR gptr;
    for (i = 0;i<No_Gate();++i) { 
	    gptr=Gate(i);
	    cout << gptr->GetName() << ":" << gptr->GetTransition() << " "; 
    }
    cout << endl;
    cout << "--------------------";
    cout << endl;
    return;
}

void CIRCUIT::PrintIO_t()
{
    register unsigned i;
    for (i = 0;i<No_PI();++i) { cout << PIGate(i)->GetValue_t(); }
    cout << " ";
    for (i = 0;i<No_PO();++i) { cout << POGate(i)->GetValue_t(); }
    cout << endl;
    return;
}

