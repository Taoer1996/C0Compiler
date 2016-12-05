#pragma once
#include "quadcode.h"

typedef struct mips{
	string label;
	string op;
	string var1;
	string var2;
	string var3;
}mipscode;


class quad2mips
{
public:
	quad2mips();
	~quad2mips();
	void translate();
	void data2mips();
	void text2mips();
	void func2mips();
	void statement2mips(code t);
	void loadIdent(string dst, string src);
	void loadIdent(string dst, string src, bool forceChar);
	void saveIdent(string src, string dst);
	void saveIdent(string src, string dst,bool forceChar);

	int FindVarNum(int index);
	int enterCode(string, string, string, string, string);
	void Print2File(mipscode c);

private:
	FILE * res;
};

