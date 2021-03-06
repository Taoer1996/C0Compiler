#include "stdafx.h"
#include "error.h"
#include "symTable.h"
#include "lexical.h"

extern int LineNum;
extern int CharCount;
extern symTable symtab;
extern lexical lex;

string ErrorTable[] = {
	"符号表溢出! - 0",
	"标识符多次定义! - 1",
	"缺少标识符! - 2",
	"缺少右中括号! - 3",
	"此处缺少数字! - 4",
	"变量定义之后应为左中括号或者逗号! - 5",
	"缺少分号! - 6",
	"变量定义中出现不合法单词! - 7",
	"缺少右大括号! - 8",
	"缺少左大括号! - 9",
	"缺少右小括号! - 10",
	"缺少左小括号! - 11",
	"应该为=而不是==! - 12",
	"常量定义中常量类型应为int或者char! - 13",
	"此处应该为=! - 14",
	"字符常量定义缺少字符值! - 15",
	"函数参数的类型必须为int或char! - 16",
	"main函数的类型必须为void! - 17",
	"缺少main函数或者main函数不是最后一个函数! - 18",
	"语句开头出现不合法字符! - 19",
	"条件语句未以if开头! - 20",
	"循环语句未以while开头! - 21",
	"条件语句未以switch开头! - 22",
	"case条件语句未以case开头! - 23",
	"缺少冒号! - 24",
	"case语句中必须使用常量作为选择条件! - 25",
	"缺省情况应该以default开头! - 26",
	"函数调用语句需要以函数名开头! - 27",
	"读语句必须以scanf开头! - 28",
	"返回语句必须以return开头! - 29",
	"数字超长! - 30",
	"缺少双引号! - 31",
	"非法字符常量! - 32",
	"缺少单引号! - 33",
	"非法出现感叹号! - 34",
	"出现非法字符! - 35",
	"标识符多次定义! - 36",
	"调用未定义函数! - 37",
	"无返回值函数出现返回语句! - 38",
	"有返回值函数没有返回语句或者是分支中存在无返回值情况! - 39",
	"传入参数个数和函数参数个数不符! - 40",
	"语句格式错误！ - 41",
	"条件运算符错误！ - 42",
	"返回值类型与函数声明不匹配！ - 43",
	"返回语句返回值应该使用括号包围，或者没有返回值！ - 44",
	"未定义标识符！ - 45",
	"非数组变量进行数组操作！ - 46",
	"数组越界！ - 47",
	"非函数进行函数调用! - 48",
	"无返回值函数调用返回值! - 49",
	"函数名或者数组名作为变量调用! - 50",
	"常量不可以修改值! - 51",
	"对数组名进行赋值! - 52",
	"scanf语句中标识符不允许为函数名! - 53",
	"读语句中至少需要有一个参数! - 54",
	"写语句必须以printf开头! - 55",
	"printf语句中字符串后为逗号或者是右括号! - 56",
	"数组长度应为正整数! - 57",
	"scanf语句中标识符不允许为数组头! - 58",
	"因子中出现非法内容! - 59",
	"对非变量类型进行赋值操作! - 60",
	"! - 61",
	"! - 62",
	"! - 63",
	"! - 64",
	"! - 65",
	"! - 66",
	"! - 67",
	"! - 68",
	"! - 69",
	"! - 70",
	"! - 71",
	"! - 72",
	"! - 73",
	"! - 74",
	"! - 75",
	"! - 76",
	"! - 77",
	"! - 78",
	"! - 79",
	"! - 80",
	"! - 81",
	"! - 82",
	"! - 83",
	"! - 84",
	"! - 85",
	"! - 86",
	"! - 87",
	"! - 88",
	"! - 89",
	"! - 90",
	"! - 91",
	"! - 92",
	"! - 93",
	"! - 94",
	"! - 95",
	"! - 96",
	"! - 97",
	"! - 98",
	"! - 99"
};

error::error()
{
	error::errnum = 0;
}


error::~error()
{

}

void error::Err(int code)
{
	errnum++;
	cout << "错误" << errnum << " : 第" << LineNum << "行-单词 " << lex.getWord() << " - " << ErrorTable[code] << endl;
}
int error::getErrNum() {
	return error::errnum;
}
