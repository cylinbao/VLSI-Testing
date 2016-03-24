#ifndef PATTERN_H
#define PATTERN_H

#include <fstream>
#include <stdlib.h>
#include "gate.h"

#define xstr(s) str(s)                                                           
#define str(s) #s                                                                
#define INDIR input

using namespace std;

class PATTERN
{
    private:
        ifstream patterninput;
        vector<GATE*> inlist;
        int no_pi_infile;
				ofstream ofs;
    public:
        PATTERN(): no_pi_infile(0){}
				~PATTERN(){
					if(ofs.is_open())
						ofs.close();
				}
        void Initialize(char* InFileName, int no_pi, string TAG);
        //Assign next input pattern to PI
        void ReadNextPattern();
        void ReadNextPattern_t();
				void ReadNextPattern(unsigned idx);
        bool eof()
        {
            return (patterninput.eof());
        }
				void setupInput(string pattern_name);
				void genRandomPattern(string pattern_name, int pattern_number);
				void genRandomPatternUnknown(string pattern_name, int pattern_number);
};
#endif
