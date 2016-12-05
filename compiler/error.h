#pragma once
#include <iostream>
#include <string>
#include <string.h>
#include <vector>
#include <sstream>

using namespace std;

class error
{
public:
	error();
	~error();
	void Err(int code);
	int getErrNum();
private:
	int errnum;
};

