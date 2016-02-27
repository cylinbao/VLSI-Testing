/* stuck-at fault simulator for combinational circuit
 * Last update: 2006/12/09 */
#include <iostream>
#include "circuit.h"
#include "GetLongOpt.h"
using namespace std;

extern GetLongOpt option;

// fault simulation test patterns
void CIRCUIT::FaultSimVectors()
{
    cout << "Run stuck-at fault simulation" << endl;
    unsigned pattern_num(0);
    if(!Pattern.eof()){ // Readin the first vector
        while(!Pattern.eof()){
            ++pattern_num;
            Pattern.ReadNextPattern();
            //fault-free simulation
            SchedulePI();
            LogicSim();
            //single pattern parallel fault simulation
            FaultSim();
        }
    }

    //compute fault coverage
    unsigned total_num(0);
    unsigned undetected_num(0), detected_num(0);
    unsigned eqv_undetected_num(0), eqv_detected_num(0);
    FAULT* fptr;
    list<FAULT*>::iterator fite;
    for (fite = Flist.begin();fite!=Flist.end();++fite) {
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
    cout << "Equivalent fault number = " << Flist.size() << endl;
    cout << "Equivalent detected fault number = " << eqv_detected_num << endl; 
    cout << "Equivalent undetected fault number = " << eqv_undetected_num << endl; 
    cout << "---------------------------------------" << endl;
    cout << "Fault Coverge = " << 100*detected_num/double(total_num) << "%" << endl;
    cout << "Equivalent FC = " << 100*eqv_detected_num/double(Flist.size()) << "%" << endl;
    cout << "---------------------------------------" << endl;
    return;
}

//single pattern parallel fault simulation
//parallel fault number is defined by PatternNum in typeemu.h
void CIRCUIT::FaultSim()
{
    register unsigned i, fault_idx(0);
    GATEPTR gptr;
    FAULT *fptr;
    FAULT *simulate_flist[PatternNum];
    list<GATEPTR>::iterator gite;
    //initial all gates
    for (i = 0; i < Netlist.size(); ++i) {
        Netlist[i]->SetFaultFreeValue();
    }

    //for all undetected faults
    list<FAULT*>::iterator fite;
    for (fite = UFlist.begin();fite!=UFlist.end();++fite) {
        fptr = *fite;
        //skip redundant and detected faults
        if (fptr->GetStatus() == REDUNDANT || fptr->GetStatus() == DETECTED) { continue; }
        //the fault is not active
        if (fptr->GetValue() == fptr->GetInputGate()->GetValue()) { continue; }
        //the fault can be directly seen
        gptr = fptr->GetInputGate();
        if (gptr->GetFlag(OUTPUT) && (!fptr->Is_Branch() || fptr->GetOutputGate()->GetFunction() == G_PO)) {
            fptr->SetStatus(DETECTED);
            continue;
        }
        if (!fptr->Is_Branch()) { //stem
            if (!gptr->GetFlag(FAULTY)) {
                gptr->SetFlag(FAULTY); GateStack.push_front(gptr);
            }
            InjectFaultValue(gptr, fault_idx, fptr->GetValue());
            gptr->SetFlag(FAULT_INJECTED);
            ScheduleFanout(gptr);
            simulate_flist[fault_idx++] = fptr;
        }
        else { //branch
            if (!CheckFaultyGate(fptr)) { continue; }
            gptr = fptr->GetOutputGate();
            //if the fault is shown at an output, it is detected
            if (gptr->GetFlag(OUTPUT)) {
                fptr->SetStatus(DETECTED);
                continue;
            }
            if (!gptr->GetFlag(FAULTY)) {
                gptr->SetFlag(FAULTY); GateStack.push_front(gptr);
            }
            // add the fault to the simulated list and inject it
            VALUE fault_type = gptr->Is_Inversion()? NotTable[fptr->GetValue()]: fptr->GetValue();
            InjectFaultValue(gptr, fault_idx, fault_type);
            gptr->SetFlag(FAULT_INJECTED);
            ScheduleFanout(gptr);
            simulate_flist[fault_idx++] = fptr;
        }

        //collect PatternNum fault, do fault simulation
        if (fault_idx == PatternNum) {
            //do parallel fault simulation
            for (i = 0;i<= MaxLevel; ++i) {
                while (!Queue[i].empty()) {
                    gptr = Queue[i].front(); Queue[i].pop_front();
                    gptr->ResetFlag(SCHEDULED);
                    FaultSimEvaluate(gptr);
                }
            }

            // check detection and reset wires' faulty values
            // back to fault-free values
            for (gite = GateStack.begin();gite != GateStack.end();++gite) {
                gptr = *gite;
                //clean flags
                gptr->ResetFlag(FAULTY);
                gptr->ResetFlag(FAULT_INJECTED);
                gptr->ResetFaultFlag();
                if (gptr->GetFlag(OUTPUT)) {
                    for (i = 0; i < fault_idx; ++i) {
                        if (simulate_flist[i]->GetStatus() == DETECTED) { continue; }
                        //faulty value != fault-free value && fault-free != X &&
                        //faulty value != X (WireValue1[i] == WireValue2[i])
                        if (gptr->GetValue() != VALUE(gptr->GetValue1(i)) && gptr->GetValue() != X 
                                && gptr->GetValue1(i) == gptr->GetValue2(i)) {
                            simulate_flist[i]->SetStatus(DETECTED);
                        }
                    }
                }
                gptr->SetFaultFreeValue();    
            } //end for GateStack
            GateStack.clear();
            fault_idx = 0;
        } //end fault simulation
    } //end for all undetected faults

    //fault sim rest faults
    if (fault_idx) {
        //do parallel fault simulation
        for (i = 0;i<= MaxLevel; ++i) {
            while (!Queue[i].empty()) {
                gptr = Queue[i].front(); Queue[i].pop_front();
                gptr->ResetFlag(SCHEDULED);
                FaultSimEvaluate(gptr);
            }
        }

        // check detection and reset wires' faulty values
        // back to fault-free values
        for (gite = GateStack.begin();gite != GateStack.end();++gite) {
            gptr = *gite;
            //clean flags
            gptr->ResetFlag(FAULTY);
            gptr->ResetFlag(FAULT_INJECTED);
            gptr->ResetFaultFlag();
            if (gptr->GetFlag(OUTPUT)) {
                for (i = 0; i < fault_idx; ++i) {
                    if (simulate_flist[i]->GetStatus() == DETECTED) { continue; }
                    //faulty value != fault-free value && fault-free != X &&
                    //faulty value != X (WireValue1[i] == WireValue2[i])
                    if (gptr->GetValue() != VALUE(gptr->GetValue1(i)) && gptr->GetValue() != X 
                            && gptr->GetValue1(i) == gptr->GetValue2(i)) {
                        simulate_flist[i]->SetStatus(DETECTED);
                    }
                }
            }
            gptr->SetFaultFreeValue();    
        } //end for GateStack
        GateStack.clear();
        fault_idx = 0;
    } //end fault simulation

    // remove detected faults
    for (fite = UFlist.begin();fite != UFlist.end();) {
        fptr = *fite;
        if (fptr->GetStatus() == DETECTED || fptr->GetStatus() == REDUNDANT) {
            fite = UFlist.erase(fite);
        }
        else { ++fite; }
    }
    return;
}

//evaluate parallel faulty value of gptr
void CIRCUIT::FaultSimEvaluate(GATEPTR gptr)
{
    register unsigned i;
    bitset<PatternNum> new_value1(gptr->Fanin(0)->GetValue1());
    bitset<PatternNum> new_value2(gptr->Fanin(0)->GetValue2());
    switch(gptr->GetFunction()) {
        case G_AND:
        case G_NAND:
            for (i = 1; i < gptr->No_Fanin(); ++i) {
                new_value1 &= gptr->Fanin(i)->GetValue1();
                new_value2 &= gptr->Fanin(i)->GetValue2();
            }
            break;
        case G_OR:
        case G_NOR:
            for (i = 1; i < gptr->No_Fanin(); ++i) {
                new_value1 |= gptr->Fanin(i)->GetValue1();
                new_value2 |= gptr->Fanin(i)->GetValue2();
            }
            break;
        default: break;
    } 
    //swap new_value1 and new_value2 to avoid unknown value masked
    if (gptr->Is_Inversion()) {
        new_value1.flip(); new_value2.flip();
        bitset<PatternNum> value(new_value1);
        new_value1 = new_value2; new_value2 = value;
    }
    if (gptr->GetValue1() != new_value1 || gptr->GetValue2() != new_value2) {
        //if gptr has fault injected, reverse the injected value
        if (gptr->GetFlag(FAULT_INJECTED)) {
            for (i = 0; i < PatternNum; ++i) {
                if (!gptr->GetFaultFlag(i)) { continue; }
                //recover injected fault value
                new_value1[i] = gptr->GetValue1(i);
                new_value2[i] = gptr->GetValue2(i);
            }
        }
        //set gptr as FAULTY and push to gate stack
        if (!(gptr->GetFlag(FAULTY))) {
            gptr->SetFlag(FAULTY); GateStack.push_front(gptr);
        }
        gptr->SetValue1(new_value1);
        gptr->SetValue2(new_value2);
        ScheduleFanout(gptr);
    }
    return;
}

//check if the fault can be propagated
bool CIRCUIT::CheckFaultyGate(FAULT* fptr)
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

//inject faulty value to the gate
void CIRCUIT::InjectFaultValue(GATEPTR gptr, unsigned idx,VALUE value)
{
    if (value == S1) {
        gptr->SetValue1(idx);
        gptr->SetValue2(idx);
    }
    else if (value == S0) {
        gptr->ResetValue1(idx);
        gptr->ResetValue2(idx);
    }
    gptr->SetFaultFlag(idx);
    return;
}

//mark gates connecting to PO
void CIRCUIT::MarkOutputGate()
{
    for (unsigned i = 0;i<POlist.size();++i) {
        POlist[i]->SetFlag(OUTPUT);
        POlist[i]->Fanin(0)->SetFlag(OUTPUT);
    }
    return;
}
