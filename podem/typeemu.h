// Main data structure
#include <string>
#include <vector>
#include <list>

enum VALUE {S0, S1, X, D, B, ILLIGAL};
//good/faulty D = 1/0, B = D' = 0/1
enum GATEFUNC {G_PI, G_PO, G_PPI, G_PPO, G_NOT, G_AND, G_NAND, G_OR, G_NOR, G_DFF, G_BUF, G_BAD };
enum FLAGS { ALL_ASSIGNED, MARKED, FAULT_INJECTED, FAULTY, SCHEDULED, OUTPUT, NumFlags };
enum ATPG_STATUS { TRUE, CONFLICT, FALSE };
enum FAULT_STATUS { UNKNOWN, DETECTED, REDUNDANT, ABORT };

//Controlling Values
const VALUE CV[] = {X, X, X, X, X, S0, S0, S1, S1, X, X, X};
//Noncontrolling Values
const VALUE NCV[] = {X, X, X, X, X, S1, S1, S0, S0, X, X, X};

const unsigned PatternNum = 16;

const VALUE NotTable[] = {S1, S0, X, B, D, ILLIGAL};

const VALUE AndTable[5][5] =  { { S0,  S0,  S0,  S0,  S0 },
    { S0, S1, X, D,  B },
    { S0, X,  X, X,  X },
    { S0, D,  X, D,  S0 },
    { S0, B,  X, S0, B } };

const VALUE OrTable[5][5]  =  { { S0,  S1,  X,  D,  B },
    { S1, S1, S1, S1, S1 },
    { X,  S1, X,  X,  X },
    { D,  S1, X,  D,  S1 },
    { B,  S1, X,  S1, B } };

const VALUE XorTable[5][5] =  { { S0,  S1,  X,  D,  B },
    { S1, S0, X,  B,  D },
    { X,  X,  X,  X,  X },
    { D,  B,  X,  S0, S1 },
    { B,  D,  X,  S1, S0 } };

const VALUE EqvTable[5][5] =  { { S1,  S0,  X,  B,  D },
    { S0, S1, X, D,  B },
    { X,  X,  X, X,  X },
    { B,  D,  X, S1, S0 },
    { D,  B,  X, S0, S1 } };

