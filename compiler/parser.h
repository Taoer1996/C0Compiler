#pragma once
#include "stdafx.h"
#include "lexical.h"
#include "symTable.h"
#include "quadcode.h"
#include "myutils.h"


extern error ERR;						// �����������
extern lexical lex;						// ��ʼ���ʷ����������
extern symTable symtab;					// ��ʼ�����ű�
extern quadcode codetab;					// ��ʼ�������
extern Symbol sym;						// ��ǰ���ʵ�����
extern char ch;							// ��ǰ���ַ������ұ����ʼ��Ϊ�ո�
extern char line[MaxLineLen];			// ��ǰ�У��ΪMaxLineLen-1
extern string token;					// ��ǰ���ʣ�string����
extern string str;						// �洢printf�е��ַ���ʹ��
extern string msg;						// ������Ϣ����ʱʹ��
extern int num;							// �洢���ֻ������ַ���ֵ
extern int CharCount;					// ��ǰ���е�ָ��
extern int LineLen;						// ��ǰ�еĳ���
extern int LineNum;						// ��ǰ�кţ�Դ�ļ���������

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
	int integer();           //����������򣬻�ȡ������ֵ
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
	code oppbranchcode(code tmp);				// ����֧����е�branch������ת

	string int2str(int num);
	int str2int(string str);

	bool errjump(set<Symbol> symset);
private:
	int tmpvarcode;
	int tmplabcode;
	int tmpstrcode;
};

