#ifndef BRIDGING_FAULT_H
#define BRIDGING_FAULT_H
#include "gate.h"

class BRIDGING_FAULT
{
    private:
        FAULT_TYPE Type;
        GATE* Input;
        GATE* Output; //record output gate for branch fault
        //if stem, Input = Output
        unsigned EqvFaultNum; //equivalent fault number (includes itself)
        FAULT_STATUS Status;
    public:
        BRIDGING_FAULT(GATE* gptr, GATE* ogptr, FAULT_TYPE type): Type(type), Input(gptr),
        Output(ogptr), EqvFaultNum(1), Status(UNKNOWN) {}
        ~BRIDGING_FAULT() {}
        FAULT_TYPE GetType() { return Type; }
        GATE* GetInputGate() { return Input; }
        GATE* GetOutputGate() { return Output; }
        void SetEqvFaultNum(unsigned n) { EqvFaultNum = n; }
        void IncEqvFaultNum() { ++EqvFaultNum; }
        unsigned GetEqvFaultNum() { return EqvFaultNum; }
        void SetStatus(FAULT_STATUS status) { Status = status; }
        FAULT_STATUS GetStatus() { return Status; }
};
#endif
