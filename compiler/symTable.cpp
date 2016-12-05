#include "stdafx.h"
#include "symTable.h"

extern error ERR;

string kindEnumName[] = {
	"cstkind","varkind","arrkind","funckind","parakind","tmpkind"
};
string typeEnumName[] = {
	"Int","Char","Void","String"
};


symTable::symTable()
{
}


symTable::~symTable()
{
}

int symTable::enter(string name,kindEnum kind,typeEnum type,int level,int arrsize,int address,int paranum,int value)
{
	int i = 0;
	tableColumn tmp;
	for (i = 0; i < int(SymbolTable.size()); i++) {
		if (name == SymbolTable[i].name && level == SymbolTable[i].level) {
			ERR.Err(36);
			return -1;
		}
	}
	tmp.address = address;
	tmp.arrsize = arrsize;
	tmp.kind = kind;
	tmp.level = level;
	tmp.name = name;
	tmp.paranum = paranum;
	tmp.type = type;
	tmp.value = value;
	SymbolTable.push_back(tmp);
	if (DEBUG && TableDEBUG) {
		cout << "@@@@@@@@@@@@@@@@ ENTER TABLE @@@@@@@@@@@@@@@@@@@@@@" << endl;
		cout << name << " - " << kindEnumName[kind] << " - " << typeEnumName[type] << " - " << level << " - " << arrsize << " - " << address << " - " << paranum << " - " << value << endl;
		cout << "@@@@@@@@@@@@@@@@@@@@@ END @@@@@@@@@@@@@@@@@@@@@@@@@" << endl;
	}
	return SymbolTable.size() - 1;
}

// �ҵ��˷����±꣬û���ҵ�����-1��ע���Ǵ���������
int symTable::locate(string name, int level) {				
	int i = 0;
	// �������lev == 0 ��������⴦������Ϊlev == 0�൱����ȫ�ֱ���
	for (i = SymbolTable.size() - 1; i >= 0; i--) {
		if (SymbolTable[i].name == name && (SymbolTable[i].level == level || SymbolTable[i].level == 0)) {
			return i;
		}
	}
	return -1;

}

void symTable::PrintTable()
{
	int  i = 0;
	TableColumn t;
	cout << "\n########################## ���ű����� #########################" << endl;
	for (i = 0; i < int(SymbolTable.size()); i++) {
		t = SymbolTable[i];
		cout << setiosflags(ios::left) << setw(18) << t.name << " ";
		cout << setw(13) << kindEnumName[t.kind] << " ";
		cout << setw(8) << typeEnumName[t.type] ;
		cout << " | ";
		cout << setw(2) << t.level;
		cout << " | ";
		cout << t.arrsize;
		cout << " | ";
		cout << t.address;
		cout << " | ";
		cout << t.paranum;
		cout << " | ";
		cout << setw(3) << t.value << endl;
	}
	cout << "############################### END ############################" << endl;
}


// ������ű���Ϊÿ����һ�κ������û����Ǻ������壬�ͻ��оֲ�������������
void symTable::clean() {
	int i = 0;
	kindEnum j;
	int curNum = SymbolTable.size() - 1;
	for (i = curNum; i >= 0; i--) {
		j = SymbolTable[i].kind;
		if (j == varkind || j == cstkind || j == arrkind || j == tmpkind) {
			SymbolTable.pop_back();
		}
		else if(j == parakind){				// ����level����Ϊ-1���൱���ǽ�������Ĩȥ
			SymbolTable[i].level = -1;
		}
		else if(j == funckind){
			break;
		}
	}
}

// ��շ��ű�
void symTable::cleanAll() {
	SymbolTable.clear();
}