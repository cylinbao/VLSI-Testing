/* VLSI-Testing Assignment
 * Last update: 2016/02/29 */
#include <iostream>
#include "circuit.h"
using namespace std;

// Add a function for the assignment0 //
void CIRCUIT::Ass0()
{
		computeNeededData();
		cout << "It's processing command ass0" << endl << endl;
		//printNetlist();
    cout << "Number of inputs: " << No_PI() << endl;
    cout << "Number of outputs: " << No_PO() << endl;
    cout << "Total number of gates(INV, OR, NOR, AND and NAND): " 
				 << No_Tot_Gate() << endl;
		cout << "Number of NOT gates: " << No_NOT_Gate() << endl;
		cout << "Number of OR gates: " << No_OR_Gate() << endl;
		cout << "Number of NOR gates: " << No_NOR_Gate() << endl;
		cout << "Number of AND gates: " << No_AND_Gate() << endl;
		cout << "Number of NAND gates: " << No_NAND_Gate() << endl;
		if(No_DFF() != 0)
			cout << "Number of flip-flops: " << No_DFF() << endl;
		cout << "Total number of signal nets: " << No_Gate() << endl;
		cout << "Number of branch nets: " << No_Branch() << endl;
		cout << "number of stem nets: " << No_Stem() << endl;
		//cout << "Total number of fanouts of each gates: " << No_Tot_Fanout() << endl;
		cout << "Avg number of fanouts of each gates: " << No_Avg_Fanout() << endl;
		cout << endl;
}

void CIRCUIT::printNetlist()
{
		vector<GATE*>::iterator it;

		cout << "!!! Print out all the elemenst in Netlist !!!" << endl;

		for(it = Netlist.begin(); it != Netlist.end(); it++) {
			cout << "Gate name: " << (*it)->GetName() << ", ";
			cout << "Gate ID: " << (*it)->GetID() << ", ";
			cout << "Gate function: " << (*it)->GetFunction() << endl;
		}
}

void CIRCUIT::computeNeededData()
{
		vector<int>::iterator itGateFuncList;

		for(itGateFuncList = NO_Gate_Func_list.begin(); 
				itGateFuncList != NO_Gate_Func_list.end(); itGateFuncList++) {
				*itGateFuncList = 0;
		}

		TotalGateNum = 0;
		TotalBranchNets = 0;
		TotalStemNets = 0;
		TotalFanout = 0;
		AvgFanout = 0.0;

		vector<GATE*>::iterator itNetlist;

		for(itNetlist = Netlist.begin(); itNetlist != Netlist.end(); itNetlist++) {
				NO_Gate_Func_list[(*itNetlist)->GetFunction()] += 1;

				if((*itNetlist)->No_Fanout() > 1)		
					TotalBranchNets += 1;

				TotalFanout += (*itNetlist)->No_Fanout();

				if((*itNetlist)->No_Fanin() > 1)		
					TotalStemNets += 1;
		}

		for(int i = 4; i < 9; i++) {
				TotalGateNum += NO_Gate_Func_list[i];
		}

		AvgFanout = ((float)TotalFanout) / ((float)No_Gate());
}
