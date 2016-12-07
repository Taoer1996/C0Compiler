#include "stdafx.h"
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include "error.h"
#include "lexical.h"
#include "symTable.h"
#include "quadcode.h"
#include "parser.h"
#include "quad2mips.h"

using namespace std;

FILE * fin;			// 源文件指针
error ERR;			// 初始化错误类对象
lexical lex;		// 初始化词法分析类对象
symTable symtab;	// 初始化符号表
quadcode codetab;	// 初始化代码表
quad2mips transcode;

int _tmain(int argc, _TCHAR* argv[])
{
	string path ;
	parser myparser;

	cout << "请输入文件路径（默认为./test.txt)：" << endl;
	getline(cin, path);
	if (path == "") {
		path = "./test5.txt";
	}
	fin = fopen(path.c_str(), "r");
	
	try
	{
		myparser.program();
	}
	catch (const std::exception& e)
	{
		cout << "Standard exception - " << e.what() << endl;
		cout << "代码中部分错误如上！" << endl;
		return 0;
	}

	cout << "出现 " << ERR.getErrNum() << " 个错误！" << endl;

	if (ERR.getErrNum() == 0) {
		cout << "四元式翻译完成" << endl;
		transcode.translate();
		cout << "目标代码成功生成！" << endl;
	}
	return 0;
	
}