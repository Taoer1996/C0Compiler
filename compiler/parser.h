#pragma once
#include "stdafx.h"
#include "lexical.h"
#include "symTable.h"
#include "quadcode.h"
#include "myutils.h"


extern error ERR;						// 错误处理类对象
extern lexical lex;						// 初始化词法分析类对象
extern symTable symtab;					// 初始话符号表
extern quadcode codetab;					// 初始化代码表
extern Symbol sym;						// 当前单词的类型
extern char ch;							// 当前的字符，并且必须初始化为空格
extern char line[MaxLineLen];			// 当前行，最长为MaxLineLen-1
extern string token;					// 当前单词，string类型
extern string str;						// 存储printf中的字符串使用
extern string msg;						// 错误信息，临时使用
extern int num;							// 存储数字或者是字符的值
extern int CharCount;					// 当前行中的指针
extern int LineLen;						// 当前行的长度
extern int LineNum;						// 当前行号（源文件中行数）

class parser
{
public:
	parser();
	~parser();
	void program();
	void constDeclaration(bool isData);
	void constDefine(bool isData);
	void varDeclaration();
	void varDefine();
	int paraList();
	int integer();           //整数处理程序，获取整数的值
	void compoundStat();
	void statList();
	string expression();
	string terms();
	string factor();
	void statement();
	void ifStatement();
	code condition();
	void loopStatement();
	void switchStatement();
	void caseDescription(string selectVar, string endLabel);
	int caseStatement(string selectVar, string endLabel, int lastIndex);
	int valList(string fName);
	void readStatement();
	void writeStatement();
	void returnStatement();
	
	string nextTmpVar();
	string nextLab();
	string nextStrCstName();
	code oppbranchcode(code tmp);				// 将分支语句中的branch条件反转

	string int2str(int num);
	int str2int(string str);

	bool errjump(set<Symbol> symset);
private:
	int tmpvarcode;
	int tmplabcode;
	int tmpstrcode;
};

