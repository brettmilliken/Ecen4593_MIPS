
#ifndef DEFSTAGE_HPP
#define DEFSTAGE_HPP

extern int gregisters[];
extern int cycle_count;


//extern struct IF_ID ifid;
//extern struct IF_ID shadifid;
//extern struct ID_EX idex;
//extern struct ID_EX shadidex;
//extern struct EX_MEM exmem;
//extern struct EX_MEM shadexmem;
//extern struct MEM_WB memwb;
//extern struct MEM_WB shadmemwb;

void stall(void);
void setcontrol(void);
void IF(void);
void ID(void);
void EX(void);
void MEM(void);
void WB(void);
void shadToNorm(int *iCount);


#endif
