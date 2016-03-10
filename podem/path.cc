#include <string>
#include <iostream>
#include "circuit.h"
using namespace std;

void CIRCUIT::path(string src_gate_name, string dest_gate_name)
{
	GATE empty_gate;
	GATE *src_gate = &empty_gate, *dest_gate = &empty_gate;
	vector<GATE*>::iterator it_list;

	for(it_list = PIlist.begin(); it_list != PIlist.end(); ++it_list)
		if((*it_list)->GetName() == src_gate_name)
			src_gate = (*it_list);

	if(src_gate == &empty_gate){
		cerr << "Can not find starting input gate " << src_gate_name << endl;
		return;
	}

	for(it_list = POlist.begin(); it_list != POlist.end(); ++it_list)
		if((*it_list)->GetName() == dest_gate_name)
			dest_gate = (*it_list);

	if(dest_gate == &empty_gate){
		cerr << "Can not find ending output gate " << dest_gate_name << endl;
		return;
	}

	//cout << "Source gate: " << src_gate->GetName() << endl;
	//cout << "Dest gate: " << dest_gate->GetName() << endl;
	bool path_flag = false;
	path_flag = findPath(src_gate, dest_gate);

	if(path_flag == true)
		printPath(src_gate_name, dest_gate_name);
	else{
		cout << "Can not find path from " << src_gate_name << " to ";
		cout << dest_gate_name << "!" << endl;
	}
}

// if it finds a path, then return true
bool CIRCUIT::findPath(GATE *src_gate, GATE *dest_gate)
{
	bool path_flag = false;
	unsigned no_src_out;
	unsigned i;
	GATE *gate_ptr;

	no_src_out = src_gate->No_Fanout();
	for(i = 0; i < no_src_out; ++i){
		gate_ptr = src_gate->Fanout(i);	
		if(gate_ptr == dest_gate){
			path_stack.push(gate_ptr);
			path_stack.push(src_gate);
			return true;
		}
		else
			path_flag = findPath(gate_ptr, dest_gate);
	}
	if(path_flag == true){
		path_stack.push(src_gate);
		return true;
	}
	else
		return false;
}

void CIRCUIT::printPath(string src_gate_name, string dest_gate_name)
{
	string gate_name;
	bool meet_end_gate = true;
	int count = 0;

	cout << endl << "Found paths:" << endl;
	while(!path_stack.empty()){
		gate_name = path_stack.top()->GetName();
		if(meet_end_gate == true){
			cout << src_gate_name << " ";
			meet_end_gate = false;
		}
		if(gate_name != src_gate_name)
			cout << gate_name << " ";
		if(gate_name == dest_gate_name){
			count++;
			meet_end_gate = true;
			cout << endl;
		}
		path_stack.pop();
	}
	cout << "The paths from " << src_gate_name << " to ";
	cout << dest_gate_name << ": " << count << endl << endl;
}
