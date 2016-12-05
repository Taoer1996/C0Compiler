#pragma once
#include "stdafx.h"
#include <iostream>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <string.h>
#include <vector>
#include <sstream>

#define ASCIILen 128			// ASCII表中字符个数
#define ReservedWords 14        // 保留字个数
#define MaxLineLen 1000			// 每行最长包含字符数
#define MaxNumLen 9				// 数字最长的位数
#define MaxNumVal 2147483647	// 最大的数字的值

using namespace std;

typedef enum symbol {
	intsy, charsy, voidsy, intcon, charcon, stringcon, mainsy, constsy,
	ifsy, elsesy, whilesy, switchsy, casesy, defaultsy, scanfsy, printfsy, returnsy,
	add, sub, mul, divi, lt, gt, lte, gte, eql, neq, becomes,
	comma, semicolon, colon, lparent, rparent, lbrack, rbrack, lbrace, rbrace, excl, ident, nul, errorsy, eofsy
}Symbol;


class lexical
{
public:
	lexical();
	~lexical();
	void Initial();
	int getsym();
	int getch();
	int isLetter(char c);
	int isDigit(char c);
	void PrintInfo();
private:
	FILE * log;				// log文件的指针，用于调试
};

