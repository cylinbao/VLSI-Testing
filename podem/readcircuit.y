/* Parser for reading the benchmark circuit in .bench format */

%{

#include <cstdio>
#include <iostream>
#include "gate.h"
#include "circuit.h"
#include "hash.h"
using namespace std;
        
extern  int yylex(void); 
int yyerror(char *err);
bool ParseError=false;

extern long lineno;   /* All defined in readcircuit.l */
extern char* yytext;
extern FILE *yyin;

//temporary objects for processing the circuit file
//vector<GATE*> gatelist;
CIRCUIT Circuit;
unsigned no_pi=0;
unsigned no_po=0;
unsigned no_dff=0;
unsigned no_gate=0;
string name;

GATE* gptr=NULL;
GATE* inlist=NULL;

Hash<string, GATE*, Str_hash_function> NameTable;

%}
%union {
long num;
char str[256];
GATEFUNC func;
}


%token <num> GINPUT GOUTPUT GDFF GNOT GBUF GAND GOR GNAND GNOR
%token EOLINE LPAR RPAR COMMA EQUAL

%token <str> NAMESTRING

%type <str> output
%type <func> gatename


%start netlist

%%
netlist: circuit{
         cout << "Finish reading circuit file\n" ;
}

circuit : 
         input_definition 
         | output_definition 
         | gate_definition 
         | ff_definition 
         | circuit input_definition
         | circuit output_definition
         | circuit gate_definition
         | circuit ff_definition
         | circuit error EOLINE
;

input_definition: GINPUT LPAR NAMESTRING RPAR EOLINE{
                       no_pi++;
                       if(!NameTable.is_member($3)){
                               gptr=new GATE;
                               gptr->SetName($3);
                               gptr->SetFunction(G_PI);
                               NameTable.insert($3,gptr);
                               Circuit.AddGate(gptr);

                       }
                       else{
                               gptr=NameTable.get_value($3);
                               gptr->SetFunction(G_PI);
                       }
                  }
;
        
output_definition: GOUTPUT LPAR NAMESTRING RPAR EOLINE{
                       no_po++;
                       name=string("PO_")+$3;
                       if(!NameTable.is_member(name)){
                               gptr=new GATE;
                               gptr->SetName(name);
                               gptr->SetFunction(G_PO);
                               NameTable.insert(name,gptr);
                               Circuit.AddGate(gptr);
                       }
                       else{
                               gptr=NameTable.get_value(name);
                               gptr->SetFunction(G_PO);
                       }
                       //include the original PO as fanin of PO_
                       if(!NameTable.is_member($3)){
                               inlist=new GATE;
                               inlist->SetName($3);
                               //inlist->SetFunction(G_PO);
                               NameTable.insert($3,inlist);
                               Circuit.AddGate(inlist);
                       }
                       else{
                               inlist=NameTable.get_value($3);
                               //inlist->SetFunction(G_PO);
                       }
                       gptr->AddInput_list(inlist);
                   }
;
        
gate_definition: output EQUAL gatename LPAR{
                       no_gate++;
                       //for output
                       if(!NameTable.is_member($1)){
                               gptr=new GATE;
                               gptr->SetName($1);
                               NameTable.insert($1,gptr);
                               Circuit.AddGate(gptr);
                       }
                       else{
                               gptr=NameTable.get_value($1);
                       }
                       gptr->SetFunction($3);
                 }fanin_list RPAR EOLINE

;

output: NAMESTRING 
;
        
gatename: GNOT{$$=G_NOT;}
         |GAND{$$=G_AND;}
         |GBUF{$$=G_BUF;}
         |GOR{$$=G_OR;}
         |GNAND{$$=G_NAND;}
         |GNOR{$$=G_NOR;}
;

ff_definition: output EQUAL GDFF LPAR NAMESTRING RPAR EOLINE{
                       no_dff++;
                       //PPI
                       if(!NameTable.is_member($1)){
                               gptr=new GATE;
                               gptr->SetName($1);
                               gptr->SetFunction(G_PPI);
                               NameTable.insert($1,gptr);
                               Circuit.AddGate(gptr);
                       }
                       else{
                               gptr=NameTable.get_value($1);
                               gptr->SetFunction(G_PPI);
                       }
                       //Add inlist for PPI
                       name=$5;
                       if(!NameTable.is_member(name)){
                               inlist=new GATE;
                               inlist->SetName(name);
                               NameTable.insert(name,inlist);
                               Circuit.AddGate(inlist);
                       }
                       else{
                               inlist=NameTable.get_value(name);
                       }
                       gptr->AddInput_list(inlist);

                       //PPO
                       name=string("PPO_")+$5;
                       if(!NameTable.is_member(name)){
                               gptr=new GATE;
                               gptr->SetName(name);
                               NameTable.insert(name,gptr);
                               Circuit.AddGate(gptr);
                       }
                       else{
                               gptr=NameTable.get_value(name);
                       }
                       //Add PPO fanin at the first time
                       if (gptr->No_Fanin() == 0) {
                           gptr->SetFunction(G_PPO);
                           gptr->AddInput_list(inlist);
                       }
                 }
;

fanin_list: 
          NAMESTRING{
                       if(!NameTable.is_member($1)){
                               inlist=new GATE;
                               inlist->SetName($1);
                               NameTable.insert($1,inlist);
                               Circuit.AddGate(inlist);
                               gptr->AddInput_list(inlist);
                       }
                       else{
                               inlist=NameTable.get_value($1);
                               gptr->AddInput_list(inlist);
                       }
          } 
          | NAMESTRING {
                       if(!NameTable.is_member($1)){
                               inlist=new GATE;
                               inlist->SetName($1);
                               NameTable.insert($1,inlist);
                               Circuit.AddGate(inlist);
                               gptr->AddInput_list(inlist);
                       }
                       else{
                               inlist=NameTable.get_value($1);
                               gptr->AddInput_list(inlist);
                       }
          } COMMA fanin_list 
;

%%

int yyerror(char *err)
{
        cout << err << endl;
        cout << "\t Parsing error in line " << lineno << " at " << yytext << endl;
        ParseError=true;
        return -1;
}

GATE* NameToGate(string name){
        return NameTable.get_value(name);
}

GATE* CreateBuf(GATE* in, GATE* out){
        GATE* gptr=new GATE;
        gptr->SetFunction(G_BUF);
        gptr->SetName(in->GetName()+out->GetName());
        NameTable.insert(gptr->GetName(),gptr);
        gptr->SetID(Circuit.No_Gate());
        Circuit.AddGate(gptr);
        gptr->AddInput_list(in);
        gptr->AddOutput_list(out);
        //To the same level of fanin
        gptr->SetLevel(in->GetLevel());
        return gptr;
}

