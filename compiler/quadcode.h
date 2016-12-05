#pragma once
#include "stdafx.h"

enum Opt {
	cst,var,arr,func,para,ret,end,addop,subop,mulop,diviop,move,beq,bne,bge,bgt,ble,blt,j,call,scan,print,larr,sarr,parain,setlab
};


typedef struct quad{
	Opt opt;
	string var1;
	string var2;
	string var3;
}code;

class quadcode
{
public:
	quadcode();
	~quadcode();
	int emitdata(Opt,string,string,string);
	int emit(Opt, string, string, string);
	void logCode();
public:
	vector<code> dataSeg;
	vector<code> textSeg;
};

