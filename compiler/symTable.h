#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <iomanip>

using namespace std;

enum kindEnum {          // ���ű��е�kindȡֵ��tmp������ʱ����
	cstkind, varkind, arrkind, funckind, parakind, tmpkind
};

enum typeEnum {          // ���ű��е�typeȡֵ
	Int, Char, Void, StrType
};

typedef struct TableColumn {        // ��ʶ�������ݽṹ
	string name;			// ÿ������֣�������ʶ�������������顢�����������ȣ�
	kindEnum kind;          // ��ʶ�������࣬�糣�������顢����
	typeEnum type;          // ��ʶ�������ͣ���Int��Char��Void
	int level;              // ��ʶ�����ڵĲ��
	int arrsize;            // �������ı�ʶ�������飬����������ĳ��ȣ�������
	int address;            // �����ʶ��Ϊ�����������뺯�������λ��
	int paranum;            // �����Ǻ����Ĳ�������������ֻ��int��char���ͣ����Բ�����������ʵ���Ǻ���Ҫ
	int value;              // �����ʶ��Ϊ���������������볣����ֵ�������char��������ASCII��ֵ��
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

