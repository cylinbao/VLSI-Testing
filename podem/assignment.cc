/* VLSI-Testing Assignment
 * Last update: 2016/02/29 */
#include <iostream>
#include "circuit.h"
using namespace std;

// Add a function for the assignment0 //
void CIRCUIT::Ass0()
{
		cout << "It's processing command ass0" << endl;
		cout << "!!! Print out all the elemenst in Netlist !!!" << endl;
		printNetlist();
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
		// TODO
		cout << "Number of branch nets: " << No_Branch() << endl;
		cout << "number of stem nets: " << No_Stem() << endl;
		cout << "Avg num of fanouts or each gates: " << No_Avg_Fanout() << endl;
}

void CIRCUIT::printNetlist()
{
		vector<GATE*>::iterator it;

		for(it = Netlist.begin(); it != Netlist.end(); it++) {
			cout << "Gate name: " << (*it)->GetName() << ", ";
			cout << "Gate ID: " << (*it)->GetID() << ", ";
			cout << "Gate function: " << (*it)->GetFunction() << endl;
		}
}

void CIRCUIT::init_NO_Gate_Func_list()
{
		vector<int>::iterator it;

		for(it = NO_Gate_Func_list.begin(); it != NO_Gate_Func_list.end(); it++) {
				*it = 0;
		}
}

void CIRCUIT::computeGateNumberByType()
{
		init_NO_Gate_Func_list();

		vector<GATE*>::iterator it;

		for(it = Netlist.begin(); it != Netlist.end(); it++) {
				NO_Gate_Func_list[(*it)->GetFunction()] += 1;
		}
}

unsigned CIRCUIT::No_Tot_Gate()
{
		computeGateNumberByType();

		unsigned acc = 0;

		for(int i = 4; i < 9; i++) {
				acc += NO_Gate_Func_list[i];
		}

		return acc;
}
