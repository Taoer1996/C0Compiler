#include "stdafx.h"
#include "parser.h"


int lev = 0;				// 当前所在层次
bool retMark = false;		// return语句的mark，如果有返回值则为true，否则为false，这是一个重要的标识变量
bool isExpTypeChar = true;	// 标识表达式的类型，只有全为char类型的变量或常量且没有乘除操作，表达式才是char类型，每次出expression()之后根据isExpTypeChar就可判断表达式类型
string funcName = "";		// 表示当前所在的函数的名字，在return语句中使用

set<Symbol> errset;
string tmpSymName[3] = { "int","char","void" };

myutils util;				// 工具类

parser::parser()
{
	parser::tmpvarcode = 0;
	parser::tmplabcode = 0;
	parser::tmpstrcode = 0;
}


parser::~parser()
{
}

// TODO 检测出错处理是否完善
void parser::program() {
	int mainMark = 0;
	Symbol lastFuncType;
	string lastFuncName;
	int paNum;
	int tmpnum;
	int tmpindex = 0;
	int codetmpindex = 0;
	typeEnum tmptype;
	string tmptoken;
	Symbol tmpsym;

	lex.getsym();

	if (sym == constsy) {
		constDeclaration(true);
	}

	while (sym == intsy || sym == charsy) {
		tmpsym = sym;
		lex.getsym();
		tmptoken = token;
		lex.getsym();

		tmptype = (tmpsym == intsy) ? Int : Char;

		if (sym == lbrack) {
			goto ArrayVarStart;
		}

		else if (sym == comma || sym == semicolon) {
			
			symtab.enter(tmptoken, kindEnum::varkind, tmptype, lev, 0, 0, 0, 0);
			// 格式：var,	int,	name,	;
			codetab.emitdata(Opt::var, tmpSymName[tmpsym], tmptoken, "");

			while (sym == comma) {
				lex.getsym();
				tmptoken = token;
				lex.getsym();
				if (sym == comma || sym == semicolon) {
					symtab.enter(tmptoken, kindEnum::varkind, tmptype, lev, 0, 0, 0, 0);
					codetab.emitdata(Opt::var, tmpSymName[tmpsym], tmptoken, "");
				}
				else if (sym == lbrack) {

				ArrayVarStart:
					lex.getsym();
					if (sym == intcon) {
						tmpnum = num;
						if (tmpnum <= 0) {
							ERR.Err(57);
							tmpnum = 0;
						}
						lex.getsym();
						if (sym == rbrack) {
							lex.getsym();
						}
						else {
							ERR.Err(3);   // 此处虽报错，但是继续编译，做容错处理
						}
						symtab.enter(tmptoken, kindEnum::arrkind, tmptype, lev, tmpnum, 0, 0, 0);
						// 格式：arr,	int,	name,	size;
						codetab.emitdata(Opt::arr, tmpSymName[tmpsym], tmptoken, int2str(tmpnum));
					}
					else {
						ERR.Err(4);
						errset.clear();
						errset.insert(comma);
						errset.insert(semicolon);	
						errjump(errset);			// 跳读，缩小影响范围
					}
				}
				else {
					ERR.Err(5);
					errset.clear();
					errset.insert(comma);
					errset.insert(semicolon);
					errjump(errset);			// 跳读，缩小影响范围
				}
			}
			if (sym == semicolon) {
				lex.getsym();
			}
			else {
				ERR.Err(6);       // 依旧是容错处理
			}
		}

		else if (sym == lparent) {
			goto FuncDefStart;
		}
		
		else {
			ERR.Err(7);
			errset.clear();
			errset.insert(semicolon);
			errjump(errset);			// 跳读，缩小影响范围
		}
	}

	while (sym == intsy || sym == charsy || sym == voidsy) {
		tmpsym = sym;
		lastFuncType = sym;
		tmptype = (tmpsym == intsy) ? typeEnum::Int : (tmpsym == charsy)? typeEnum::Char:typeEnum::Void;
		lex.getsym();
		if (sym != ident && sym != mainsy) {
			ERR.Err(2);
			errset.clear();
			errset.insert(ident);
			errjump(errset);			// 跳读，缩小影响范围
		}
		
		tmptoken = token;
		lastFuncName = token;
		lex.getsym();
		if (sym == lparent) {
		FuncDefStart:
			// 设置标签
			tmpindex  =  symtab.enter(tmptoken, kindEnum::funckind, tmptype, 0, 0, 0, 0, 0);		//  反填paranum
			// 格式：func,	int,	f,	;
			codetmpindex = codetab.emit(Opt::func, tmpSymName[tmpsym], tmptoken, "");

			funcName = tmptoken;			// 设置当前所在的函数名
			parser::tmpvarcode = 0;			// 每次进入新函数都可以重新分配临时变量
			lev++;

			lex.getsym();
			paNum = paraList();
			symtab.SymbolTable[tmpindex].paranum = paNum;
			codetab.textSeg[codetmpindex].var3 = util.int2str(paNum);

			if (sym == rparent) {
				lex.getsym();
				if (sym != lbrace) {
					ERR.Err(9);
					errset.clear();
					errjump(errset);			// 跳读，缩小影响范围
				}
				
				retMark = false;
				lex.getsym();
				compoundStat();
				if (sym == rbrace) {
					
					if (tmpsym == intsy) {
						tmptype = typeEnum::Int;
					}
					else if (tmpsym == charsy) {
						tmptype = typeEnum::Char;
					}
					else {
						tmptype = typeEnum::Void;
					}
					
					// 格式：setlab,		,	,	end_f;
					// 格式：end,	func,	f,	;
					codetab.emit(Opt::setlab, "", "", "end_" + tmptoken);
					codetab.emit(Opt::end, "func", tmptoken, "");
					lex.getsym();
					if (retMark == true && tmpsym == voidsy) {
						ERR.Err(38);
					}
					else if (retMark == false && tmpsym != voidsy ) {
						ERR.Err(39);
					}
				}
				else {
					ERR.Err(8);       // 这里不是容错，是直接报错
				}
				lev--;
				funcName = "";		// 将当前所在函数名清空
				symtab.clean();		// 清理临时变量
			}
			else {
				ERR.Err(10);      //这里也不是容错处理，而是直接报错
			}
		}
		else {
			ERR.Err(11);          // 这里不是容错，而是直接报错，且已经预读了
		}
	}

	if (!(lastFuncName == "main" && lastFuncType == voidsy)) {
		ERR.Err(18);
	}
		
	// 将生成的四元式打印到文件中
	codetab.logCode();
}

// 每次进入constDeclaration之前都已经检查过是以const开头的了
// isData代表是否为数据段四元式
void parser::constDeclaration(bool isData) {
	if (sym == constsy) {
		lex.getsym();
		constDefine(isData);
		if (sym == semicolon) {
			lex.getsym();
		}
		else {
			ERR.Err(6);       //容错处理
		}
		while (sym == constsy) {
			lex.getsym();
			constDefine(isData);
			if (sym == semicolon) {
				lex.getsym();
			}
			else {
				ERR.Err(6);       //容错处理
			}
		}
	}
}

// isData代表是否为全局数据段
void parser::constDefine(bool isData) {
	typeEnum tmptype;
	Symbol tmpsym;
	string tmptoken;
	int tmpnum;
	if (sym == intsy || sym == charsy) {
		tmpsym = sym;
		tmptype = (tmpsym == intsy) ? typeEnum::Int : typeEnum::Char;
		lex.getsym();
		if (sym != ident) {                           // 跳读
			ERR.Err(2);
			errset.clear();
			errset.insert(comma);
			errset.insert(semicolon);
			errjump(errset);
			if (sym == comma) {						//跳至下一个常量，继续进行
				lex.getsym();
			}
			else {
				return;
			}
		}
		if (sym == ident) {
			tmptoken = token;
			lex.getsym();
			if (sym == becomes || sym == eql) {
				if (sym == eql) {
					ERR.Err(12);       //容错处理
				}
				lex.getsym();
				if (tmpsym == intsy) {
					tmpnum = integer();
					symtab.enter(tmptoken, kindEnum::cstkind, tmptype, lev, 0, 0, 0, tmpnum);
					// 格式：cst,	int,	name,	value;
					// 字符常量也使用值存入
					if (isData) {
						codetab.emitdata(Opt::cst, tmpSymName[tmpsym], tmptoken, int2str(tmpnum));
					}
					else {
						codetab.emit(Opt::cst, tmpSymName[tmpsym], tmptoken, int2str(tmpnum));
					}
				}
				else if (tmpsym == charsy) {
					if (sym == charcon) {
						tmpnum = num;
						symtab.enter(tmptoken, kindEnum::cstkind, tmptype, lev, 0, 0, 0, tmpnum);
						// 格式：cst,	int,	name,	value;
						// 字符常量也使用值存入
						if (isData) {
							codetab.emitdata(Opt::cst, tmpSymName[tmpsym], tmptoken, int2str(tmpnum));
						}
						else {
							codetab.emit(Opt::cst, tmpSymName[tmpsym], tmptoken, int2str(tmpnum));
						}
						lex.getsym();
					}
					else {
						ERR.Err(15);
						errset.clear();
						errset.insert(comma);
						errset.insert(semicolon);
						errjump(errset);
					}
				}
				
				while (sym == comma) {
					lex.getsym();
					if (sym == ident) {
						tmptoken = token;
						lex.getsym();
						if (sym == becomes || sym == eql) {
							if (sym == eql) {
								ERR.Err(12);       //容错处理
							}
							lex.getsym();
							if (tmpsym == intsy) {
								tmpnum = integer();
								symtab.enter(tmptoken, kindEnum::cstkind, tmptype, lev, 0, 0, 0, tmpnum);
								// 格式：cst,	int,	name,	value;
								// 字符常量也使用值存入
								if (isData) {
									codetab.emitdata(Opt::cst, tmpSymName[tmpsym], tmptoken, int2str(tmpnum));
								}
								else {
									codetab.emit(Opt::cst, tmpSymName[tmpsym], tmptoken, int2str(tmpnum));
								}
							}
							else if (tmpsym == charsy) {
								if (sym == charcon) {
									tmpnum = num;
									symtab.enter(tmptoken, kindEnum::cstkind, tmptype, lev, 0, 0, 0, tmpnum);
									// 格式：cst,	int,	name,	value;
									// 字符常量也使用值存入
									if (isData) {
										codetab.emitdata(Opt::cst, tmpSymName[tmpsym], tmptoken, int2str(tmpnum));
									}
									else {
										codetab.emit(Opt::cst, tmpSymName[tmpsym], tmptoken, int2str(tmpnum));
									}
									lex.getsym();
								}
								else {
									ERR.Err(15);
									errset.clear();
									errset.insert(comma);
									errset.insert(semicolon);
									errjump(errset);
								}
							}
						}
						else {
							ERR.Err(14);
							errset.clear();
							errset.insert(comma);
							errset.insert(semicolon);
							errjump(errset);
						}
					}
					else {
						ERR.Err(2);           // 报错
						errset.clear();
						errset.insert(comma);
						errset.insert(semicolon);
						errjump(errset);
					}
				}
			}
			else {
				ERR.Err(14);                  // 直接报错
				errset.clear();
				errset.insert(semicolon);
				errjump(errset);			 // 直接读到分号那里
			}
		}
		else {
			ERR.Err(2);
			errset.clear();
			errset.insert(semicolon);
			errjump(errset);	// 直接读完这一行或者读至分号
			return;
		}
	}
	else {
		ERR.Err(13);
		errset.clear();
		errset.insert(semicolon);
		errjump(errset);	// 直接读完这一行或者读至分号
	}
}

int parser::integer() {
	int flag = 1;
	int tmpnum = 0;
	if (sym == intcon && num == 0) {
		lex.getsym();
		return 0;
	}
	if (sym == add) {
		lex.getsym();
	}
	else if (sym == sub) {
		flag = -1;
		lex.getsym();
	}
	if (sym == intcon) {
		tmpnum = num;
		lex.getsym();
		return flag*tmpnum;
	}
	else {
		ERR.Err(4);
		errset.clear();
		errset.insert(comma);
		errset.insert(semicolon);
		errjump(errset);	
		return 0;
	}
}

// ＜变量说明＞::= ＜变量定义＞;{＜变量定义＞;}
// ＜变量定义＞::= ＜类型标识符＞(＜标识符＞|＜标识符＞‘[’＜无符号整数＞‘]’){,＜标识符＞|＜标识符＞‘[’＜无符号整数＞‘]’ }
void parser::varDeclaration() {
	varDefine();
	if (sym == semicolon) {
		lex.getsym();
		while (sym == intsy || sym == charsy) {
			varDefine();
			if (sym == semicolon) {
				lex.getsym();
			}
			else {
				ERR.Err(6);       // 容错处理
			}
		}
	}
	else {
		ERR.Err(6);       //容错处理
	}
}

void parser::varDefine() {
	typeEnum tmptype;
	Symbol tmpsym;
	string tmptoken;
	int tmpnum;
	if (sym == intsy || sym == charsy) {
		tmpsym = sym;
		tmptype = (sym == intsy) ? typeEnum::Int : typeEnum::Char;
		lex.getsym();
		tmptoken = token;
		lex.getsym();

		if (sym == lbrack) {
			goto varDefine_Array;
		}

		else if (sym == comma || sym == semicolon) {
			symtab.enter(tmptoken, kindEnum::varkind, tmptype, lev, 0, 0, 0, 0);
			// 格式：var,	int,	name,	;
			codetab.emit(Opt::var, tmpSymName[tmpsym], tmptoken, "");

			while (sym == comma) {
				lex.getsym();
				tmptoken = token;
				lex.getsym();
				if (sym == comma || sym == semicolon) {
					symtab.enter(tmptoken, kindEnum::varkind, tmptype, lev, 0, 0, 0, 0);
					// 格式：var,	int,	name,	;
					codetab.emit(Opt::var, tmpSymName[tmpsym], tmptoken, "");
				}
				else if (sym == lbrack) {
				varDefine_Array:
					lex.getsym();
					if (sym == intcon) {
						tmpnum = num;
						lex.getsym();
						if (sym == rbrack) {
							lex.getsym();
						}
						else {
							ERR.Err(3);   // 此处虽报错，但是继续编译，做容错处理
						}
						symtab.enter(tmptoken, kindEnum::arrkind, tmptype, lev, tmpnum, 0, 0, 0);
						// 格式：arr,	int,	name,	size;
						codetab.emit(Opt::arr, tmpSymName[tmpsym], tmptoken, int2str(tmpnum));
					}
					else {
						ERR.Err(4);
						errset.clear();
						errset.insert(semicolon);
						errset.insert(comma);
						errjump(errset);
					}
				}
				else{
					ERR.Err(5);
					errset.clear();
					errset.insert(comma);
					errset.insert(semicolon);
					errjump(errset);
				}
			}
		}
	}
	else {
		ERR.Err(13);
		errset.clear();
		errjump(errset);
	}
}

int parser::paraList() {
	int n = 0;
	typeEnum tmptype;
	Symbol tmpsym;
	if (sym == rparent) {     // 为空
		return 0;
	}
	else if (sym == intsy || sym == charsy) {
		tmpsym = sym;
		tmptype = (sym == intsy) ? typeEnum::Int : typeEnum::Char;
		lex.getsym();
		if (sym == ident) {
			symtab.enter(token, kindEnum::parakind, tmptype, lev, 0, 0, 0, 0);
			// 格式：para,	int,	name,	;
			codetab.emit(Opt::para, tmpSymName[tmpsym], token, "");
			n++;
			lex.getsym();
		}
		else {
			ERR.Err(2);
			errset.clear();
			errset.insert(comma);
			errset.insert(rparent);
			errset.insert(rbrace);
			errjump(errset);
		}
		while (sym == comma) {
			lex.getsym();
			if (sym == intsy || sym == charsy) {
				tmpsym = sym;
				tmptype = (sym == intsy) ? typeEnum::Int : typeEnum::Char;
				lex.getsym();
				if (sym == ident) {
					symtab.enter(token, kindEnum::parakind, tmptype, lev, 0, 0, 0, 0);
					// 格式：para,	int,	name,	;
					codetab.emit(Opt::para, tmpSymName[tmpsym], token, "");
					n++;
					lex.getsym();
				}
				else {
					ERR.Err(2);
					errset.clear();
					errset.insert(comma);
					errset.insert(rparent);
					errjump(errset);
					return n;
				}
			}
		}
		return n;
	}
	else {
		ERR.Err(16);
		errset.clear();
		errset.insert(rparent);
		errjump(errset);
		return 0;
	}
}

void parser::compoundStat() {
	if (sym == constsy) {
		constDeclaration(false);
	}
	if (sym == intsy || sym == charsy) {
		varDeclaration();
	}
	statList();
}

void parser::statList() {
	while (sym == ifsy || sym == switchsy || sym == whilesy || sym == lbrace || sym == ident || sym == scanfsy || sym == printfsy || sym == returnsy || sym == semicolon) {
		statement();
	}
}

/*
	＜语句＞ ::= ＜条件语句＞｜＜循环语句＞| ‘{’＜语句列＞‘}’｜＜有返回值函数调用语句＞; | ＜无返回值函数调用语句＞;
	            ｜＜赋值语句＞;｜＜读语句＞;｜＜写语句＞;｜＜空＞;|＜情况语句＞｜＜返回语句＞;
*/
void parser::statement() {
	int pos = 0;
	int paras = 0;
	int tmpnum = 0;
	string tmpstr1, tmpstr2;
	string tmptoken;
	kindEnum tmpkind;

	if (sym == ifsy) {
		ifStatement();
	}
	else if (sym == whilesy) {
		loopStatement();
	}
	else if (sym == lbrace) {
		lex.getsym();
		statList();
		if (sym == rbrace) {
			lex.getsym();
			return;
		}
		else {
			ERR.Err(8);       // 容错
		}
	}
	else if (sym == scanfsy) {
		readStatement();
		if (sym == semicolon) {
			lex.getsym();
		}
		else {
			ERR.Err(6);			// 容错处理，相当于预读
		}
	}
	else if (sym == printfsy) {
		writeStatement();
		if (sym == semicolon) {
			lex.getsym();
		}
		else {
			ERR.Err(6);			// 容错处理，相当于预读
		}
	}
	else if (sym == switchsy) {
		switchStatement();
	}
	else if (sym == returnsy) {
		returnStatement();
		if (sym == semicolon) {
			lex.getsym();
		}
		else {
			ERR.Err(6);			// 容错处理，相当于预读
		}
	}
	else if (sym == semicolon) {
		lex.getsym();
	}
	else if (sym == ident) {
		// 这里可能是函数调用和赋值语句
		tmptoken = token;
		lex.getsym();
		pos = symtab.locate(tmptoken, lev);
		if (pos == -1) {
			ERR.Err(45);
			errset.clear();
			errjump(errset);			// 如果第一个就是未定义的标识符，则直接跳过此行
			return;
		}

		// 说明是函数调用
		if (sym == lparent) {
			if (pos == -1) {
				ERR.Err(37);
				errset.clear();
				errjump(errset);	// 直接跳至下一行
				return;
			}

			if (symtab.SymbolTable[pos].kind != kindEnum::funckind) {
				ERR.Err(48);
			}
			// 格式为：call,	void,	name,	;
			// 因为是没有赋值的调用，同时这里的函数call指令在参数填好后调用
			codetab.emit(Opt::call, "void", tmptoken, "");

			lex.getsym();
			paras = valList(tmptoken);
			if (paras != symtab.SymbolTable[pos].paranum) {
				ERR.Err(40);		// 继续编译，但是如果运行程序的话结果不确定
			}

			if (sym == rparent) {
				lex.getsym();
			}
			else {
				ERR.Err(10);		// 容错
			}
		}
		// 说明是数组，也即为赋值语句
		else if (sym == lbrack) {
			if (symtab.SymbolTable[pos].kind != kindEnum::arrkind) {
				ERR.Err(46);
			}
			lex.getsym();
			tmpstr1 = expression();
			if (util.isInt(tmpstr1) == true) {
				tmpnum = util.str2int(tmpstr1);
				if (!(0 <= tmpnum && tmpnum < symtab.SymbolTable[pos].arrsize)) {
					ERR.Err(47);
				}
			}
			if (sym != rbrack) {
				ERR.Err(3);					// 容错处理
			}
			else if (sym == rbrack) {
				lex.getsym();
			}
			if (sym == becomes || sym == eql) {
				if (sym == eql) {
					ERR.Err(12);				// 容错处理
				}
				lex.getsym();
				tmpstr2 = expression();
				// 格式：sarr,	$t0,	arrname,	index;
				codetab.emit(Opt::sarr, tmpstr2, tmptoken, tmpstr1);
			}
			else {
				ERR.Err(14);
				errset.clear();
				errset.insert(semicolon);
				errjump(errset);
			}
		}
		// 直接是赋值语句
		else if (sym == becomes || sym == eql) {
			if (sym == eql) {
				ERR.Err(12);				// 容错处理
			}
			tmpkind = symtab.SymbolTable[pos].kind;

			if (tmpkind == kindEnum::cstkind) {
				ERR.Err(51);
			}
			else if (tmpkind == kindEnum::varkind || tmpkind == kindEnum::parakind) {
				lex.getsym();
				tmpstr1 = expression();
				// 格式：move,	x,	$t0,	;
				codetab.emit(Opt::move, tmptoken, tmpstr1, "");
			}
			else {
				ERR.Err(60);				// 虽然是错的，但是报错之后依旧进行下去
				lex.getsym();
				expression();
			}
		}
		else {
			ERR.Err(41);
			errset.clear();
			errset.insert(semicolon);
			errjump(errset);	//直接处理掉这一行
		}

		// 赋值语句和函数调用两种情况都需要处理分号
		if (sym == semicolon) {
			lex.getsym();
		}
		else {
			ERR.Err(6);			// 容错处理，相当于预读
		}
	}
	else {
		ERR.Err(19);
		errset.clear();
		errjump(errset);		// 这里会直接预读
	}
}

// ＜条件语句＞  ::=  if ‘(’＜条件＞‘)’＜语句＞［else＜语句＞］
// 注意ifStatement中还需要考虑返回值的问题
void parser::ifStatement() {
	bool tmpretMark;				// 用来暂存全局的retMark标志
	bool ifretMark = true;			// if语句中的retMark标志，初始化为true
	int index1 = 0;
	int index2 = 0;
	string labelname1;
	string labelname2;
	code tmp;				// 用于保存condition()返回的code
	if (sym == ifsy) {
		lex.getsym();
		if (sym == lparent) {
			lex.getsym();
			tmp = condition();
			tmp = oppbranchcode(tmp);
			// 格式： beq,	$t0,	$t1,	label; (注意的label反填)
			index1 = codetab.emit(tmp.opt, tmp.var1, tmp.var2, tmp.var3);
			if (sym == rparent) {
				lex.getsym();
			}
			else {
				ERR.Err(10);          //此处作为容错处理
			}

			tmpretMark = retMark;		// 暂存全局的retMark，之后恢复
			
			retMark = false;
			statement();				// 如果存在return语句，则retMark将会变成true
			ifretMark = ifretMark && retMark;		// 如果存在有返回值的return语句，则retMark为true，ifretMark值不变，否则为false

			if (sym == elsesy) {
				// 格式：j,	,	,	label;
				index2 = codetab.emit(Opt::j, "", "", "");		// 这里也需要反填label
				lex.getsym();
				labelname1 = nextLab();
				// 格式：setlab,		,		,label;
				codetab.emit(Opt::setlab, "", "", labelname1);
				// 回填else部分的label
				codetab.textSeg[index1].var3 = labelname1;

				retMark = false;
				statement();				// 如果存在return语句，则retMark将会变成true
				ifretMark = ifretMark && retMark;		// 如果存在有返回值的return语句，则retMark为true，ifretMark值不变，否则为false
				
				labelname2 = nextLab();
				// 格式：setlab,		,		,label;
				codetab.emit(Opt::setlab, "", "", labelname2);
				// 回填跳至if语句结尾的label
				codetab.textSeg[index2].var3 = labelname2;
			}
			else {
				labelname1 = nextLab();
				// 格式：setlab,		,		,label;
				codetab.emit(setlab, "", "", labelname1);
				// 回填跳至if语句结尾的label
				codetab.textSeg[index1].var3 = labelname1;

				ifretMark = ifretMark && false;		// 因为缺省else，相当于是缺少了else中的return语句，所以直接&&false
			}

			retMark = tmpretMark;		// 恢复全局的retMark
			retMark = retMark || ifretMark;		// 这里只需要是或操作即可
		}
		else {
			ERR.Err(11);
			errset.clear();
			errjump(errset);		// 直接跳过这一行
		}
	}
	else {
		ERR.Err(20);
		errset.clear();
		errjump(errset);		// 直接跳过这一行
	}

}

code parser::condition() {
	Opt tmp;
	code tmpcode;
	string tmpstr1, tmpstr2;
	tmpstr1 = expression();
	if(sym == rparent){					// 如果仅仅是纯表达式
		tmpcode.opt = Opt::bne;
		tmpcode.var1 = tmpstr1;
		tmpcode.var2 = "0";
		tmpcode.var3 = "";
		return tmpcode;
	}
	if (sym == lt) {
		tmp = Opt::blt;
	}
	else if (sym == lte) {
		tmp = Opt::ble;
	}
	else if (sym == gt) {
		tmp = Opt::bgt;
	}
	else if (sym == gte) {
		tmp = Opt::bge;
	}
	else if (sym == eql) {
		tmp = Opt::beq;
	}
	else if (sym == neq) {
		tmp = Opt::bne;
	}
	else if (sym == becomes) {
		ERR.Err(12);
		tmp = Opt::beq;			// 容错处理
	}
	if (sym == lt || sym == gt || sym == gte || sym == lte || sym == eql || sym == neq || sym == becomes) {
		lex.getsym();
		tmpstr2 = expression();
		tmpcode.opt = tmp;
		tmpcode.var1 = tmpstr1;
		tmpcode.var2 = tmpstr2;
		// 这里需要返填label，所以需要返回四元式以供填写
		return tmpcode;
	}
	else {
		ERR.Err(42);
		expression();
		return tmpcode;
	}
}

// ＜表达式＞    ::= ［＋｜－］＜项＞{＜加法运算符＞＜项＞}
// 只要出现int类型的变量，则将表达式类型变为int(即isExpTypeChar=false);
// expression可能返回的类型有 : ""（空）	数字（123）		变量（a）	临时变量（$t0）
string parser::expression() {
	int flag = 1;
	Opt tmp;
	string tmpstr1, tmpstr2;
	string tmpvarstr;
	
	isExpTypeChar = true;		// 初始化为true，默认为char类型

	if (sym == add) {
		flag = 1;
		lex.getsym();
	}
	else if (sym == sub) {
		flag = -1;
		lex.getsym();
	}

	tmpstr1 = terms();
	
	if (flag == -1) {
		tmpvarstr = nextTmpVar();
		// 格式：mul,	$t1,	$t0,	-1;
		codetab.emit(Opt::mulop, tmpvarstr, tmpstr1, "-1");
		tmpstr1 = tmpvarstr;
	}
	
	while (sym == Symbol::add || sym == Symbol::sub) {
		tmp = (sym == Symbol::add) ? Opt::addop : Opt::subop;
		lex.getsym();
		tmpstr2 = terms();
		tmpvarstr = nextTmpVar();
		// 格式：add,	$t0,	a,	b;
		codetab.emit(tmp, tmpvarstr, tmpstr1, tmpstr2);
		tmpstr1 = tmpvarstr;
	}
	return tmpstr1;
}

// ＜项＞ ::= ＜因子＞{＜乘法运算符＞＜因子＞}
// terms可能返回的类型有 : ""（空）	数字（123）		变量（a）	临时变量（$t0）
string parser::terms() {
	Opt tmpopt;
	string tmpstr1, tmpstr2, tmpvarstr;

	tmpstr1 = factor();
	tmpvarstr = tmpstr1;

	while (sym == Symbol::mul || sym == Symbol::divi) {

		isExpTypeChar = false;				// 出现乘除操作

		tmpopt = (sym == Symbol::mul) ? Opt::mulop : Opt::diviop;
		lex.getsym();
		tmpstr2 = factor();
		tmpvarstr = nextTmpVar();
		// 格式：mul,	$t2,	$t1,	$t0;
		codetab.emit(tmpopt, tmpvarstr, tmpstr1, tmpstr2);
		tmpstr1 = tmpvarstr;
	}
	return tmpvarstr;
}


/* 
	这里需要对变量的类型和lev等进行详细的检查和报错，是种类最为多的一个函数
	需要检查：
	1、数组是否越界
	1、变量/常量是否定义（常量的话直接返回值）
	3、是否为有返回值函数调用
	4、是否出现除零错误（如果是数字则直接通过值来判断，否则不能确定）
	＜因子＞ ::= ＜标识符＞｜＜标识符＞‘[’＜表达式＞‘]’｜＜整数＞|＜字符＞｜＜有返回值函数调用语句＞|‘(’＜表达式＞‘)’
 */

// factor可能返回中的有： ""（空）	数字（123）		变量（a）	临时变量（$t0）		
string parser::factor() {
	int index;
	int tmpnum;
	string tmpstr;
	bool tmpExpTypeChar = true;
	string tmpvarstr, tmptypename;
	kindEnum tmpkind;
	string tmptoken;
	// 如果是标识符，则可能是变量、常量、数组、函数调用
	if (sym == ident) {
		tmptoken = token;
		index = symtab.locate(tmptoken, lev);
		if (index == -1) {
			ERR.Err(45);
			lex.getsym();
			return "";
		}

		if (symtab.SymbolTable[index].type != typeEnum::Char) {
			isExpTypeChar = false;
		}
		
		lex.getsym();
		// 数组元素
		if (sym == lbrack) {
			
			lex.getsym();

			tmpExpTypeChar = isExpTypeChar;
			tmpstr = expression();
			isExpTypeChar = tmpExpTypeChar;			// 因为数组选元素时会出现数字，但这个不影响表达式的类型

			
			if (sym == rbrack) {
				lex.getsym();
			}
			else {
				ERR.Err(3);            //容错
			}
			if (symtab.SymbolTable[index].kind != kindEnum::arrkind) {
				ERR.Err(46);
			}

			if (util.isInt(tmpstr)) {
				tmpnum = util.str2int(tmpstr);
				if (!(0 <= tmpnum && tmpnum < symtab.SymbolTable[index].arrsize)) {
					ERR.Err(47);
				}
			}
			// 格式：larr,	$t1,	arrname,	$t0;
			tmpvarstr = nextTmpVar();
			codetab.emit(Opt::larr, tmpvarstr, tmptoken, tmpstr);
			return tmpvarstr;
		}
		// 函数调用
		else if (sym == lparent) {
			tmpExpTypeChar = isExpTypeChar;			// 同理，函数调用仅仅看返回值类型，所以其中可能改变isExpTypeChar的值
			if (symtab.SymbolTable[index].kind != kindEnum::funckind) {
				ERR.Err(48);
				return "";
			}
			else {
				if (symtab.SymbolTable[index].type == typeEnum::Void) {
					ERR.Err(49);
					return "";
				}
				else {
					tmpvarstr = nextTmpVar();
					tmptypename = (symtab.SymbolTable[index].type == typeEnum::Char) ? "char" : "int";
					// 格式：call,	int,	f,	$t0;
					codetab.emit(Opt::call, tmptypename, tmptoken, tmpvarstr);
				}
			}
			lex.getsym();
			valList(tmptoken);
			if (sym == rparent) {
				lex.getsym();
			}
			else {
				ERR.Err(10);          //容错
			}
			isExpTypeChar = tmpExpTypeChar;			// 恢复类型值	
			return tmpvarstr;
		}
		// 普通标识符
		else {
			tmpkind = symtab.SymbolTable[index].kind;
			// 如果是常量，则已经填好了值，所以直接返回值
			if (tmpkind == kindEnum::cstkind) {
				return util.int2str(symtab.SymbolTable[index].value);
			}
			// 如果是参数则还没有值，所以返回名称
			else if (tmpkind == kindEnum::parakind) {
				return tmptoken;
			}
			// 如果是普通变量
			else if(tmpkind == kindEnum::varkind){
				return tmptoken;
			}
			else {
				ERR.Err(50);
				return "";
			}
		}
	}
	// 如果是整数
	else if (sym == add || sym == sub || sym == intcon) {
		tmpnum = integer();
		isExpTypeChar = false;
		return util.int2str(tmpnum);
	}
	// 如果是字符常量
	else if (sym == charcon) {
		tmpnum = num;
		lex.getsym();
		return util.int2str(tmpnum);
	}
	// 如果是表达式
	else if (sym == lparent) {
		lex.getsym();
		
		tmpExpTypeChar = isExpTypeChar;		// 暂存当前的类型，因为进入expression()之后会统一设置为true

		tmpstr = expression();
		if (sym == rparent) {
			lex.getsym();
		}
		else {
			ERR.Err(10);                //容错
		}
		
		if (isExpTypeChar == true) {
			isExpTypeChar = tmpExpTypeChar;		// 如果该表达式为char类型，则全局的isExpTypeChar和原来的一致，直接恢复即可
		}										// 否则全局的表达式就是int类型，这里就直接是isExpTypeChar为false，所以不用任何操作
		return tmpstr;
	}
	// 如果什么都不是
	else {
		ERR.Err(59);
		lex.getsym();		// 跳至下一个token
		return "";
	}
}

// ＜循环语句＞   ::=  while ‘(’＜条件＞‘)’＜语句＞
void parser::loopStatement() {
	code tmpcode;
	int index1;
	string tmplabel1, tmplabel2;
	if (sym == whilesy) {
		lex.getsym();
		tmplabel1 = nextLab();
		// 格式：setlab,		,		,label;
		codetab.emit(Opt::setlab, "", "", tmplabel1);

		if (sym == lparent) {
			lex.getsym();
			tmpcode = condition();
			tmpcode = oppbranchcode(tmpcode);
			// 留下index反填
			// 格式：blt,	$t0,	10,		label;
			index1 = codetab.emit(tmpcode.opt, tmpcode.var1, tmpcode.var2, tmpcode.var3);

			if (sym != rparent) {
				ERR.Err(10);      //容错
			}
			else {
				lex.getsym();
			}
			statement();
			// 格式：j,		,		,		label;
			codetab.emit(Opt::j, "", "", tmplabel1);
			tmplabel2 = nextLab();
			// 格式：setlab,		,		,		label;
			codetab.emit(Opt::setlab, "", "", tmplabel2);
			// 反填退出循环的label
			codetab.textSeg[index1].var3 = tmplabel2;
		}
		else {
			ERR.Err(11);
			errset.clear();
			errjump(errset);
		}
	}
	else {
		ERR.Err(21);
		errset.clear();
		errjump(errset);
	}
}

/*
	此部分改写文法：
	＜情况语句＞  ::=  switch ‘(’＜表达式＞‘)’ ‘{’＜情况描述＞‘}’
	＜情况描述＞　::=　＜情况子语句＞{＜情况子语句＞}［＜缺省＞］
	＜情况子语句＞  :: = case＜常量＞：＜语句＞
	＜缺省＞   :: = default: ＜语句＞
*/
// 同时注意考虑分支返回值出现的情况
void parser::switchStatement() {
	string tmpstr;
	string tmplab;
	bool switchretMark = false;
	bool tmpretMark;				// 暂存全局的retMark

	tmpretMark = retMark;

	retMark = false;

	if (sym == switchsy) {
		lex.getsym();
		if (sym == lparent) {
			lex.getsym();
			tmpstr = expression();
			tmplab = nextLab();		// 这个作为条件语句的结尾label

			if (sym == rparent) {
				lex.getsym();
			}
			else {
				ERR.Err(10);        //容错
			}

			if (sym != lbrace) {      //跳读到另一行
				ERR.Err(9);
				errset.clear();
				errjump(errset);
			}
			else {
				lex.getsym();	
			}

			caseDescription(tmpstr, tmplab);		// 将选择值传入，并将最后的结尾label传入

			if (sym == rbrace) {
				lex.getsym();
			}
			else {
				ERR.Err(8);       //容错
			}
			// 格式：setlab,		,		,	label_end;
			codetab.emit(Opt::setlab, "", "", tmplab);			// 设置结尾label
		}
		else {
			ERR.Err(11);
			errset.clear();
			errjump(errset);
		}

	}
	else {
		ERR.Err(22);
		errset.clear();
		errjump(errset);
	}

	switchretMark = retMark;		// 得到switch各个语句中是否都有retMark;
	tmpretMark = tmpretMark || switchretMark;	// 和原来的retMark进行或操作
	retMark = tmpretMark;			// 得到经过了switch语句之后整个函数的retMark情况
}


// ＜情况描述＞　::=　＜情况子语句＞{＜情况子语句＞}［＜缺省＞］
// ＜情况子语句＞  :: = case＜常量＞：＜语句＞
// ＜缺省＞   :: = default: ＜语句＞
void parser::caseDescription(string selectVar, string endLabel)
{
	int index = 0;
	string tmplab;
	bool tmpretMark;
	bool caseretMark = true;
	
	tmpretMark = retMark;			// 暂存全局retMark

	retMark = false;
	index = caseStatement(selectVar, endLabel, -1);			// 此处-1代表这个case为第一个case，所以没有代码中没有其上一个case的代码位置
	caseretMark = caseretMark && retMark;				// 此时是与操作，只有当所有的case语句中都有return时，caseretMark才为true

	while (sym == casesy) {
		retMark = false;
		index = caseStatement(selectVar, endLabel, index);
		caseretMark = caseretMark && retMark;
	}

	if (sym == defaultsy) {
		retMark = false;

		tmplab = nextLab();
		// 设置default label
		// 格式：setlab,		,		,	label;
		codetab.emit(Opt::setlab, "", "", tmplab);
		codetab.textSeg[index].var3 = tmplab;			// 反填上一个case的跳转标签
		lex.getsym();
		if (sym == colon) {
			lex.getsym();
		}
		else {
			ERR.Err(24);
			errset.clear();
			errset.insert(colon);
			errjump(errset);
			if (sym == colon) {			// 跳转冒号之后
				lex.getsym();
			}
		}
		statement();
		// 设置直接跳转至switch结束
		codetab.emit(Opt::j, "", "", endLabel);

		caseretMark = caseretMark && retMark;
	}
	else {
		codetab.textSeg[index].var3 = endLabel;			// 如果没有缺省，也要能跑，这一点很重要
		// 由于case语句可以枚举，所以这里对于retMark要求不是那么严，所以不考虑
		caseretMark = caseretMark && false;				// 因为缺少Default则缺失了一种情况，所以认为这里的caseretMark为false
	}

	tmpretMark = tmpretMark || caseretMark;			// 这里就是或操作了
	retMark = tmpretMark;							// 重新恢复全局retMark
}

// ＜情况子语句＞  :: = case＜常量＞：＜语句＞
// 参数中的selectVar为选择变量、endLabel为switch语句中的结尾的label，
// lastIndex为上一个case跳转语句在代码中的位置（需要反填），同时返回本case跳转语句在代码段中位置
int parser::caseStatement(string selectVar, string endLabel,int lastIndex) {
	int tmpval = 0;
	int index = -1;
	string tmplab;
	if (sym == casesy) {
		// 设置情况标签
		tmplab = nextLab();
		codetab.emit(Opt::setlab, "", "", tmplab);
		if (lastIndex != -1) {
			codetab.textSeg[lastIndex].var3 = tmplab;
		}
		lex.getsym();
		if (sym == add || sym == sub || sym == intcon || sym == charcon) {
			// 是字符常量
			if (sym == charcon) {
				tmpval = num;
				lex.getsym();
			}
			// 否则是整数常量
			else {
				tmpval = integer();
			}
		}
		else {
			ERR.Err(25);
			errset.clear();
			errset.insert(colon);
			errjump(errset);
		}
		if (sym != colon) {
			ERR.Err(24);          //容错
		}
		else {
			lex.getsym();
		}
		// 格式：bne,	$t0,	1,		label;
		index = codetab.emit(Opt::bne, selectVar, util.int2str(tmpval), "");			// 标签待反填
		statement();
		// 设置跳转到结尾的跳转语句
		codetab.emit(Opt::j, "", "", endLabel);
	}
	else {
		ERR.Err(23);
		errset.clear();
		errjump(errset);
	}
	return index;
}

// ＜值参数表＞ ::= ＜表达式＞{,＜表达式＞}｜＜空＞
// 同时返回参数的个数
int parser::valList(string fName)
{
	int index = 0;
	int paNum = 0;
	string tmpvar;
	typeEnum type;
	string typeName;

	index = symtab.locate(fName, 0);		// 得到函数在符号表中的位置
	if (sym == rparent) {
		return 0;			// 不需要预读
	}
	else {
		tmpvar = expression();
		paNum++;
		if (symtab.SymbolTable[index + paNum].kind == kindEnum::parakind) {
			type = symtab.SymbolTable[index + paNum].type;
		}
		typeName = (type == typeEnum::Char) ? "char" : "int";
		// 格式：parain,	 int,	$t0,	f;	
		codetab.emit(Opt::parain, typeName, tmpvar, fName);
		while (sym == comma) {
			lex.getsym();
			tmpvar = expression();
			paNum++;
			if (symtab.SymbolTable[index + paNum].kind == kindEnum::parakind) {
				type = symtab.SymbolTable[index + paNum].type;
			}
			typeName = (type == typeEnum::Char) ? "char" : "int";
			// 格式：parain,	 int,	$t0,	f;		
			codetab.emit(Opt::parain, typeName, tmpvar, fName);
		}
	}
	return paNum;
}

// ＜读语句＞ ::=  scanf ‘(’＜标识符＞{,＜标识符＞}‘)’
// 注意scanf中的标识符不能是数组头
void parser::readStatement() {
	int index;
	string tmpstr;
	kindEnum tmpkind;
	if (sym == scanfsy) {
		lex.getsym();
		if (sym == lparent) {
			lex.getsym();

			if (sym != ident) {
				ERR.Err(2);
				errset.clear();
				errset.insert(ident);				// 跳至下一个标识符或者是右括号
				errset.insert(rparent);				// 从而使得可以继续分析
				errjump(errset);
			}

			if (sym == ident) {
				index = symtab.locate(token, lev);
				if (index == -1) {
					ERR.Err(45);
				}
				else {
					tmpkind = symtab.SymbolTable[index].kind;
					if (tmpkind == kindEnum::cstkind) {
						ERR.Err(51);
					}
					else if (tmpkind == kindEnum::varkind || tmpkind == kindEnum::parakind) {
						tmpstr = (symtab.SymbolTable[index].type == typeEnum::Char) ? "char" : "int";
						// 格式：scan,	int,	var,	;
						codetab.emit(Opt::scan, tmpstr, token, "");
					} 
					else if (tmpkind == kindEnum::arrkind) {
						ERR.Err(58);
					}
					else if(tmpkind == kindEnum::funckind){
						ERR.Err(53);
					}
				}
				lex.getsym();
				
				while (sym == comma) {
					lex.getsym();
					if (sym == ident) {
						index = symtab.locate(token, lev);
						if (index == -1) {
							ERR.Err(45);
						}
						else {
							tmpkind = symtab.SymbolTable[index].kind;
							if (tmpkind == kindEnum::cstkind) {
								ERR.Err(51);
							}
							else if (tmpkind == kindEnum::varkind || tmpkind == kindEnum::parakind) {
								tmpstr = (symtab.SymbolTable[index].type == typeEnum::Char) ? "char" : "int";
								// 格式：scan,	int,	var,	;
								codetab.emit(Opt::scan, tmpstr, token, "");
							}
							else {
								ERR.Err(53);
							}
						}
						lex.getsym();
					}
					else {
						ERR.Err(2);
						errset.clear();
						errset.insert(comma);
						errset.insert(semicolon);
						errset.insert(rparent);
						errjump(errset);
					}
				}
				if (sym == rparent) {
					lex.getsym();
				}
				else {
					ERR.Err(10);          //容错
				}
			}
			// 如果直接读到右括号
			else if(sym == rparent){
				ERR.Err(54);
				lex.getsym();
			}
			// 如果直接读到了其它
			else {
				ERR.Err(2);
				errset.clear();
				errset.insert(semicolon);
				errjump(errset);
			}
		}
		else {
			ERR.Err(11);
			errset.clear();
			errset.insert(semicolon);
			errjump(errset);
		}
	}
	else {
		ERR.Err(28);
		errset.clear();
		errset.insert(semicolon);
		errjump(errset);
	}
}


// ＜写语句＞ ::= printf ‘(’ ＜字符串＞,＜表达式＞ ‘)’| printf ‘(’＜字符串＞ ‘)’| printf ‘(’＜表达式＞‘)’
void parser::writeStatement() {
	string tmpstrname;
	string tmpvarname;
	string tmptypename;
	if (sym != printfsy) {
		ERR.Err(55);
		errset.clear();
		errset.insert(semicolon);
		errjump(errset);
	}
	else {
		lex.getsym();
		if (sym != lparent) {
			ERR.Err(11);
			errset.clear();
			errset.insert(semicolon);
			errjump(errset);
		}
		else {
			lex.getsym();
			if (sym == stringcon) {
				tmpstrname = nextStrCstName();
				str = "\"" + str + "\"";		// 添加引号
				// 将该字符串存至全局
				// 格式：cst,	str,	MSG,	"abc";
				codetab.emitdata(Opt::cst, "str", tmpstrname, str);
				codetab.emit(Opt::print, "str", tmpstrname, "");
				lex.getsym();
				if (sym == comma) {
					lex.getsym();
										
					isExpTypeChar = true;		// 进入之前确保初始化为true
					tmpvarname = expression();
					tmptypename = (isExpTypeChar == true) ? "char" : "int";		// TODO 检查isExpTypeChar的值是否能保证正确
					// 格式：print, char, $t0,	;
					codetab.emit(Opt::print, tmptypename, tmpvarname, "");
					
					if (sym == rparent) {
						lex.getsym();
					} 
					else {
						ERR.Err(10);		// 容错
					}
				}
				else if(sym == rparent){
					lex.getsym();
				}
				else {
					ERR.Err(56);
					errset.clear();
					errset.insert(semicolon);
					errjump(errset);
				}
			}
			else {
				isExpTypeChar = true;		// 进入之前确保初始化为true
				tmpvarname = expression();
				tmptypename = (isExpTypeChar == true) ? "char" : "int";
				// 格式：print, char, $t0,	;
				codetab.emit(Opt::print, tmptypename, tmpvarname, "");

				if (sym == rparent) {
					lex.getsym();
				}
				else {
					ERR.Err(10);		// 容错
				}
			}
		}
	}
}

// 在这个函数中需要对branchretMark、retMark等进行处理
// 同时通过查找符号表，看一下其返回值和返回类型是否和原来一致
// ＜返回语句＞   ::=  return[‘(’＜表达式＞‘)’] 
void parser::returnStatement()
{
	int index = 0;
	string retval;
	typeEnum tmptype;
	index = symtab.locate(funcName, 0);			// 这里的0是因为没有函数嵌套
	tmptype = symtab.SymbolTable[index].type;

	if (sym == returnsy) {
		lex.getsym();
		if (sym == semicolon) {
			if (tmptype == typeEnum::Void) {
				// 格式：ret,	void,	,	f;
				codetab.emit(ret, "void", "", funcName);		// 此时已经预读
			}
			else {
				ERR.Err(43);									// 此时继续处理，但是已经预读了
			}
		}
		else if (sym == lparent) {
			lex.getsym();
			retval = expression();
			if (sym == rparent) {
				lex.getsym();
			}
			else {
				ERR.Err(10);									// 容错
			}
			if (tmptype == typeEnum::Void) {
				ERR.Err(38);
			}
			else {
				// 格式：ret,	int,	$t0,	f;
				if (tmptype == typeEnum::Char) {
					codetab.emit(ret, "char", retval, funcName);		// 此时已经预读
				}
				else {
					codetab.emit(ret, "int", retval, funcName);			// 此时已经预读
				}
				retMark = true;								// 表示这里有返回语句（并且是有返回值的）
			}
		}
		else {
			ERR.Err(44);
			errset.clear();
			errjump(errset);	// 这时候直接跳至下一行
		}
	}
	else {
		ERR.Err(29);	
		errset.clear();
		errjump(errset);		// 这时候直接跳至下一行
	}
}

string parser::nextTmpVar() {
	string str;
	str.push_back('$');
	str.push_back('t');
	str = str + int2str(tmpvarcode++);
	return str;
}

string parser::nextLab()
{
	string str;
	str = "label" + int2str(tmplabcode++);
	return str;
}

string parser::nextStrCstName()
{
	string  strName;
	strName = "STRCST" + util.int2str(tmpstrcode++);
	return strName;
}

code parser::oppbranchcode(code tmp)
{
	code t;
	if (tmp.opt == beq) {
		t.opt = bne;
	}
	else if (tmp.opt == bne) {
		t.opt = beq;
	}
	else if (tmp.opt == bge) {
		t.opt = blt;
	}
	else if (tmp.opt == bgt) {
		t.opt = ble;
	}
	else if (tmp.opt == ble) {
		t.opt = bgt;
	}
	else if (tmp.opt == blt) {
		t.opt = bge;
	}
	else {
		t.opt = tmp.opt;
	}
	t.var1 = tmp.var1;
	t.var2 = tmp.var2;
	t.var3 = tmp.var3;
	return t;
}

string parser::int2str(int num) {
	if (num == 0)
		return "0";

	string str = "";
	num = num > 0 ? num : -1 * num;

	while (num)
	{
		str = (char)(num % 10 + '0') + str;
		num = num / 10;
	}

	if (num < 0)
		str = "-" + str;
	
	return str;
}

int parser::str2int(string str)
{
	int i, num = 0;
	int len = str.size();

	i = 0;
	if (str[0] == '-')
		i = 1;

	while (i < len)
	{
		num = num * 10 + (int)(str[i] - '0');
		i++;
	}

	if (str[0] == '-')
		num = -1 * num;

	return num;
}

// 跳至set中的符号(此时读到单词恰是集合中元素)，如果没有集合中元素，则读完本行，跳转至下一行第一个单词(相当于预读)
// 如果跳转至下一行，则返回true，否则返回false
bool parser::errjump(set<Symbol> symset) {
	int tmpLineNum = LineNum;
	while (symset.find(sym) == symset.end() && LineNum == tmpLineNum) {
		lex.getsym();
	}
	if (tmpLineNum != LineNum) {
		return true;
	}
	else {
		return false;
	}
}