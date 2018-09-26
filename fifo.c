#include "global.h"
//start: point to 1st valid data
//end: point after last valid data


BOOL IsEmpty(struct Fifo *this)
{
    return (this->start == this->end);
}

BOOL AddFifo(struct Fifo *this, UINT8 data)
{
    if ((this->end == FifoLength - 1) & (this->start == 0)) return false;
    if (this->end == this->start - 1) return false;
    this->fifodata[this->end] = data;
    if ((this->end) < (FifoLength - 1)) {
        this->end++;
    }
    else this->end = 0;
    return true;
}

UINT8 FetchFifo(struct Fifo *this)
{
    UINT8 temp;
    temp = this->fifodata[this->start];
    if ((this->start) < (FifoLength - 1)) this->start++;
    else this->start = 0;
    return temp;
}

void ClearFifo(struct Fifo *this)
{
    this->start = 0;
    this->end = 0;
    return;
}