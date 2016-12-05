#include "stdafx.h"
#include "quad2mips.h"
#include "symTable.h"
#include "quadcode.h"

extern symTable symtab;
extern quadcode codetab;
extern myutils util;
extern string OptName[];

vector<mipscode> asmcode;
int dataCodeIndex = 0;
int textCodeIndex = 0;
int dataOffset = 0;

quad2mips::quad2mips()
{
	quad2mips::res = fopen("./asm.txt", "w");
	
}


quad2mips::~quad2mips()
{
	fclose(res);
}

void quad2mips::translate()
{
	int  i = 0;
	mipscode tmpcode;
	// ������շ��ű�
	symtab.cleanAll();

	// �տ�ʼ�Ǵ���ȫ�����ݶ�
	fprintf(res, "\n########################### data segment ######################\n");
	fprintf(res, ".data\n");
	
	while(dataCodeIndex < int(codetab.dataSeg.size())){
		data2mips();
	}

	fprintf(res, "\n########################### text segment ######################\n");
	fprintf(res, ".text\n");
	while(textCodeIndex < int(codetab.textSeg.size())){
		text2mips();
	}
	fprintf(res, "\n############################### end ###########################\n");
}

void quad2mips::data2mips()
{
	int arraysize;
	code c = codetab.dataSeg[dataCodeIndex++];
	
	// cst		int		a			10
	// cst		char	ch			67
	// cst		str		STRCST0		"TEST!"
	if (c.opt == Opt::cst) {
		if (c.var1 == "int") {
			symtab.enter(c.var2,kindEnum::cstkind,typeEnum::Int,0,0,0,0,util.str2int(c.var3));
			enterCode("", c.var2 + ":", ".word", c.var3, "");
		}
		// Ϊ��ά��4�ֽڶ��룬����ֱ�Ӷ�����һ��word��֮����ݷ��ű��ж��Ƿ�Ϊchar
		else if (c.var1 == "char") {
			symtab.enter(c.var2, kindEnum::cstkind, typeEnum::Char, 0, 0, 0, 0, util.str2int(c.var3));
			enterCode("", c.var2 + ":", ".word", c.var3, "");
		}
		else if(c.var1 == "str"){
			symtab.enter(c.var2, kindEnum::cstkind, typeEnum::StrType, 0, 0, 0, 0, 0);
			enterCode("", c.var2 + ":", ".asciiz", c.var3, "");
		}
	}
	// var		int		x	
	// var		char	ch
	else if (c.opt == Opt::var) {
		// ȫ�ֱ�����ʼ��Ϊ0
		if (c.var1 == "int") {
			symtab.enter(c.var2, kindEnum::varkind, typeEnum::Int, 0, 0, 0, 0, 0);
			enterCode("", c.var2 + ":", ".word", "0", "");
		}
		else if (c.var1 == "char") {
			symtab.enter(c.var2, kindEnum::varkind, typeEnum::Char, 0, 0, 0, 0, 0);
			enterCode("", c.var2 + ":", ".word", "0", "");
		}
	}
	// arr		int		num		100 
	// arr		char	s		100
	else if (c.opt == Opt::arr) {
		arraysize = util.str2int(c.var3);
		if (c.var1 == "int") {
			symtab.enter(c.var2, kindEnum::arrkind, typeEnum::Int, 0, arraysize, 0, 0, 0);
			enterCode("", c.var2 + ":", ".space", util.int2str(4*arraysize), "");
		}
		else if (c.var1 == "char") {
			symtab.enter(c.var2, kindEnum::arrkind, typeEnum::Char, 0, arraysize, 0, 0, 0);
			enterCode("", c.var2 + ":", ".space", util.int2str(4*arraysize), "");
		}
	}

}

void quad2mips::text2mips()
{
	// ��text���ֵ�codeֻ�������������ͷ��һ����jָ�һ����funcָ��
	// j	��	��	label
	code c = codetab.textSeg[textCodeIndex++];
	if (c.opt == Opt::j) {
		enterCode("", "j", c.var3, "", "");
	}
	else if (c.opt == Opt::func) {
		textCodeIndex--;			// ���ˣ���funcָ��Ҳ�Ÿ�func2mips����
		func2mips();
	}
}

// op		����,		��,		��������
// func		int,		f1,		0
// func		char,		f2,		1
// func		void,		f3,		2
void quad2mips::func2mips()
{
	code c;
	typeEnum type;
	int paNum = 0;
	string tmpFuncName;

	int i = 0, j = 0;
	int varNum = 0;
	int num = 0;
	
	c = codetab.textSeg[textCodeIndex++];
	if (c.opt == Opt::func) {
		// ��������ű�
		tmpFuncName = c.var2;
		paNum = util.str2int(c.var3);
		type = (c.var1 == "int") ? typeEnum::Int : (c.var1 == "char") ? typeEnum::Char : typeEnum::Void;
		symtab.enter(c.var2, kindEnum::funckind, type, 0, 0, 0, paNum, 0);
		enterCode(c.var2 + ":", "", "", "", "");			// ���ñ�ǩ

		dataOffset = 2;		// ���뺯��֮�����Ҫ��ʼ����ǰջ�ڴ���������2Ϊ$fp��$ra
							
		num = paNum - 1 + 2;
		enterCode("", "addi", "$fp", "$sp", util.int2str(4 * num));		// ���������뺯������֮��Ÿı�$fp
		enterCode("", "sw", "$ra", "$fp", "-4");		// ��jal֮��Ž�$ra����
		
		
		// ����洢�ռ䣨��Ϊ�ں��������õ�ʱ���Ѿ��������$fp��$ra��paras���֣����Խ���Ҫ�ٷ���local��tmp�Ĳ��֣�
		// ͬʱ�ں��������õ�ʱ��$fp��$sp���Ѿ������ú���
		varNum = FindVarNum(textCodeIndex);			// �����varNumָ���Ǿֲ���������������ʱ����������
		if (varNum != 0) {
			enterCode("", "addi", "$sp", "$sp", util.int2str(-1 * 4 * varNum));
		}

		// �������
		// para		int,	a
		// para		char,	ch
		c = codetab.textSeg[textCodeIndex++];
		while (c.opt == Opt::para) {
			type = (c.var1 == "int") ? typeEnum::Int : typeEnum::Char;
			symtab.enter(c.var2, kindEnum::parakind, type, 1, 0, dataOffset++, 0, 0);
			c = codetab.textSeg[textCodeIndex++];
		}
		
		// ����ֲ������ͱ���
		// cst	int,	a,	10
		// cst	char,	ch,	67
		// var	int,	x
		// var	char,	c
		// arr	int,	a,	100
		// arr	char,	s,	100
		while (c.opt == Opt::cst || c.opt == Opt::var || c.opt == Opt::arr) {
			type = (c.var1 == "int") ? typeEnum::Int : typeEnum::Char;
			if (c.opt == Opt::cst) {
				symtab.enter(c.var2, kindEnum::cstkind, type, 1, 0, dataOffset++, 0, util.str2int(c.var3));
			}
			else if (c.opt == Opt::var) {
				symtab.enter(c.var2, kindEnum::varkind, type, 1, 0, dataOffset++, 0, 0);
			}
			else {
				symtab.enter(c.var2, kindEnum::varkind, type, 1, util.str2int(c.var3), dataOffset, 0, 0);
				dataOffset = dataOffset + util.str2int(c.var3);
			}
			c = codetab.textSeg[textCodeIndex++];
		}

		// Ϊ��ʱ��������ű�
		// varNum =  local + tmp  
		// local = dataOffset - 2 - paNum;
		i = 0;
		j = varNum - dataOffset + 2 + paNum;
		
		while(i < j){
			symtab.enter("$t"+util.int2str(i),kindEnum::tmpkind,typeEnum::Int,1,0,dataOffset++,0,0);
			i++;
		}

		// ����������
		while (c.opt != Opt::end) {
			statement2mips(c);
			c = codetab.textSeg[textCodeIndex++];
		}

		// ������һ������֮��������ű��ָ�����ָ�룬return����Ѿ�������ֵ�������
		// end		func,		getpower
		if (c.opt == Opt::end) {
			// �ָ�����ָ�룬��ȡ��$ra��ͬʱ����ԭ����λ��
			// lw	$ra,	4($fp)
			// $sp = $fp + 4;	
			// lw	$fp,	0($fp);
			// jr	$ra
			if (tmpFuncName == "main") {
				enterCode("", "li", "$v0", "10", "");
				enterCode("", "syscall", "", "", "");
			}
			else {
				enterCode("", "lw", "$ra", "$fp", "-4");
				enterCode("", "add", "$sp", "$fp", "4");
				enterCode("", "lw", "$fp", "$fp", "0");
				enterCode("", "jr", "$ra", "", "");
			}
			symtab.clean();
		}
	}
}

void quad2mips::statement2mips(code t)
{
	code s;
	int i = 0;
	int num = 0;
	int index = 0;
	int tmpaddress = 0;
	int paNum = 0;

	// addop��var1��Ϊ��ʱ����
	// addop	$t0,	a,		b
	// addop	$t0,	a,		10
	// addop	$t0,	10,		b
	// addop	$t0,	10,		20
	if (t.opt == Opt::addop) {
		// �ڶ���������Ϊ����
		if (util.isInt(t.var2)) {
			if (util.isInt(t.var3)) {
				num = util.str2int(t.var2) + util.str2int(t.var3);		// ������������ֱ��תΪ��ֵ����
				enterCode("", "li", "$s0", util.int2str(num), "");		
				saveIdent("$s0", t.var1);
			}
			else {
				loadIdent("$s0", t.var3);
				enterCode("", "addi", "$s0", "$s0", t.var2);
				saveIdent("$s0", t.var1);
			}
		}
		else {
			if (util.isInt(t.var3)) {
				loadIdent("$s0", t.var2);
				enterCode("", "add", "$s0", "$s0", t.var3);
				saveIdent("$s0", t.var1);
			}
			else {
				loadIdent("$s0", t.var2);
				loadIdent("$s1", t.var3);
				enterCode("", "add", "$s0", "$s0", "$s1");
				saveIdent("$s0", t.var1);
			}
		}
	}
	// ������ͬ��
	else if(t.opt == Opt::subop){

		if (util.isInt(t.var2)) {
			if (util.isInt(t.var3)) {
				num = util.str2int(t.var2) - util.str2int(t.var3);
				enterCode("", "li", "$s0", util.int2str(num), "");
				saveIdent("$s0", t.var1);
			}
			else {
				enterCode("", "li", "$s0", t.var2,"");
				loadIdent("$s1", t.var3);
				enterCode("", "sub", "$s0", "$s0", "$s1");
				saveIdent("$s0", t.var1);
			}
		}
		else {
			if (util.isInt(t.var3)) {
				loadIdent("$s0", t.var2);
				enterCode("", "sub", "$s0", "$s0", t.var3);
				saveIdent("$s0", t.var1);
			}
			else {
				loadIdent("$s0", t.var2);
				loadIdent("$s1", t.var3);
				enterCode("", "sub", "$s0", "$s0", "$s1");
				saveIdent("$s0", t.var1);
			}
		}
	}
	// �˲���ͬ��
	else if (t.opt == Opt::mulop) {

		if (util.isInt(t.var2)) {
			if (util.isInt(t.var3)) {
				num = util.str2int(t.var2) * util.str2int(t.var3);
				enterCode("", "li", "$s0", util.int2str(num), "");
				saveIdent("$s0", t.var1);
			}
			else {
				loadIdent("$s0", t.var3);
				enterCode("", "mul", "$s0", "$s0", t.var2);
				saveIdent("$s0", t.var1);
			}
		}
		else {
			if (util.isInt(t.var3)) {
				loadIdent("$s0", t.var2);
				enterCode("", "mul", "$s0", "$s0", t.var3);
				saveIdent("$s0", t.var1);
			}
			else {
				loadIdent("$s0", t.var2);
				loadIdent("$s1", t.var3);
				enterCode("", "mul", "$s0", "$s0", "$s1");
				saveIdent("$s0", t.var1);
			}
		}
	}
	// ������ͬ��
	else if (t.opt == Opt::diviop) {

		if (util.isInt(t.var2)) {
			if (util.isInt(t.var3)) {
				num = util.str2int(t.var2) / util.str2int(t.var3);
				enterCode("", "li", "$s0", util.int2str(num), "");
				saveIdent("$s0", t.var1);
			}
			else {
				enterCode("", "li", "$s0", t.var2, "");
				loadIdent("$s1", t.var3);
				enterCode("", "div", "$s0", "$s0", "$s1");
				saveIdent("$s0", t.var1);
			}
		}
		else {
			if (util.isInt(t.var3)) {
				loadIdent("$s0", t.var2);
				enterCode("", "div", "$s0", "$s0", t.var3);
				saveIdent("$s0", t.var1);
			}
			else {
				loadIdent("$s0", t.var2);
				loadIdent("$s1", t.var3);
				enterCode("", "div", "$s0", "$s0", "$s1");
				saveIdent("$s0", t.var1);
			}
		}
	}
	// �����move������var1��Ϊ����
	// move		a,		10
	// move		a,		b
	// move		a,		$t0   
	else if (t.opt == Opt::move) {
		if (util.isInt(t.var2)) {
			enterCode("", "li", "$s0", t.var2, "");
			saveIdent("$s0", t.var1);
		}
		else {
			loadIdent("$s0", t.var2);
			saveIdent("$s0", t.var1);
		}
	}
	// setlab	,		,		label	
	else if (t.opt == Opt::setlab) {
		enterCode(t.var3 + ":", "", "", "", "");
	}
	// bne		$t0,	$t1,	label
	// bne		$t0,	10,		label
	// bne		10,		20		label
	// bne		$t0,	a,		label
	// bne		a,		b,		label
	else if (t.opt == Opt::beq || t.opt == Opt::bne || t.opt == Opt::bgt || t.opt == Opt::bge || t.opt == Opt::blt || t.opt == Opt::ble) {
		if (util.isInt(t.var1)) {
			enterCode("", "li", "$s0", t.var1, "");
		}
		else {
			loadIdent("$s0", t.var1);
		}
		if (util.isInt(t.var2)) {
			enterCode("", "li", "$s1", t.var2, "");
		}
		else {
			loadIdent("$s1", t.var2);
		}
		enterCode("", OptName[t.opt], "$s0", "$s1", t.var3);
	}
	// ret		int,	1,		f
	// ret		char,	ch,		f
	// ret		void,	  ,		f
	// ret		int,	$t0,	f
	else if (t.opt == Opt::ret) {
		if (t.var1 == "int") {
			if (util.isInt(t.var2)) {
				enterCode("", "li", "$v0", t.var2,"");
			}
			else {
				loadIdent("$v0", t.var2);
			}
		}
		else if(t.var1 == "char"){
			if (util.isInt(t.var2)) {
				num = util.str2int(t.var2);
				num = num % 256;		// ��Ϊchar����
				enterCode("", "li", "$v0", util.int2str(num), "");
			}
			else {
				loadIdent("$v0", t.var2, true);
			}
		}
		enterCode("", "j", "end_" + t.var3, "", "");
	}
	// scan		int,	m
	// scan		char,	ch
 	else if (t.opt == Opt::scan) {
		if (t.var1 == "int") {
			enterCode("", "li", "$v0", "5", "");
			enterCode("", "syscall", "", "", "");
			saveIdent("$v0", t.var2);
		}
		else if(t.var1 == "char"){
			enterCode("", "li", "$v0", "12", "");
			enterCode("", "syscall", "", "", "");
			saveIdent("$v0", t.var2, true);
		}
	}
	// print	int,	10
	// print	char,	67
	// print	char,	ch
	// print	str,	STRCST0
	// print	char,	$t0
	else if (t.opt == Opt::print) {
		if (t.var1 == "int") {
			loadIdent("$a0", t.var2);
			enterCode("", "li", "$v0", "1", "");
			enterCode("", "syscall", "", "", "");
		}
		else if (t.var1 == "char") {
			loadIdent("$a0", t.var2, true);
			enterCode("", "li", "$v0", "11", "");
			enterCode("", "syscall", "", "", "");
		}
		else {
			enterCode("", "la", "$a0", t.var2, "");
			enterCode("", "li", "$v0", "4", "");
			enterCode("", "syscall", "", "", "");
		}
	}
	// larr�е�var1һ������ʱ����
	// larr		$t0,	arr,	10
	// larr		$t0,	arr,	$t1
	// larr		$t0,	arr,	a
	else if (t.opt == Opt::larr) {
		if (symtab.locate(t.var2, 0) != -1) {
			enterCode("", "la", "$s0", t.var2, "");
		}
		else if (symtab.locate(t.var2, 1) != -1) {
			index = symtab.locate(t.var2, 1);
			tmpaddress = -4 * symtab.SymbolTable[index].address;		
			enterCode("", "addi", "$s0", "$fp", util.int2str(tmpaddress));
		}

		loadIdent("$s1", t.var3);
		enterCode("", "mul", "$s1", "$s1", "4");
		enterCode("", "add", "$s0", "$s0", "$s1");

		if (symtab.SymbolTable[index].type == typeEnum::Char) {
			enterCode("", "lb", "$s1", "$s0", "0");
			saveIdent("$s1", t.var1, true);
		}
		else {
			enterCode("", "lw", "$s1", "$s0", "0");
			saveIdent("$s1", t.var1);
		}
	}
	// sarr��var1��var3�����������֡���������ʱ����
	// sarr		$t0,	arr,	$t1
	// sarr		10,		arr,	$t1
	// sarr		a,		arr,	$t1
	// sarr		$t0,	arr,	10
	// sarr		$t0,	arr,	a
	else if (t.opt == Opt::sarr) {
		
		loadIdent("$s0", t.var1);
		loadIdent("$s1", t.var3);
		enterCode("", "mul", "$s1", "$s1", "4");

		if (symtab.locate(t.var2, 0) != -1) {
			enterCode("", "la", "$s2", t.var2, "");
		}
		else if (symtab.locate(t.var2, 1) != -1) {
			index = symtab.locate(t.var2, 1);
			tmpaddress = -4 * symtab.SymbolTable[index].address;
			enterCode("", "addi", "$s2", "$fp", util.int2str(tmpaddress));
		}

		enterCode("", "add", "$s2", "$s2", "$s1");
		
		if (symtab.SymbolTable[index].type == typeEnum::Char) {
			enterCode("", "sb", "$s0", "$s2", "0");
		}
		else {
			enterCode("", "sw", "$s0", "$s2", "0");
		}
	}
	// call�е�var3һ������ʱ����
	// call		int,	f,		$t0
	// call		char,	f,		$t0
	// call		void,	f,		
	else if (t.opt == Opt::call) {

		enterCode("", "addi", "$sp", "$sp", "-8");
		enterCode("", "sw", "$fp", "$sp", "4");
		// enterCode("", "sw", "$ra", "$sp", "0"); ��jal���뺯��֮���ٱ��� $ra
		
		s = codetab.textSeg[textCodeIndex++];
		index = symtab.locate(t.var2, 0);	
		paNum = symtab.SymbolTable[index].paranum;

		i = 0;
		// parain	int,	10,		f
		// parain	char,	ch,		f
		// parain	int,	$t0,	f
		while (i < paNum) {
			if (s.opt == Opt::parain) {
				enterCode("", "addi", "$sp", "$sp", "-4");
				if (s.var1 == "char") {
					loadIdent("$s0", s.var2, true);
				}
				else {
					loadIdent("$s0", s.var2);
				}
				enterCode("", "sw", "$s0", "$sp", "0");
				i++;
			}
			else {
				statement2mips(s);
			}
			s = codetab.textSeg[textCodeIndex++];
		}
		textCodeIndex--;		// ����һ����䣬��Ϊ�����1��֮�󻹻��1
		num = paNum - 1 + 2;	// -1��ָsp����ָ����һ������+ 2ָ$fp,$ra��������

		// enterCode("", "addi", "$fp", "$sp",util.int2str(4 * num));	����$fp������Ѱַ�ļĴ������������������뵽����֮ǰ���ܸı�		
		enterCode("", "jal", t.var2, "", "");

		if (t.var1 == "int") {
			saveIdent("$v0", t.var3);
		}
		else if(t.var1 == "char"){
			saveIdent("$v0", t.var3, true);
		}
	} 
	// j	,		,		label
	else if (t.opt == Opt::j) {
		enterCode("", "j", t.var3, "", "");
	}
}

// ��src����������load��dst�Ĵ�����
void quad2mips::loadIdent(string dst,string src)
{
	int index = 0;
	int tmpaddress = 0;

	if (util.isInt(src)) {
		enterCode("", "li", dst, src, "");
		return;
	}
	
	// ȫ�ֱ�����ֱ��ȡַ
	if (symtab.locate(src, 0) != -1) {
		index = symtab.locate(src, 0);
		// char���Ϳ��Դ渺ֵ��Ҳ��Ϊ�з�����
		if (symtab.SymbolTable[index].type == typeEnum::Char) {
			enterCode("", "lb", dst, src, "");
		}
		else {
			enterCode("", "lw", dst, src, "");
		}
	}
	
	else if(symtab.locate(src, 1) != -1){
		index = symtab.locate(src, 1);
		tmpaddress = -1 * 4 * symtab.SymbolTable[index].address;
		if (symtab.SymbolTable[index].type == typeEnum::Char) {
			enterCode("", "lb", dst, "$fp", util.int2str(tmpaddress));
		}
		else {
			enterCode("", "lw", dst, "$fp", util.int2str(tmpaddress));
		}
	}
	else {
		cout << "loadIdent - ���ű���û�м�鵽�ñ�ʶ�� �� " << src << " in " << src << " -> " << dst << endl;
		if (DEBUG && MipsDEBUG) {
			symtab.PrintTable();
		}
	}
}

// forceChar��ǿ�ƽ�src��Ϊchar����load��dst�Ĵ�����
void quad2mips::loadIdent(string dst, string src, bool forceChar)
{
	int index = 0;
	int tmpaddress = 0;
	int tmpnum = 0;
	if (forceChar) {
		if (util.isInt(src)) {
			tmpnum = util.str2int(src);
			tmpnum = tmpnum % 256;
			enterCode("", "li", dst, util.int2str(tmpnum), "");
			return;
		}
		if (symtab.locate(src, 0) != -1) {
			enterCode("", "lb", dst, src, "");
		}
		else if (symtab.locate(src, 1) != -1) {
			index = symtab.locate(src, 1);
			tmpaddress = -1 * 4 * symtab.SymbolTable[index].address;
			enterCode("", "lb", dst, "$fp", util.int2str(tmpaddress));
		}
		else {
			cout << "loadIdent - ���ű���û�м�鵽�ñ�ʶ�� �� " << src << " in " << src << " -> " << dst << endl;
			if (DEBUG && MipsDEBUG) {
				symtab.PrintTable();
			}
		}
	}
	else {
		loadIdent(dst, src);
	}
}

// ��src�Ĵ���save��dst������ı�����
void quad2mips::saveIdent(string src, string dst)
{
	int index = 0;
	int tmpaddress = 0;
	if (symtab.locate(dst, 0) != -1) {
		index = symtab.locate(dst, 0);
		if (symtab.SymbolTable[index].type == typeEnum::Char) {
			enterCode("", "sb", src, dst, "");
		}
		else {
			enterCode("", "sw", src, dst, "");
		}
	}

	else if (symtab.locate(dst, 1) != -1) {
		index = symtab.locate(dst, 1);
		tmpaddress = -1 * 4 * symtab.SymbolTable[index].address;
		if (symtab.SymbolTable[index].type == typeEnum::Char) {
			enterCode("", "sb", src, "$fp", util.int2str(tmpaddress));
		}
		else {
			enterCode("", "sw", src, "$fp", util.int2str(tmpaddress));
		}
	}
	else {
		cout << "saveIdent - ���ű���û�м�鵽�ñ�ʶ�� �� " << dst << " in " << src << " -> " << dst << endl;
		if (DEBUG && MipsDEBUG) {
			symtab.PrintTable();
		}
	}
}

// forceChar��ǿ�ƽ�src��Ϊchar����save��dst�Ĵ�����
void quad2mips::saveIdent(string src, string dst, bool forceChar)
{
	int index = 0;
	int tmpaddress = 0;
	if (forceChar) {
		if (symtab.locate(dst, 0) != -1) {
			enterCode("", "sb", src, dst, "");
		}

		else if (symtab.locate(dst, 1) != -1) {
			index = symtab.locate(dst, 1);
			tmpaddress = -1 * 4 * symtab.SymbolTable[index].address;
			enterCode("", "sb", src, "$fp", util.int2str(tmpaddress));
		}
		else {
			cout << "saveIdent - ���ű���û�м�鵽�ñ�ʶ�� �� " << dst << " in " << src << " -> " << dst << endl;
			if (DEBUG && MipsDEBUG) {
				symtab.PrintTable();
			}
		}
	}
	else {
		saveIdent(src, dst);
	}
}

// Ѱ�Ҿֲ���������ʱ�������ܺͣ��ر�ע�����飩
int quad2mips::FindVarNum(int index)
{
	int num = 0;
	int MaxTmpVar = -1;
	int tmpCode = 0;
	code tmp = codetab.textSeg[index++];

	while (tmp.opt != Opt::end) {
		if (tmp.opt == Opt::cst || tmp.opt == Opt::var) {
			num++;
		}
		else if (tmp.opt == Opt::arr) {
			num = num + util.str2int(tmp.var3);
		}
		else {
			if (util.isTmpVar(tmp.var1)) {
				tmpCode = util.str2int(tmp.var1.substr(2, tmp.var1.length()));
				if (tmpCode > MaxTmpVar) {
					MaxTmpVar = tmpCode;
				}
			}
			if (util.isTmpVar(tmp.var2)) {
				tmpCode = util.str2int(tmp.var2.substr(2, tmp.var2.length()));
				if (tmpCode > MaxTmpVar) {
					MaxTmpVar = tmpCode;
				}
			}
			if (util.isTmpVar(tmp.var3)) {
				tmpCode = util.str2int(tmp.var3.substr(2, tmp.var3.length()));
				if (tmpCode > MaxTmpVar) {
					MaxTmpVar = tmpCode;
				}
			}
		}
		tmp = codetab.textSeg[index++];
	}
	num = num + MaxTmpVar + 1;		// ��Ϊ�Ǵ�$t0��ʼ��
	return num;
}

int quad2mips::enterCode(string label, string op, string var1, string var2, string var3)
{
	mipscode code;
	code.label = label;
	code.op = op;
	code.var1 = var1;
	code.var2 = var2;
	code.var3 = var3;

	asmcode.push_back(code);
	
	Print2File(code);

	return asmcode.size()-1;
}

// ��ӡ������func:
//				a:		.word,		100
//				sw		$t0,		4($sp)
//				add		$t0,		$t1,		$t2
// ֻ�е�var1,var2��Ϊ�գ������ǵĺ��var��Ϊ��ʱ���ж���
void quad2mips::Print2File(mipscode c)
{
	if (c.label == "") {
		c.label = "\t";
	}
	if (c.op == "lw" || c.op == "sw" || c.op == "sb" || c.op == "lb") {
		// lw	$t0,	-4($t1)
		if (c.var3 == "") {				// �����ȫ�ֱ���
			fprintf(res, "%s%-15s%-15s%-15s\n", c.label.c_str(), c.op.c_str(), (c.var1 + ",").c_str(), c.var2.c_str());
		}
		else {					// ����Ǿֲ�����
			fprintf(res, "%s%-15s%-15s%-15s\n", c.label.c_str(), c.op.c_str(), (c.var1 + ",").c_str(), (c.var3 + "(" + c.var2 + ")").c_str());
		}
	}
	else {
		if (c.var1 != "" && c.var2 != "") {
			c.var1 = c.var1 + ",";
		}
		if (c.var2 != "" && c.var3 != "") {
			c.var2 = c.var2 + ",";
		}
		fprintf(res, "%s%-15s%-15s%-15s%-15s\n", c.label.c_str(), c.op.c_str(), c.var1.c_str(), c.var2.c_str(), c.var3.c_str());
	}
	if (DEBUG && MipsDEBUG) {
		cout << c.label << "\t" << c.op << "\t\t" << c.var1 << "\t\t" << c.var2 << "\t\t" << c.var3 << endl;
	}
}



