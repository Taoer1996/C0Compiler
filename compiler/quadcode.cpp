#include "stdafx.h"
#include "quadcode.h"

string OptName[] = {
	"cst","var","arr","func","para","ret","end","addop","subop","mulop","diviop","move","beq","bne","bge","bgt","ble","blt","j","call","scan","print","larr","sarr","parain","setlab"
};

quadcode::quadcode()
{
	code tmp;		// 首先添加一条 j,	,	,	main; 的四元式
	tmp.opt = j;
	tmp.var1 = "";
	tmp.var2 = "";
	tmp.var3 = "main";
	textSeg.push_back(tmp);
}


quadcode::~quadcode()
{
}

// 将生成的代码存入代码段，同时返回填入的位置（下标）
int quadcode::emitdata(Opt opt, string var1, string var2, string var3)
{
	code tmp;
	tmp.opt = opt;
	tmp.var1 = var1;
	tmp.var2 = var2;
	tmp.var3 = var3;
	dataSeg.push_back(tmp);
	if (DEBUG && QuadDEBUG) {
		cout << "######################## quadcode #########################" << endl;
		cout << OptName[opt] << "\t\t" << var1 << "\t\t" << var2 << "\t\t" << var3 << endl;
		cout << "########################## end ############################" << endl;
	}
	return dataSeg.size() - 1;
}

// 将生成的代码存入代码段，同时返回填入的位置（下标）
int quadcode::emit(Opt opt, string var1, string var2, string var3)
{
	code tmp;
	tmp.opt = opt;
	tmp.var1 = var1;
	tmp.var2 = var2;
	tmp.var3 = var3;
	textSeg.push_back(tmp);
	if (DEBUG && QuadDEBUG) {
		cout << "######################## quadcode #########################" << endl;
		cout << OptName[opt] << "\t\t" << var1 << "\t\t" << var2 << "\t\t" << var3 << endl;
		cout << "########################## end ############################" << endl;
	}
	return textSeg.size() - 1;
}

void quadcode::logCode()
{
	int i = 0;
	code t;
	FILE * code = fopen("./quadcode.txt", "w");
	fprintf(code, "\n################################## data segment ###################################################\n\n");
	for (i = 0; i < int(dataSeg.size()); i++) {
		t = dataSeg[i];
		fprintf(code, "%-20s%-20s%-20s%-20s\n", OptName[t.opt].c_str(), t.var1.c_str(), t.var2.c_str(),t.var3.c_str());
	}
	fprintf(code, "\n################################## text segment ###################################################\n\n");
	for (i = 0; i < int(textSeg.size()); i++) {
		t = textSeg[i];
		fprintf(code, "%-20s%-20s%-20s%-20s\n", OptName[t.opt].c_str(), t.var1.c_str(), t.var2.c_str(),t.var3.c_str());
	}
	fprintf(code, "\n###################################### end ########################################################\n");
	fclose(code);
}
