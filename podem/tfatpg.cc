/* Transition fault ATPG for combinational circuit
 * Last update: 2007/08/19 */
#include <iostream>
#include "circuit.h"
#include "GetLongOpt.h"
using namespace std;

extern GetLongOpt option;

//Transition fualt PODEM ATPG (fault dropping)
void CIRCUIT::TFAtpg()
{
    cout << "Run transition fault ATPG" << endl;
    unsigned i, total_backtrack_num(0), pattern_num(0);
    ATPG_STATUS status;
    TFAULT* fptr;
    FAULT* vfptr;
    list<TFAULT*>::iterator fite;

    //Prepare the output files
    ofstream OutputStrm;
    if (option.retrieve("output")){
        OutputStrm.open((char*)option.retrieve("output"),ios::out);
        if(!OutputStrm){
              cout << "Unable to open output file: "
                   << option.retrieve("output") << endl;
              cout << "Unsaved output!\n";
              exit(-1);
        }
    }

    if (option.retrieve("output")){
	    for (i = 0;i<PIlist.size();++i) {
		OutputStrm << "PI " << PIlist[i]->GetName() << " ";
	    }
	    OutputStrm << endl;
    }

    for (fite = TFlist.begin(); fite != TFlist.end();++fite) {
        fptr = *fite;
#ifdef DEBUG_ATPG
	//debug
	cout << "Generate: " << fptr->GetInputGate()->GetName() << "->" << fptr->GetOutputGate()->GetName() << ":" << fptr->GetValue() << endl;
#endif
        if (fptr->GetStatus() == DETECTED) { 
#ifdef DEBUG_ATPG
			cout << "Detected by fault simulation." << endl;
#endif
		continue; 
	}
#ifdef DEBUG_ATPG
	//debug
	cout << "Initialization." << endl;
#endif
        //run justification to initialize the first timeframe
        status = Initialization(fptr->GetInputGate(), NotTable[fptr->GetValue()], total_backtrack_num);
	if(status==TRUE){
		//Create a stuck-at fault for the second timeframe
		vfptr= new FAULT(fptr->GetInputGate(), fptr->GetOutputGate(), NotTable[fptr->GetValue()]);
		if(fptr->Is_Branch()) {vfptr->SetBranch(true);}
		status = Podem(vfptr, total_backtrack_num);
		switch (status) {
		    case TRUE:
#ifdef DEBUG_ATPG
			cout << "Detected." << endl;
#endif
			fptr->SetStatus(DETECTED);
			++pattern_num;
			//run fault simulation for fault dropping
			for (i = PIlist.size();i<Netlist.size();++i) { Netlist[i]->SetValue_t(X); Netlist[i]->SetValue(X);  }
			for (i = 0;i < PIlist.size();++i) { 
				ScheduleFanout(PIlist[i]); 
				if (option.retrieve("output")){ OutputStrm << PIlist[i]->GetValue_t();}
			}
			if (option.retrieve("output")){ OutputStrm << endl;}
			LogicSim_t();
			for (i = 0;i < PIlist.size();++i) { 
				ScheduleFanout(PIlist[i]); 
				if (option.retrieve("output")){ OutputStrm << PIlist[i]->GetValue();}
			}
			if (option.retrieve("output")){ OutputStrm << endl;}
			LogicSim();
			TFaultSim();
			break;
		    case CONFLICT:
#ifdef DEBUG_ATPG
			cout << "Undetected." << endl;
#endif
			fptr->SetStatus(REDUNDANT);
			break;
		    case FALSE:
#ifdef DEBUG_ATPG
			cout << "ABORT." << endl;
#endif
			fptr->SetStatus(ABORT);
			break;
		}
	}
	else if (status==CONFLICT) fptr->SetStatus(REDUNDANT);
        if (status==FALSE) fptr->SetStatus(ABORT);
    } //end all faults

    //compute fault coverage
    unsigned total_num(0);
    unsigned abort_num(0), redundant_num(0), detected_num(0);
    unsigned eqv_abort_num(0), eqv_redundant_num(0), eqv_detected_num(0);
    for (fite = TFlist.begin();fite!=TFlist.end();++fite) {
        fptr = *fite;
        switch (fptr->GetStatus()) {
            case DETECTED:
                ++eqv_detected_num;
                detected_num += fptr->GetEqvFaultNum();
                break;
            case REDUNDANT:
                ++eqv_redundant_num;
                redundant_num += fptr->GetEqvFaultNum();
                break;
            case ABORT:
                ++eqv_abort_num;
                abort_num += fptr->GetEqvFaultNum();
                break;
            default:
                cerr << "Unknown fault type exists" << endl;
                break;
        }
    }
    total_num = detected_num + abort_num + redundant_num;

    cout.setf(ios::fixed);
    cout.precision(2);
    cout << "---------------------------------------" << endl;
    cout << "Test pattern number = " << pattern_num << endl;
    cout << "Total backtrack number = " << total_backtrack_num << endl;
    cout << "---------------------------------------" << endl;
    cout << "Total fault number = " << total_num << endl;
    cout << "Detected fault number = " << detected_num << endl;
    cout << "Undetected fault number = " << abort_num + redundant_num << endl;
    cout << "Abort fault number = " << abort_num << endl;
    cout << "Redundant fault number = " << redundant_num << endl;
    cout << "---------------------------------------" << endl;
    cout << "Total equivalent fault number = " << TFlist.size() << endl;
    cout << "Equivalent detected fault number = " << eqv_detected_num << endl;
    cout << "Equivalent undetected fault number = " << eqv_abort_num + eqv_redundant_num << endl;
    cout << "Equivalent abort fault number = " << eqv_abort_num << endl;
    cout << "Equivalent redundant fault number = " << eqv_redundant_num << endl;
    cout << "---------------------------------------" << endl;
    cout << "Fault Coverge = " << 100*detected_num/double(total_num) << "%" << endl;
    cout << "Equivalent FC = " << 100*eqv_detected_num/double(TFlist.size()) << "%" << endl;
    cout << "Fault Efficiency = " << 100*detected_num/double(total_num - redundant_num) << "%" << endl;
    cout << "---------------------------------------" << endl;
    return;
}

ATPG_STATUS CIRCUIT::Initialization(GATEPTR gptr, VALUE target, unsigned &total_backtrack_num)
{
    unsigned i, backtrack_num(0);
    GATEPTR pi_gptr(0), decision_gptr(0);
    ATPG_STATUS status;

    //set all values as unknown
    for (i = 0;i<Netlist.size();++i) { Netlist[i]->SetValue_t(X); }
    //propagate fault free value
    status = BackwardImply_t(gptr, target);
    switch (status) {
        case TRUE:
            LogicSim_t();
            //check if the target is satisfied
	    if (gptr->GetValue_t() != target) { status = FALSE; }
            break;
        case CONFLICT:
            status = CONFLICT;
            break;
        case FALSE: break;
    }

    while(backtrack_num < BackTrackLimit && status == FALSE) {
        //search possible PI decision
        pi_gptr = FindPIAssignment_t(gptr, target);
        if (pi_gptr) { //decision found
            ScheduleFanout(pi_gptr);
            //push to decision tree
            GateStack.push_back(pi_gptr);
            decision_gptr = pi_gptr;
        }
        else { //backtrack previous decision
            while (!GateStack.empty() && !pi_gptr) {
                //all decision tried (1 and 0)
                if (decision_gptr->GetFlag(ALL_ASSIGNED)) {
                    decision_gptr->ResetFlag(ALL_ASSIGNED);
                    decision_gptr->SetValue(X);
                    ScheduleFanout(decision_gptr);
                    //remove decision from decision tree
                    GateStack.pop_back();
                    decision_gptr = GateStack.back();
                }
                //inverse current decision value
                else {
                    decision_gptr->InverseValue_t();
                    ScheduleFanout(decision_gptr);
                    decision_gptr->SetFlag(ALL_ASSIGNED);
                    ++backtrack_num;
                    pi_gptr = decision_gptr;
                }
            }
            //no other decision
            if (!pi_gptr) { status = CONFLICT; }
        }
        if (pi_gptr) {
            LogicSim_t();
	    if (gptr->GetValue_t() == target) { status = TRUE; }
        }
    } //end while loop

    //clean ALL_ASSIGNED and MARKED flags
    list<GATEPTR>::iterator gite;
    for (gite = GateStack.begin();gite != GateStack.end();++gite) {
        (*gite)->ResetFlag(ALL_ASSIGNED);
    }
    //clear stacks
    GateStack.clear(); 

    //assign test pattern to PIs
    if (status ==  TRUE) {
        for (i = 0;i<PIlist.size();++i) {
            if (PIlist[i]->GetValue_t()== X){
                        PIlist[i]->SetValue_t(VALUE(2.0 * rand()/(RAND_MAX + 1.0)));
	    }
        }//end for all PI
    } //end status == TRUE
    total_backtrack_num += backtrack_num;
    return status;
}


//apply the input values of gate according to its output value
//TRUE: fault can be backward propagated to PI
//CONFLICT: conflict assignment
//FALSE: fault can not be propagated to PI
ATPG_STATUS CIRCUIT::BackwardImply_t(GATEPTR gptr, VALUE value)
{
    register unsigned i;
    register ATPG_STATUS status(FALSE);
    GATEFUNC fun(gptr->GetFunction());

    if (fun == G_PI) { //reach PI
        //conflict assignment
        if (gptr->GetValue_t() != X && gptr->GetValue_t() != value) {
            return CONFLICT;
        }
        gptr->SetValue_t(value); ScheduleFanout(gptr);
        return TRUE;
    }
    //not PI gate
    switch (fun) {
        case G_NOT:
            switch (BackwardImply_t(gptr->Fanin(0), NotTable[value])) {
                case TRUE: status = TRUE; break;
                case FALSE: break;
                case CONFLICT: return CONFLICT; break;
            }
            break;
        case G_BUF:
            switch (BackwardImply_t(gptr->Fanin(0), value)) {
                case TRUE: status = TRUE; break;
                case FALSE: break;
                case CONFLICT: return CONFLICT; break;
            }
            break;
        case G_AND:
        case G_OR:
            if (value != NCV[fun]) { break; }
            for (i = 0;i<gptr->No_Fanin();++i) {
                switch (BackwardImply_t(gptr->Fanin(i), NCV[fun])) {
                    case TRUE: status = TRUE; break;
                    case FALSE: break;
                    case CONFLICT: return CONFLICT; break;
                }
            }
            break;
        case G_NAND:
        case G_NOR:
            if (value != CV[fun]) { break; }
            for (i = 0;i<gptr->No_Fanin();++i) {
                switch (BackwardImply_t(gptr->Fanin(i), NCV[fun])) {
                    case TRUE: status = TRUE; break;
                    case FALSE: break;
                    case CONFLICT: return CONFLICT; break;
                }
            }
            break;
        default: break;
    }
    return status;
}

//find PI decision to set gptr = value
//success: return PI
//fail: return 0
GATEPTR CIRCUIT::FindPIAssignment_t(GATEPTR gptr, VALUE value)
{
    //search PI desicion
    if (gptr->GetFunction() == G_PI) {
        gptr->SetValue_t(value);
        return gptr;
    }
    GATEPTR j_gptr(0); //J-frontier
    VALUE j_value(X), cv_out;
    switch (gptr->GetFunction()) {
        case G_AND:
        case G_NAND:
        case G_OR:
        case G_NOR:
            cv_out = CV[gptr->GetFunction()];
            cv_out = gptr->Is_Inversion()? NotTable[cv_out]: cv_out;
            //select one fanin as cv
            if (value == cv_out) {
                j_gptr = FindEasiestControl_t(gptr);
                j_value = CV[gptr->GetFunction()];
            }
            //select one fanin as ncv 
            else {
                j_gptr = FindHardestControl_t(gptr);
                j_value = NCV[gptr->GetFunction()];
            }
            break;
        case G_BUF:
        case G_NOT:
            j_value = gptr->Is_Inversion()? NotTable[value]: value;
            j_gptr = gptr->Fanin(0);
            break;
        default:
            break;
    }
    if (j_gptr) { return FindPIAssignment_t(j_gptr, j_value); }
    return 0;
}


//serach lowest level unknown fanin
GATEPTR CIRCUIT::FindEasiestControl_t(GATEPTR gptr)
{
    GATEPTR fanin;
    for (unsigned i = 0;i< gptr->No_Fanin();++i) {
        fanin = gptr->Fanin(i);
        if (fanin->GetValue_t() == X) { return fanin; }
    }
    return 0;
}

//serach highest level unknown fanin
GATEPTR CIRCUIT::FindHardestControl_t(GATEPTR gptr)
{
    GATEPTR fanin;
    for (unsigned i = gptr->No_Fanin();i>0;--i) {
        fanin = gptr->Fanin(i-1);
        if (fanin->GetValue_t() == X) { return fanin; }
    }
    return 0;
}
