#ifndef BRIDGING_FAULT_H
#define BRIDGING_FAULT_H
#include "gate.h"

class BRIDGING_FAULT
{
    private:
				VALUE Value;
        FAULT_TYPE Type;
        GATE* Input1;
        GATE* Input2;
        //if stem, Input = Output
        unsigned EqvFaultNum; //equivalent fault number (includes itself)
        FAULT_STATUS Status;
    public:
        BRIDGING_FAULT(GATE* gptr1, GATE* gptr2, FAULT_TYPE type, VALUE value): 
				Type(type), Input1(gptr1), Input2(gptr2), Value(value), EqvFaultNum(1), 
				Status(UNKNOWN) {}
        ~BRIDGING_FAULT() {}
        FAULT_TYPE GetType() { return Type; }
				VALUE GetValue() { return Value; }
        GATE* GetInput1Gate() { return Input1; }
        GATE* GetInput2Gate() { return Input2; }
        void SetEqvFaultNum(unsigned n) { EqvFaultNum = n; }
        void IncEqvFaultNum() { ++EqvFaultNum; }
        unsigned GetEqvFaultNum() { return EqvFaultNum; }
        void SetStatus(FAULT_STATUS status) { Status = status; }
        FAULT_STATUS GetStatus() { return Status; }
};
#endif
