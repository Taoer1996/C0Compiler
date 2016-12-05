#pragma once
class myutils
{
public:
	myutils();
	~myutils();
	string int2str(int);
	int str2int(string);
	bool isInt(string);
	bool isIdent(string);
	bool isTmpVar(string);
};

