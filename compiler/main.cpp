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

FILE * fin;			// Դ�ļ�ָ��
error ERR;			// ��ʼ�����������
lexical lex;		// ��ʼ���ʷ����������
symTable symtab;	// ��ʼ�����ű�
quadcode codetab;	// ��ʼ�������
quad2mips transcode;

int _tmain(int argc, _TCHAR* argv[])
{
	string path ;
	parser myparser;

	cout << "�������ļ�·����Ĭ��Ϊ./test.txt)��" << endl;
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
		cout << "�����в��ִ������ϣ�" << endl;
		return 0;
	}

	cout << "���� " << ERR.getErrNum() << " ������" << endl;

	if (ERR.getErrNum() == 0) {
		cout << "��Ԫʽ�������" << endl;
		transcode.translate();
		cout << "Ŀ�����ɹ����ɣ�" << endl;
	}
	return 0;
	
}