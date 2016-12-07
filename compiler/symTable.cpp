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

// 从下往上寻找，按照名字和指定lev进行寻找
int symTable::locate(string name, int level) {				
	int i = 0;
	for (i = SymbolTable.size() - 1; i >= 0; i--) {
		if (SymbolTable[i].name == name && SymbolTable[i].level == level) {
			return i;
		}
	}
	return -1;

}

// 纯粹按照名字来定位
int symTable::locate(string name) {
	int i = 0;
	for (i = SymbolTable.size() - 1; i >= 0; i--) {
		if (SymbolTable[i].name == name) {
			return i;
		}
	}
	return -1;
}

void symTable::PrintTable()
{
	int  i = 0;
	TableColumn t;
	cout << "\n########################## 符号表内容 #########################" << endl;
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


// 清理符号表，因为每进入一次函数调用或者是函数定义，就会有局部变量存入其中
void symTable::clean() {
	int i = 0;
	kindEnum j;
	int curNum = SymbolTable.size() - 1;
	for (i = curNum; i >= 0; i--) {
		j = SymbolTable[i].kind;
		if (j == varkind || j == cstkind || j == arrkind || j == tmpkind) {
			SymbolTable.pop_back();
		}
		else if(j == parakind){				// 将参数的名字抹去
			SymbolTable[i].name = "";
		}
		else if(j == funckind){
			break;
		}
	}
}

// 清空符号表
void symTable::cleanAll() {
	SymbolTable.clear();
}