#ifndef READPATTERN_H
#define READPATTERN_H

#include <fstream>
#include "gate.h"
using namespace std;

class PATTERN
{
    private:
        ifstream patterninput;
        vector<GATE*> inlist;
        int no_pi_infile;
    public:
        PATTERN(): no_pi_infile(0){}
        void Initialize(char* InFileName, int no_pi, string TAG);
        //Assign next input pattern to PI
        void ReadNextPattern();
        void ReadNextPattern_t();
	void ReadNextPattern(unsigned idx);
        bool eof()
        {
            return (patterninput.eof());
        }
};
#endif
