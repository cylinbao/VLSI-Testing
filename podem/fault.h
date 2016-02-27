#ifndef FAULT_H
#define FAULT_H
#include "gate.h"

class FAULT
{
    private:
        VALUE Value;
        GATE* Input;
        GATE* Output; //record output gate for branch fault
        //if stem, Input = Output
        bool Branch; //fault is on branch
        unsigned EqvFaultNum; //equivalent fault number (includes itself)
        FAULT_STATUS Status;
    public:
        FAULT(GATE* gptr, GATE* ogptr, VALUE value): Value(value), Input(gptr),
        Output(ogptr), Branch(false), EqvFaultNum(1), Status(UNKNOWN) {}
        ~FAULT() {}
        VALUE GetValue() { return Value; }
        GATE* GetInputGate() { return Input; }
        GATE* GetOutputGate() { return Output; }
        void SetBranch(bool b) { Branch = b; }
        bool Is_Branch() { return Branch; }
        void SetEqvFaultNum(unsigned n) { EqvFaultNum = n; }
        void IncEqvFaultNum() { ++EqvFaultNum; }
        unsigned GetEqvFaultNum() { return EqvFaultNum; }
        void SetStatus(FAULT_STATUS status) { Status = status; }
        FAULT_STATUS GetStatus() { return Status; }
};



#endif
