/* stuck-at fault ATPG for combinational circuit
 * Last update: 2006/12/09 */
#include <iostream>
#include "circuit.h"
#include "GetLongOpt.h"
#include "bridgingFault.h"
using namespace std;

extern GetLongOpt option;

void CIRCUIT::PutGateIntoQueueByLevel()
{
	unsigned i;
	GATE* p_gate;

	for(i = 0; i < Netlist.size(); i++){
		p_gate = Netlist[i];
		Queue[p_gate->GetLevel()].push_back(p_gate);
	}
}

//generate all stuck-at fault list
void CIRCUIT::GenerateAllBFaultList()
{
    cout << "Generate bridging fault list" << endl;
    register unsigned i;
    GATEPTR gptr;
    BRIDGING_FAULT *fptr;

		for(i = 0; i < MaxLevel; i++){
			while(Queue[i].size() > 1){
				gptr = Queue[i].front();
				Queue[i].pop_front();

				fptr = new BRIDGING_FAULT(gptr, Queue[i].front(), AND);
				BFlist.push_back(fptr);
				fptr = new BRIDGING_FAULT(gptr, Queue[i].front(), OR);
				BFlist.push_back(fptr);
			}
			Queue[i].pop_front();
		}

    //copy Flist to undetected Flist (for fault simulation)
    UBFlist = BFlist;
    return;
}

void CIRCUIT::OutputAllBFaultList()
{
	unsigned i;
	BRIDGING_FAULT *bfptr;

	cout << "Print bridging faults to output file\n";
	for(i = 0; i < BFlist.size(); i++){
		bfptr = BFlist[i];
		ofs << "(" << bfptr->GetInputGate()->GetName() << ", ";	
		ofs << bfptr->GetOutputGate()->GetName() << ", ";
		ofs << FaultTable[bfptr->GetType()] << ")\n";
	}
}
