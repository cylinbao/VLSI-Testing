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
	path_stack.push_back(src_gate);
	path_count = 0;
	findPath(dest_gate);

	if(path_count > 0){
		cout << "The paths from " << src_gate_name << " to ";
		cout << dest_gate_name << ": " << path_count << endl;
	}
	else{
		cout << "Can not find path from " << src_gate_name << " to ";
		cout << dest_gate_name << "!" << endl;
	}
}

// if it finds a path, then return true
bool CIRCUIT::findPath(GATE *dest_gate)
{
	bool path_flag = false;
	unsigned no_src_out, i;
	GATE *next_gate, *src_gate;

	src_gate = path_stack.back();
	if(src_gate == dest_gate){
		printPath();
		path_stack.pop_back();
		path_count++;
		return true;
	}

	no_src_out = src_gate->No_Fanout();

	for(i = 0; i < no_src_out; ++i){
		bool temp_flag;
		next_gate = src_gate->Fanout(i);	

		if(next_gate->getDFSStatus() != BLACK){
			path_stack.push_back(next_gate);
			temp_flag = findPath(dest_gate);
			if(path_flag == false)
				path_flag = temp_flag;
		}
	}

	if(path_flag == false)
		src_gate->setDFSStatus(BLACK);

	path_stack.pop_back();
	return path_flag;
}

void CIRCUIT::printPath()
{
	string gate_name;
	vector<GATE*>::iterator it_stack;

	cout << endl << "Found paths:" << endl;
	for(it_stack = path_stack.begin(); it_stack != path_stack.end(); ++it_stack){
		cout << (*it_stack)->GetName() << " ";	
	}
	cout << endl;
}
