/*Scanner for reading benchmark circuit netlist*/

%{
#include "typeemu.h"
#include "readcircuit.tab.h"
        
long lineno=1;
%}

WS [ \t]
STRING [_a-zA-z0-9][_a-zA-z0-9\.]*

%option nounput
%option noyywrap
%%
{WS}     { /* Ignore white space*/}
^#[^\n]*\n  { /* Ignore Comment */lineno++; }
^\n {lineno++;}
[\n]  {lineno++; return(EOLINE);}
INPUT {  return(GINPUT);}
OUTPUT {  return(GOUTPUT);}
NOT {  return(GNOT);}
BUF {  return(GBUF);}
AND {  return(GAND);}
OR {  return(GOR);}
NAND {  return(GNAND);}
NOR {  return(GNOR);}
DFF {  return(GDFF);}
{STRING} { 
        strcpy(yylval.str, (char *)yytext);
        return(NAMESTRING);
      }
"=" { return(EQUAL);}
"(" { return(LPAR);}
")" { return(RPAR);}
"," { return(COMMA);}
.  {printf("Unrecognized symbol error at %s in line %ld\n",yytext, lineno);}
%%

/* debuggin
int main(int argc, char ** argv)
{
        ++argv, --argc;
        if(argc>0)
            yyin = fopen(argv[0],"r");
        else
            yyin=stdin;
        while(!feof(yyin)){ 
            yylex();
        }
        fclose(yyin);
}
*/
