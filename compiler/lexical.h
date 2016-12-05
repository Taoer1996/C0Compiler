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

#define ASCIILen 128			// ASCII�����ַ�����
#define ReservedWords 14        // �����ָ���
#define MaxLineLen 1000			// ÿ��������ַ���
#define MaxNumLen 9				// �������λ��
#define MaxNumVal 2147483647	// �������ֵ�ֵ

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
	FILE * log;				// log�ļ���ָ�룬���ڵ���
};

