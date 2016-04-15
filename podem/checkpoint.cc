/* stuck-at fault ATPG for combinational circuit
 * Last update: 2006/12/09 */
#include <iostream>
#include "circuit.h"
#include "GetLongOpt.h"
using namespace std;

extern GetLongOpt option;

//generate all stuck-at fault list
void CIRCUIT::GenerateAllCPFaultList()
{
    cout << "Generate checkpoint fault list" << endl;
    register unsigned i, j;
    GATEFUNC fun;
    GATEPTR gptr, fanout;
    FAULT *fptr;
    for (i = 0;i<No_Gate();++i) {
        gptr = Netlist[i]; fun = gptr->GetFunction();
				if (fun == G_PI) { 
					//add stem stuck-at 0 fault to Flist
					fptr = new FAULT(gptr, gptr, S0);
					CPFlist.push_front(fptr);
					//add stem stuck-at 1 fault to Flist
					fptr = new FAULT(gptr, gptr, S1);
					CPFlist.push_front(fptr);
				}

        if (gptr->No_Fanout() == 1) { continue; } //no branch faults

        //add branch fault
        for (j = 0;j< gptr->No_Fanout();++j) {
            fanout = gptr->Fanout(j);
            fptr = new FAULT(gptr, fanout, S0);
            fptr->SetBranch(true);
            CPFlist.push_front(fptr);
            fptr = new FAULT(gptr, fanout, S1);
            fptr->SetBranch(true);
            CPFlist.push_front(fptr);
        } //end all fanouts
    } //end all gates
    //copy Flist to undetected Flist (for fault simulation)
    UCPFlist = CPFlist;
    return;
}

void CIRCUIT::CalculatePercentage()
{
	float perc;
	
	perc = ((float)CPFlist.size() / Flist.size())*100;

	cout << "CPFlist size = " << CPFlist.size() << endl;
	cout << "Flist size = " << Flist.size() << endl;
	cout << perc << "\% of faults have been collapsed\n";
}
