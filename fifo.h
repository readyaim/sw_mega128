#include "global.h"

typedef struct Fifo {
    UINT8 fifodata[FifoLength];
    UINT8 start;
    UINT8 end;
    BOOL(*IsEmpty)(struct Fifo*);
    BOOL(*AddFifo)(struct Fifo *, UINT8);
    UINT8(*FetchFifo)(struct Fifo *);
    void(*ClearFifo)(struct Fifo *);
}FIFO;

extern struct Fifo CommandFifo;