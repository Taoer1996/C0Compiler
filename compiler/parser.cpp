#include "stdafx.h"
#include "parser.h"


int lev = 0;				// ��ǰ���ڲ��
bool retMark = false;		// return����mark������з���ֵ��Ϊtrue������Ϊfalse������һ����Ҫ�ı�ʶ����
bool isExpTypeChar = true;	// ��ʶ���ʽ�����ͣ�ֻ��ȫΪchar���͵ı���������û�г˳����������ʽ����char���ͣ�ÿ�γ�expression()֮�����isExpTypeChar�Ϳ��жϱ��ʽ����
string funcName = "";		// ��ʾ��ǰ���ڵĺ��������֣���return�����ʹ��

set<Symbol> errset;
string tmpSymName[3] = { "int","char","void" };

myutils util;				// ������

parser::parser()
{
	parser::tmpvarcode = 0;
	parser::tmplabcode = 0;
	parser::tmpstrcode = 0;
}


parser::~parser()
{
}

// TODO ���������Ƿ�����
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
			// ��ʽ��var,	int,	name,	;
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
							ERR.Err(3);   // �˴��䱨�����Ǽ������룬���ݴ���
						}
						symtab.enter(tmptoken, kindEnum::arrkind, tmptype, lev, tmpnum, 0, 0, 0);
						// ��ʽ��arr,	int,	name,	size;
						codetab.emitdata(Opt::arr, tmpSymName[tmpsym], tmptoken, int2str(tmpnum));
					}
					else {
						ERR.Err(4);
						errset.clear();
						errset.insert(comma);
						errset.insert(semicolon);	
						errjump(errset);			// ��������СӰ�췶Χ
					}
				}
				else {
					ERR.Err(5);
					errset.clear();
					errset.insert(comma);
					errset.insert(semicolon);
					errjump(errset);			// ��������СӰ�췶Χ
				}
			}
			if (sym == semicolon) {
				lex.getsym();
			}
			else {
				ERR.Err(6);       // �������ݴ���
			}
		}

		else if (sym == lparent) {
			goto FuncDefStart;
		}
		
		else {
			ERR.Err(7);
			errset.clear();
			errset.insert(semicolon);
			errjump(errset);			// ��������СӰ�췶Χ
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
			errjump(errset);			// ��������СӰ�췶Χ
		}
		
		tmptoken = token;
		lastFuncName = token;
		lex.getsym();
		if (sym == lparent) {
		FuncDefStart:
			// ���ñ�ǩ
			tmpindex  =  symtab.enter(tmptoken, kindEnum::funckind, tmptype, 0, 0, 0, 0, 0);		//  ����paranum
			// ��ʽ��func,	int,	f,	;
			codetmpindex = codetab.emit(Opt::func, tmpSymName[tmpsym], tmptoken, "");

			funcName = tmptoken;			// ���õ�ǰ���ڵĺ�����
			parser::tmpvarcode = 0;			// ÿ�ν����º������������·�����ʱ����
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
					errjump(errset);			// ��������СӰ�췶Χ
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
					
					// ��ʽ��setlab,		,	,	end_f;
					// ��ʽ��end,	func,	f,	;
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
					ERR.Err(8);       // ���ﲻ���ݴ���ֱ�ӱ���
				}
				lev--;
				funcName = "";		// ����ǰ���ں��������
				symtab.clean();		// ������ʱ����
			}
			else {
				ERR.Err(10);      //����Ҳ�����ݴ�������ֱ�ӱ���
			}
		}
		else {
			ERR.Err(11);          // ���ﲻ���ݴ�����ֱ�ӱ������Ѿ�Ԥ����
		}
	}

	if (!(lastFuncName == "main" && lastFuncType == voidsy)) {
		ERR.Err(18);
	}
		
	// �����ɵ���Ԫʽ��ӡ���ļ���
	codetab.logCode();
}

// ÿ�ν���constDeclaration֮ǰ���Ѿ���������const��ͷ����
// isData�����Ƿ�Ϊ���ݶ���Ԫʽ
void parser::constDeclaration(bool isData) {
	if (sym == constsy) {
		lex.getsym();
		constDefine(isData);
		if (sym == semicolon) {
			lex.getsym();
		}
		else {
			ERR.Err(6);       //�ݴ���
		}
		while (sym == constsy) {
			lex.getsym();
			constDefine(isData);
			if (sym == semicolon) {
				lex.getsym();
			}
			else {
				ERR.Err(6);       //�ݴ���
			}
		}
	}
}

// isData�����Ƿ�Ϊȫ�����ݶ�
void parser::constDefine(bool isData) {
	typeEnum tmptype;
	Symbol tmpsym;
	string tmptoken;
	int tmpnum;
	if (sym == intsy || sym == charsy) {
		tmpsym = sym;
		tmptype = (tmpsym == intsy) ? typeEnum::Int : typeEnum::Char;
		lex.getsym();
		if (sym != ident) {                           // ����
			ERR.Err(2);
			errset.clear();
			errset.insert(comma);
			errset.insert(semicolon);
			errjump(errset);
			if (sym == comma) {						//������һ����������������
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
					ERR.Err(12);       //�ݴ���
				}
				lex.getsym();
				if (tmpsym == intsy) {
					tmpnum = integer();
					symtab.enter(tmptoken, kindEnum::cstkind, tmptype, lev, 0, 0, 0, tmpnum);
					// ��ʽ��cst,	int,	name,	value;
					// �ַ�����Ҳʹ��ֵ����
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
						// ��ʽ��cst,	int,	name,	value;
						// �ַ�����Ҳʹ��ֵ����
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
								ERR.Err(12);       //�ݴ���
							}
							lex.getsym();
							if (tmpsym == intsy) {
								tmpnum = integer();
								symtab.enter(tmptoken, kindEnum::cstkind, tmptype, lev, 0, 0, 0, tmpnum);
								// ��ʽ��cst,	int,	name,	value;
								// �ַ�����Ҳʹ��ֵ����
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
									// ��ʽ��cst,	int,	name,	value;
									// �ַ�����Ҳʹ��ֵ����
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
						ERR.Err(2);           // ����
						errset.clear();
						errset.insert(comma);
						errset.insert(semicolon);
						errjump(errset);
					}
				}
			}
			else {
				ERR.Err(14);                  // ֱ�ӱ���
				errset.clear();
				errset.insert(semicolon);
				errjump(errset);			 // ֱ�Ӷ����ֺ�����
			}
		}
		else {
			ERR.Err(2);
			errset.clear();
			errset.insert(semicolon);
			errjump(errset);	// ֱ�Ӷ�����һ�л��߶����ֺ�
			return;
		}
	}
	else {
		ERR.Err(13);
		errset.clear();
		errset.insert(semicolon);
		errjump(errset);	// ֱ�Ӷ�����һ�л��߶����ֺ�
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

// ������˵����::= ���������壾;{���������壾;}
// ���������壾::= �����ͱ�ʶ����(����ʶ����|����ʶ������[�����޷�����������]��){,����ʶ����|����ʶ������[�����޷�����������]�� }
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
				ERR.Err(6);       // �ݴ���
			}
		}
	}
	else {
		ERR.Err(6);       //�ݴ���
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
			// ��ʽ��var,	int,	name,	;
			codetab.emit(Opt::var, tmpSymName[tmpsym], tmptoken, "");

			while (sym == comma) {
				lex.getsym();
				tmptoken = token;
				lex.getsym();
				if (sym == comma || sym == semicolon) {
					symtab.enter(tmptoken, kindEnum::varkind, tmptype, lev, 0, 0, 0, 0);
					// ��ʽ��var,	int,	name,	;
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
							ERR.Err(3);   // �˴��䱨�����Ǽ������룬���ݴ���
						}
						symtab.enter(tmptoken, kindEnum::arrkind, tmptype, lev, tmpnum, 0, 0, 0);
						// ��ʽ��arr,	int,	name,	size;
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
	if (sym == rparent) {     // Ϊ��
		return 0;
	}
	else if (sym == intsy || sym == charsy) {
		tmpsym = sym;
		tmptype = (sym == intsy) ? typeEnum::Int : typeEnum::Char;
		lex.getsym();
		if (sym == ident) {
			symtab.enter(token, kindEnum::parakind, tmptype, lev, 0, 0, 0, 0);
			// ��ʽ��para,	int,	name,	;
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
					// ��ʽ��para,	int,	name,	;
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
	����䣾 ::= ��������䣾����ѭ����䣾| ��{��������У���}�������з���ֵ����������䣾; | ���޷���ֵ����������䣾;
	            ������ֵ��䣾;��������䣾;����д��䣾;�����գ�;|�������䣾����������䣾;
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
			ERR.Err(8);       // �ݴ�
		}
	}
	else if (sym == scanfsy) {
		readStatement();
		if (sym == semicolon) {
			lex.getsym();
		}
		else {
			ERR.Err(6);			// �ݴ����൱��Ԥ��
		}
	}
	else if (sym == printfsy) {
		writeStatement();
		if (sym == semicolon) {
			lex.getsym();
		}
		else {
			ERR.Err(6);			// �ݴ����൱��Ԥ��
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
			ERR.Err(6);			// �ݴ����൱��Ԥ��
		}
	}
	else if (sym == semicolon) {
		lex.getsym();
	}
	else if (sym == ident) {
		// ��������Ǻ������ú͸�ֵ���
		tmptoken = token;
		lex.getsym();
		pos = symtab.locate(tmptoken, lev);
		if (pos == -1) {
			ERR.Err(45);
			errset.clear();
			errjump(errset);			// �����һ������δ����ı�ʶ������ֱ����������
			return;
		}

		// ˵���Ǻ�������
		if (sym == lparent) {
			if (pos == -1) {
				ERR.Err(37);
				errset.clear();
				errjump(errset);	// ֱ��������һ��
				return;
			}

			if (symtab.SymbolTable[pos].kind != kindEnum::funckind) {
				ERR.Err(48);
			}
			// ��ʽΪ��call,	void,	name,	;
			// ��Ϊ��û�и�ֵ�ĵ��ã�ͬʱ����ĺ���callָ���ڲ�����ú����
			codetab.emit(Opt::call, "void", tmptoken, "");

			lex.getsym();
			paras = valList(tmptoken);
			if (paras != symtab.SymbolTable[pos].paranum) {
				ERR.Err(40);		// �������룬����������г���Ļ������ȷ��
			}

			if (sym == rparent) {
				lex.getsym();
			}
			else {
				ERR.Err(10);		// �ݴ�
			}
		}
		// ˵�������飬Ҳ��Ϊ��ֵ���
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
				ERR.Err(3);					// �ݴ���
			}
			else if (sym == rbrack) {
				lex.getsym();
			}
			if (sym == becomes || sym == eql) {
				if (sym == eql) {
					ERR.Err(12);				// �ݴ���
				}
				lex.getsym();
				tmpstr2 = expression();
				// ��ʽ��sarr,	$t0,	arrname,	index;
				codetab.emit(Opt::sarr, tmpstr2, tmptoken, tmpstr1);
			}
			else {
				ERR.Err(14);
				errset.clear();
				errset.insert(semicolon);
				errjump(errset);
			}
		}
		// ֱ���Ǹ�ֵ���
		else if (sym == becomes || sym == eql) {
			if (sym == eql) {
				ERR.Err(12);				// �ݴ���
			}
			tmpkind = symtab.SymbolTable[pos].kind;

			if (tmpkind == kindEnum::cstkind) {
				ERR.Err(51);
			}
			else if (tmpkind == kindEnum::varkind || tmpkind == kindEnum::parakind) {
				lex.getsym();
				tmpstr1 = expression();
				// ��ʽ��move,	x,	$t0,	;
				codetab.emit(Opt::move, tmptoken, tmpstr1, "");
			}
			else {
				ERR.Err(60);				// ��Ȼ�Ǵ�ģ����Ǳ���֮�����ɽ�����ȥ
				lex.getsym();
				expression();
			}
		}
		else {
			ERR.Err(41);
			errset.clear();
			errset.insert(semicolon);
			errjump(errset);	//ֱ�Ӵ������һ��
		}

		// ��ֵ���ͺ������������������Ҫ����ֺ�
		if (sym == semicolon) {
			lex.getsym();
		}
		else {
			ERR.Err(6);			// �ݴ����൱��Ԥ��
		}
	}
	else {
		ERR.Err(19);
		errset.clear();
		errjump(errset);		// �����ֱ��Ԥ��
	}
}

// ��������䣾  ::=  if ��(������������)������䣾��else����䣾��
// ע��ifStatement�л���Ҫ���Ƿ���ֵ������
void parser::ifStatement() {
	bool tmpretMark;				// �����ݴ�ȫ�ֵ�retMark��־
	bool ifretMark = true;			// if����е�retMark��־����ʼ��Ϊtrue
	int index1 = 0;
	int index2 = 0;
	string labelname1;
	string labelname2;
	code tmp;				// ���ڱ���condition()���ص�code
	if (sym == ifsy) {
		lex.getsym();
		if (sym == lparent) {
			lex.getsym();
			tmp = condition();
			tmp = oppbranchcode(tmp);
			// ��ʽ�� beq,	$t0,	$t1,	label; (ע���label����)
			index1 = codetab.emit(tmp.opt, tmp.var1, tmp.var2, tmp.var3);
			if (sym == rparent) {
				lex.getsym();
			}
			else {
				ERR.Err(10);          //�˴���Ϊ�ݴ���
			}

			tmpretMark = retMark;		// �ݴ�ȫ�ֵ�retMark��֮��ָ�
			
			retMark = false;
			statement();				// �������return��䣬��retMark������true
			ifretMark = ifretMark && retMark;		// ��������з���ֵ��return��䣬��retMarkΪtrue��ifretMarkֵ���䣬����Ϊfalse

			if (sym == elsesy) {
				// ��ʽ��j,	,	,	label;
				index2 = codetab.emit(Opt::j, "", "", "");		// ����Ҳ��Ҫ����label
				lex.getsym();
				labelname1 = nextLab();
				// ��ʽ��setlab,		,		,label;
				codetab.emit(Opt::setlab, "", "", labelname1);
				// ����else���ֵ�label
				codetab.textSeg[index1].var3 = labelname1;

				retMark = false;
				statement();				// �������return��䣬��retMark������true
				ifretMark = ifretMark && retMark;		// ��������з���ֵ��return��䣬��retMarkΪtrue��ifretMarkֵ���䣬����Ϊfalse
				
				labelname2 = nextLab();
				// ��ʽ��setlab,		,		,label;
				codetab.emit(Opt::setlab, "", "", labelname2);
				// ��������if����β��label
				codetab.textSeg[index2].var3 = labelname2;
			}
			else {
				labelname1 = nextLab();
				// ��ʽ��setlab,		,		,label;
				codetab.emit(setlab, "", "", labelname1);
				// ��������if����β��label
				codetab.textSeg[index1].var3 = labelname1;

				ifretMark = ifretMark && false;		// ��Ϊȱʡelse���൱����ȱ����else�е�return��䣬����ֱ��&&false
			}

			retMark = tmpretMark;		// �ָ�ȫ�ֵ�retMark
			retMark = retMark || ifretMark;		// ����ֻ��Ҫ�ǻ��������
		}
		else {
			ERR.Err(11);
			errset.clear();
			errjump(errset);		// ֱ��������һ��
		}
	}
	else {
		ERR.Err(20);
		errset.clear();
		errjump(errset);		// ֱ��������һ��
	}

}

code parser::condition() {
	Opt tmp;
	code tmpcode;
	string tmpstr1, tmpstr2;
	tmpstr1 = expression();
	if(sym == rparent){					// ��������Ǵ����ʽ
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
		tmp = Opt::beq;			// �ݴ���
	}
	if (sym == lt || sym == gt || sym == gte || sym == lte || sym == eql || sym == neq || sym == becomes) {
		lex.getsym();
		tmpstr2 = expression();
		tmpcode.opt = tmp;
		tmpcode.var1 = tmpstr1;
		tmpcode.var2 = tmpstr2;
		// ������Ҫ����label��������Ҫ������Ԫʽ�Թ���д
		return tmpcode;
	}
	else {
		ERR.Err(42);
		expression();
		return tmpcode;
	}
}

// �����ʽ��    ::= �ۣ������ݣ��{���ӷ�����������}
// ֻҪ����int���͵ı������򽫱��ʽ���ͱ�Ϊint(��isExpTypeChar=false);
// expression���ܷ��ص������� : ""���գ�	���֣�123��		������a��	��ʱ������$t0��
string parser::expression() {
	int flag = 1;
	Opt tmp;
	string tmpstr1, tmpstr2;
	string tmpvarstr;
	
	isExpTypeChar = true;		// ��ʼ��Ϊtrue��Ĭ��Ϊchar����

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
		// ��ʽ��mul,	$t1,	$t0,	-1;
		codetab.emit(Opt::mulop, tmpvarstr, tmpstr1, "-1");
		tmpstr1 = tmpvarstr;
	}
	
	while (sym == Symbol::add || sym == Symbol::sub) {
		tmp = (sym == Symbol::add) ? Opt::addop : Opt::subop;
		lex.getsym();
		tmpstr2 = terms();
		tmpvarstr = nextTmpVar();
		// ��ʽ��add,	$t0,	a,	b;
		codetab.emit(tmp, tmpvarstr, tmpstr1, tmpstr2);
		tmpstr1 = tmpvarstr;
	}
	return tmpstr1;
}

// ��� ::= �����ӣ�{���˷�������������ӣ�}
// terms���ܷ��ص������� : ""���գ�	���֣�123��		������a��	��ʱ������$t0��
string parser::terms() {
	Opt tmpopt;
	string tmpstr1, tmpstr2, tmpvarstr;

	tmpstr1 = factor();
	tmpvarstr = tmpstr1;

	while (sym == Symbol::mul || sym == Symbol::divi) {

		isExpTypeChar = false;				// ���ֳ˳�����

		tmpopt = (sym == Symbol::mul) ? Opt::mulop : Opt::diviop;
		lex.getsym();
		tmpstr2 = factor();
		tmpvarstr = nextTmpVar();
		// ��ʽ��mul,	$t2,	$t1,	$t0;
		codetab.emit(tmpopt, tmpvarstr, tmpstr1, tmpstr2);
		tmpstr1 = tmpvarstr;
	}
	return tmpvarstr;
}


/* 
	������Ҫ�Ա��������ͺ�lev�Ƚ�����ϸ�ļ��ͱ�����������Ϊ���һ������
	��Ҫ��飺
	1�������Ƿ�Խ��
	1������/�����Ƿ��壨�����Ļ�ֱ�ӷ���ֵ��
	3���Ƿ�Ϊ�з���ֵ��������
	4���Ƿ���ֳ�����������������ֱ��ͨ��ֵ���жϣ�������ȷ����
	�����ӣ� ::= ����ʶ����������ʶ������[�������ʽ����]������������|���ַ��������з���ֵ����������䣾|��(�������ʽ����)��
 */

// factor���ܷ����е��У� ""���գ�	���֣�123��		������a��	��ʱ������$t0��		
string parser::factor() {
	int index;
	int tmpnum;
	string tmpstr;
	bool tmpExpTypeChar = true;
	string tmpvarstr, tmptypename;
	kindEnum tmpkind;
	string tmptoken;
	// ����Ǳ�ʶ����������Ǳ��������������顢��������
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
		// ����Ԫ��
		if (sym == lbrack) {
			
			lex.getsym();

			tmpExpTypeChar = isExpTypeChar;
			tmpstr = expression();
			isExpTypeChar = tmpExpTypeChar;			// ��Ϊ����ѡԪ��ʱ��������֣��������Ӱ����ʽ������

			
			if (sym == rbrack) {
				lex.getsym();
			}
			else {
				ERR.Err(3);            //�ݴ�
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
			// ��ʽ��larr,	$t1,	arrname,	$t0;
			tmpvarstr = nextTmpVar();
			codetab.emit(Opt::larr, tmpvarstr, tmptoken, tmpstr);
			return tmpvarstr;
		}
		// ��������
		else if (sym == lparent) {
			tmpExpTypeChar = isExpTypeChar;			// ͬ���������ý���������ֵ���ͣ��������п��ܸı�isExpTypeChar��ֵ
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
					// ��ʽ��call,	int,	f,	$t0;
					codetab.emit(Opt::call, tmptypename, tmptoken, tmpvarstr);
				}
			}
			lex.getsym();
			valList(tmptoken);
			if (sym == rparent) {
				lex.getsym();
			}
			else {
				ERR.Err(10);          //�ݴ�
			}
			isExpTypeChar = tmpExpTypeChar;			// �ָ�����ֵ	
			return tmpvarstr;
		}
		// ��ͨ��ʶ��
		else {
			tmpkind = symtab.SymbolTable[index].kind;
			// ����ǳ��������Ѿ������ֵ������ֱ�ӷ���ֵ
			if (tmpkind == kindEnum::cstkind) {
				return util.int2str(symtab.SymbolTable[index].value);
			}
			// ����ǲ�����û��ֵ�����Է�������
			else if (tmpkind == kindEnum::parakind) {
				return tmptoken;
			}
			// �������ͨ����
			else if(tmpkind == kindEnum::varkind){
				return tmptoken;
			}
			else {
				ERR.Err(50);
				return "";
			}
		}
	}
	// ���������
	else if (sym == add || sym == sub || sym == intcon) {
		tmpnum = integer();
		isExpTypeChar = false;
		return util.int2str(tmpnum);
	}
	// ������ַ�����
	else if (sym == charcon) {
		tmpnum = num;
		lex.getsym();
		return util.int2str(tmpnum);
	}
	// ����Ǳ��ʽ
	else if (sym == lparent) {
		lex.getsym();
		
		tmpExpTypeChar = isExpTypeChar;		// �ݴ浱ǰ�����ͣ���Ϊ����expression()֮���ͳһ����Ϊtrue

		tmpstr = expression();
		if (sym == rparent) {
			lex.getsym();
		}
		else {
			ERR.Err(10);                //�ݴ�
		}
		
		if (isExpTypeChar == true) {
			isExpTypeChar = tmpExpTypeChar;		// ����ñ��ʽΪchar���ͣ���ȫ�ֵ�isExpTypeChar��ԭ����һ�£�ֱ�ӻָ�����
		}										// ����ȫ�ֵı��ʽ����int���ͣ������ֱ����isExpTypeCharΪfalse�����Բ����κβ���
		return tmpstr;
	}
	// ���ʲô������
	else {
		ERR.Err(59);
		lex.getsym();		// ������һ��token
		return "";
	}
}

// ��ѭ����䣾   ::=  while ��(������������)������䣾
void parser::loopStatement() {
	code tmpcode;
	int index1;
	string tmplabel1, tmplabel2;
	if (sym == whilesy) {
		lex.getsym();
		tmplabel1 = nextLab();
		// ��ʽ��setlab,		,		,label;
		codetab.emit(Opt::setlab, "", "", tmplabel1);

		if (sym == lparent) {
			lex.getsym();
			tmpcode = condition();
			tmpcode = oppbranchcode(tmpcode);
			// ����index����
			// ��ʽ��blt,	$t0,	10,		label;
			index1 = codetab.emit(tmpcode.opt, tmpcode.var1, tmpcode.var2, tmpcode.var3);

			if (sym != rparent) {
				ERR.Err(10);      //�ݴ�
			}
			else {
				lex.getsym();
			}
			statement();
			// ��ʽ��j,		,		,		label;
			codetab.emit(Opt::j, "", "", tmplabel1);
			tmplabel2 = nextLab();
			// ��ʽ��setlab,		,		,		label;
			codetab.emit(Opt::setlab, "", "", tmplabel2);
			// �����˳�ѭ����label
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
	�˲��ָ�д�ķ���
	�������䣾  ::=  switch ��(�������ʽ����)�� ��{���������������}��
	�������������::=�����������䣾{���������䣾}�ۣ�ȱʡ����
	���������䣾  :: = case��������������䣾
	��ȱʡ��   :: = default: ����䣾
*/
// ͬʱע�⿼�Ƿ�֧����ֵ���ֵ����
void parser::switchStatement() {
	string tmpstr;
	string tmplab;
	bool switchretMark = false;
	bool tmpretMark;				// �ݴ�ȫ�ֵ�retMark

	tmpretMark = retMark;

	retMark = false;

	if (sym == switchsy) {
		lex.getsym();
		if (sym == lparent) {
			lex.getsym();
			tmpstr = expression();
			tmplab = nextLab();		// �����Ϊ�������Ľ�βlabel

			if (sym == rparent) {
				lex.getsym();
			}
			else {
				ERR.Err(10);        //�ݴ�
			}

			if (sym != lbrace) {      //��������һ��
				ERR.Err(9);
				errset.clear();
				errjump(errset);
			}
			else {
				lex.getsym();	
			}

			caseDescription(tmpstr, tmplab);		// ��ѡ��ֵ���룬�������Ľ�βlabel����

			if (sym == rbrace) {
				lex.getsym();
			}
			else {
				ERR.Err(8);       //�ݴ�
			}
			// ��ʽ��setlab,		,		,	label_end;
			codetab.emit(Opt::setlab, "", "", tmplab);			// ���ý�βlabel
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

	switchretMark = retMark;		// �õ�switch����������Ƿ���retMark;
	tmpretMark = tmpretMark || switchretMark;	// ��ԭ����retMark���л����
	retMark = tmpretMark;			// �õ�������switch���֮������������retMark���
}


// �������������::=�����������䣾{���������䣾}�ۣ�ȱʡ����
// ���������䣾  :: = case��������������䣾
// ��ȱʡ��   :: = default: ����䣾
void parser::caseDescription(string selectVar, string endLabel)
{
	int index = 0;
	string tmplab;
	bool tmpretMark;
	bool caseretMark = true;
	
	tmpretMark = retMark;			// �ݴ�ȫ��retMark

	retMark = false;
	index = caseStatement(selectVar, endLabel, -1);			// �˴�-1�������caseΪ��һ��case������û�д�����û������һ��case�Ĵ���λ��
	caseretMark = caseretMark && retMark;				// ��ʱ���������ֻ�е����е�case����ж���returnʱ��caseretMark��Ϊtrue

	while (sym == casesy) {
		retMark = false;
		index = caseStatement(selectVar, endLabel, index);
		caseretMark = caseretMark && retMark;
	}

	if (sym == defaultsy) {
		retMark = false;

		tmplab = nextLab();
		// ����default label
		// ��ʽ��setlab,		,		,	label;
		codetab.emit(Opt::setlab, "", "", tmplab);
		codetab.textSeg[index].var3 = tmplab;			// ������һ��case����ת��ǩ
		lex.getsym();
		if (sym == colon) {
			lex.getsym();
		}
		else {
			ERR.Err(24);
			errset.clear();
			errset.insert(colon);
			errjump(errset);
			if (sym == colon) {			// ��תð��֮��
				lex.getsym();
			}
		}
		statement();
		// ����ֱ����ת��switch����
		codetab.emit(Opt::j, "", "", endLabel);

		caseretMark = caseretMark && retMark;
	}
	else {
		codetab.textSeg[index].var3 = endLabel;			// ���û��ȱʡ��ҲҪ���ܣ���һ�����Ҫ
		// ����case������ö�٣������������retMarkҪ������ô�ϣ����Բ�����
		caseretMark = caseretMark && false;				// ��Ϊȱ��Default��ȱʧ��һ�������������Ϊ�����caseretMarkΪfalse
	}

	tmpretMark = tmpretMark || caseretMark;			// ������ǻ������
	retMark = tmpretMark;							// ���»ָ�ȫ��retMark
}

// ���������䣾  :: = case��������������䣾
// �����е�selectVarΪѡ�������endLabelΪswitch����еĽ�β��label��
// lastIndexΪ��һ��case��ת����ڴ����е�λ�ã���Ҫ�����ͬʱ���ر�case��ת����ڴ������λ��
int parser::caseStatement(string selectVar, string endLabel,int lastIndex) {
	int tmpval = 0;
	int index = -1;
	string tmplab;
	if (sym == casesy) {
		// ���������ǩ
		tmplab = nextLab();
		codetab.emit(Opt::setlab, "", "", tmplab);
		if (lastIndex != -1) {
			codetab.textSeg[lastIndex].var3 = tmplab;
		}
		lex.getsym();
		if (sym == add || sym == sub || sym == intcon || sym == charcon) {
			// ���ַ�����
			if (sym == charcon) {
				tmpval = num;
				lex.getsym();
			}
			// ��������������
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
			ERR.Err(24);          //�ݴ�
		}
		else {
			lex.getsym();
		}
		// ��ʽ��bne,	$t0,	1,		label;
		index = codetab.emit(Opt::bne, selectVar, util.int2str(tmpval), "");			// ��ǩ������
		statement();
		// ������ת����β����ת���
		codetab.emit(Opt::j, "", "", endLabel);
	}
	else {
		ERR.Err(23);
		errset.clear();
		errjump(errset);
	}
	return index;
}

// ��ֵ������ ::= �����ʽ��{,�����ʽ��}�����գ�
// ͬʱ���ز����ĸ���
int parser::valList(string fName)
{
	int index = 0;
	int paNum = 0;
	string tmpvar;
	typeEnum type;
	string typeName;

	index = symtab.locate(fName, 0);		// �õ������ڷ��ű��е�λ��
	if (sym == rparent) {
		return 0;			// ����ҪԤ��
	}
	else {
		tmpvar = expression();
		paNum++;
		if (symtab.SymbolTable[index + paNum].kind == kindEnum::parakind) {
			type = symtab.SymbolTable[index + paNum].type;
		}
		typeName = (type == typeEnum::Char) ? "char" : "int";
		// ��ʽ��parain,	 int,	$t0,	f;	
		codetab.emit(Opt::parain, typeName, tmpvar, fName);
		while (sym == comma) {
			lex.getsym();
			tmpvar = expression();
			paNum++;
			if (symtab.SymbolTable[index + paNum].kind == kindEnum::parakind) {
				type = symtab.SymbolTable[index + paNum].type;
			}
			typeName = (type == typeEnum::Char) ? "char" : "int";
			// ��ʽ��parain,	 int,	$t0,	f;		
			codetab.emit(Opt::parain, typeName, tmpvar, fName);
		}
	}
	return paNum;
}

// ������䣾 ::=  scanf ��(������ʶ����{,����ʶ����}��)��
// ע��scanf�еı�ʶ������������ͷ
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
				errset.insert(ident);				// ������һ����ʶ��������������
				errset.insert(rparent);				// �Ӷ�ʹ�ÿ��Լ�������
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
						// ��ʽ��scan,	int,	var,	;
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
								// ��ʽ��scan,	int,	var,	;
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
					ERR.Err(10);          //�ݴ�
				}
			}
			// ���ֱ�Ӷ���������
			else if(sym == rparent){
				ERR.Err(54);
				lex.getsym();
			}
			// ���ֱ�Ӷ���������
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


// ��д��䣾 ::= printf ��(�� ���ַ�����,�����ʽ�� ��)��| printf ��(�����ַ����� ��)��| printf ��(�������ʽ����)��
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
				str = "\"" + str + "\"";		// �������
				// �����ַ�������ȫ��
				// ��ʽ��cst,	str,	MSG,	"abc";
				codetab.emitdata(Opt::cst, "str", tmpstrname, str);
				codetab.emit(Opt::print, "str", tmpstrname, "");
				lex.getsym();
				if (sym == comma) {
					lex.getsym();
										
					isExpTypeChar = true;		// ����֮ǰȷ����ʼ��Ϊtrue
					tmpvarname = expression();
					tmptypename = (isExpTypeChar == true) ? "char" : "int";		// TODO ���isExpTypeChar��ֵ�Ƿ��ܱ�֤��ȷ
					// ��ʽ��print, char, $t0,	;
					codetab.emit(Opt::print, tmptypename, tmpvarname, "");
					
					if (sym == rparent) {
						lex.getsym();
					} 
					else {
						ERR.Err(10);		// �ݴ�
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
				isExpTypeChar = true;		// ����֮ǰȷ����ʼ��Ϊtrue
				tmpvarname = expression();
				tmptypename = (isExpTypeChar == true) ? "char" : "int";
				// ��ʽ��print, char, $t0,	;
				codetab.emit(Opt::print, tmptypename, tmpvarname, "");

				if (sym == rparent) {
					lex.getsym();
				}
				else {
					ERR.Err(10);		// �ݴ�
				}
			}
		}
	}
}

// �������������Ҫ��branchretMark��retMark�Ƚ��д���
// ͬʱͨ�����ҷ��ű���һ���䷵��ֵ�ͷ��������Ƿ��ԭ��һ��
// ��������䣾   ::=  return[��(�������ʽ����)��] 
void parser::returnStatement()
{
	int index = 0;
	string retval;
	typeEnum tmptype;
	index = symtab.locate(funcName, 0);			// �����0����Ϊû�к���Ƕ��
	tmptype = symtab.SymbolTable[index].type;

	if (sym == returnsy) {
		lex.getsym();
		if (sym == semicolon) {
			if (tmptype == typeEnum::Void) {
				// ��ʽ��ret,	void,	,	f;
				codetab.emit(ret, "void", "", funcName);		// ��ʱ�Ѿ�Ԥ��
			}
			else {
				ERR.Err(43);									// ��ʱ�������������Ѿ�Ԥ����
			}
		}
		else if (sym == lparent) {
			lex.getsym();
			retval = expression();
			if (sym == rparent) {
				lex.getsym();
			}
			else {
				ERR.Err(10);									// �ݴ�
			}
			if (tmptype == typeEnum::Void) {
				ERR.Err(38);
			}
			else {
				// ��ʽ��ret,	int,	$t0,	f;
				if (tmptype == typeEnum::Char) {
					codetab.emit(ret, "char", retval, funcName);		// ��ʱ�Ѿ�Ԥ��
				}
				else {
					codetab.emit(ret, "int", retval, funcName);			// ��ʱ�Ѿ�Ԥ��
				}
				retMark = true;								// ��ʾ�����з�����䣨�������з���ֵ�ģ�
			}
		}
		else {
			ERR.Err(44);
			errset.clear();
			errjump(errset);	// ��ʱ��ֱ��������һ��
		}
	}
	else {
		ERR.Err(29);	
		errset.clear();
		errjump(errset);		// ��ʱ��ֱ��������һ��
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

// ����set�еķ���(��ʱ��������ǡ�Ǽ�����Ԫ��)�����û�м�����Ԫ�أ�����걾�У���ת����һ�е�һ������(�൱��Ԥ��)
// �����ת����һ�У��򷵻�true�����򷵻�false
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