#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <iomanip>

using namespace std;

enum kindEnum {          // 符号表中的kind取值，tmp代表临时变量
	cstkind, varkind, arrkind, funckind, parakind, tmpkind
};

enum typeEnum {          // 符号表中的type取值
	Int, Char, Void, StrType
};

typedef struct TableColumn {        // 标识符的数据结构
	string name;			// 每项的名字（包括标识符、常量、数组、变量、函数等）
	kindEnum kind;          // 标识符的种类，如常量、数组、变量
	typeEnum type;          // 标识符的类型，如Int，Char，Void
	int level;              // 标识符所在的层次
	int arrsize;            // 如果这里的标识符是数组，则填入数组的长度，否则不填
	int address;            // 如果标识符为函数，则填入函数的入口位置
	int paranum;            // 这里是函数的参数个数，由于只有int、char类型，所以参数的种类其实不是很重要
	int value;              // 如果标识符为常量，则这里填入常量的值（如果是char类型填其ASCII码值）
}tableColumn;


class symTable
{
public:
	symTable();
	~symTable();
	int enter(string name, kindEnum kind, typeEnum type, int level, int arrsize, int address, int paranum, int value);
	void clean();
	void cleanAll();
	int locate(string name, int level);
	int locate(string name);
	void PrintTable();
public:
	vector<tableColumn> SymbolTable;
};

