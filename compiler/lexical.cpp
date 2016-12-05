#include "stdafx.h"
#include "lexical.h"
#include "error.h"

string symbolName[] = {
	"intsy","charsy","voidsy","intcon","charcon","stringcon","mainsy","constsy",
	"ifsy","elsesy","whilesy","switchsy","casesy","defaultsy","scanfsy","printfsy","returnsy",
	"add","sub","mul","divi","lt","gt","lte","gte","eql","neq","becomes",
	"comma","semicolon","colon","lparent","rparent","lbrack","rbrack","lbrace","rbrace","excl","ident","nul","error","eofsy"
};                              // ��������

Symbol ssym[ASCIILen];			// 128��ASCII�ַ������ͱ�
Symbol wsym[ReservedWords];		// ���б����ֵ�����
string word[ReservedWords];		// ���б����ֵ�����

extern FILE * fin;				// Դ�ļ���ָ��
extern error ERR;				// ȡ��ȫ�ֵĴ��������
Symbol sym;                     // ��ǰ���ʵ�����
char ch = ' ';					// ��ǰ���ַ������ұ����ʼ��Ϊ�ո�
char line[MaxLineLen];			// ��ǰ�У��ΪMaxLineLen-1
string token;					// ��ǰ���ʣ�string����
string str;						// �洢printf�е��ַ���ʹ��
string msg;						// ������Ϣ����ʱʹ��
int num = 0;					// �洢���ֻ������ַ���ֵ
int CharCount = -1;				// ��ǰ���е�ָ��
int LineLen = 0;				// ��ǰ�еĳ���
int LineNum = 0;				// ��ǰ�кţ�Դ�ļ���������

lexical::lexical()
{
	lexical::log = fopen("symlog.txt", "w");
	Initial();
}

lexical::~lexical()
{
	fclose(fin);
	fclose(log);	
}

void lexical::Initial() {
	int i = 0;
	for (i = 0; i<ASCIILen; i++) {
		ssym[i] = nul;
	}

	ssym['+'] = add;       ssym['-'] = sub;      ssym['*'] = mul;
	ssym['/'] = divi;       ssym['>'] = gt;         ssym['<'] = lt;
	ssym['='] = becomes;    ssym[','] = comma;      ssym[';'] = semicolon;
	ssym[':'] = colon;      ssym['('] = lparent;    ssym[')'] = rparent;
	ssym['['] = lbrack;     ssym[']'] = rbrack;     ssym['{'] = lbrace;
	ssym['}'] = rbrace;     ssym['!'] = excl;

	word[0] = "case";		word[1] = "char";		word[2] = "const";
	word[3] = "default";	word[4] = "else";		word[5] = "if";
	word[6] = "int";		word[7] = "main";		word[8] = "printf";
	word[9] = "return";		word[10] = "scanf";		word[11] = "switch";
	word[12] = "void";		word[13] = "while";

	wsym[0] = casesy;       wsym[1] = charsy;       wsym[2] = constsy;
	wsym[3] = defaultsy;    wsym[4] = elsesy;       wsym[5] = ifsy;
	wsym[6] = intsy;        wsym[7] = mainsy;       wsym[8] = printfsy;
	wsym[9] = returnsy;     wsym[10] = scanfsy;     wsym[11] = switchsy;
	wsym[12] = voidsy;      wsym[13] = whilesy;

}

// ����������һ��EOF��Symbol����
int lexical::getch() {
	if (CharCount >= LineLen - 1) {
		LineNum++;
		if (feof(fin)) {
			ch = EOF;
			return ch;
		}
		else {
			fgets(line, MaxLineLen, fin);
			CharCount = -1;
			LineLen = strlen(line) - 1;
		}
	}
	CharCount++;
	ch = line[CharCount];
	return ch;
}

int lexical::getsym() {
	int k = 0;
	int tmpLineNum = 0;
	int high, low;
	token = "";

	while (ch == ' ' || ch == '\n' || ch == '\t') {
		getch();
	}
	if (isLetter(ch)) {
		while (isLetter(ch) || isDigit(ch)) {
			token.push_back(ch);
			getch();
		}
		
		low = 0;
		high = ReservedWords - 1;
		do {
			k = (low + high) / 2;
			if (token == word[k]) {
				high = k;
				low = k;
				break;
			}
			else if (token > word[k]) {
				low = k + 1;
			}
			else {
				high = k - 1;
			}
		} while (high >= low);
		if (high == low) {
			sym = wsym[high];
		}
		else {
			sym = ident;
			for (k = 0; k < int(token.length()); k++) {				// ��ʶ�������ִ�Сд
				if ('A' <= token[k] && token[k] <= 'Z') {
					token[k] = token[k] - 'A' + 'a';
				}
			}
		}
	}
	else if (isDigit(ch)) {
		k = 0;
		num = 0;
		sym = intcon;
		do {
			num = num * 10 + ch - '0';
			k = k + 1;
			getch();
		} while (isDigit(ch));
		if (k > MaxNumLen) {
			ERR.Err(30);
		}
	}
	else if (ch == '"') {
		k = 0;
		str = "";
		tmpLineNum = LineNum;
		getch();
		while (' ' <= ch && ch <= '~' && ch != '"' && LineNum == tmpLineNum) {
			str.push_back(ch);
			getch();
		}
		if (ch == '"') {
			sym = stringcon;
			getch();
		}
		else {
			//getch();		����������ţ������Ϊ�Ѿ�Ԥ��
			sym = errorsy;
			ERR.Err(31);
		}
	}
	else if (ch == '\'') {
		getch();
		num = ch;
		if (ch == '+' || ch == '-' || ch == '*' || ch == '/' || isDigit(ch) || isLetter(ch)) {
			sym = charcon;
		}
		else {
			sym = errorsy;
			ERR.Err(32);
		}
		getch();			
		if (ch != '\'') {
			sym = errorsy;
			ERR.Err(33);
		}
		getch();
	}
	else if (ch == '+') {
		sym = add;
		getch();
	}
	else if (ch == '-') {
		sym = sub;
		getch();
	}
	else if (ch == '*') {
		sym = mul;
		getch();
	}
	else if (ch == '/') {
		getch();
		if (ch == '*') {
			do {
				do {
					getch();
				} while (ch != '*');
				do {
					getch();
					if (ch == '/') {
						getch();
						getsym();		// ��Ϊ��ע�ͣ����Եݹ����getsym��ȡ��һ��symbol
						return 0;
					}
				} while (ch == '*');
			} while (ch != '*');
		}
		else {
			sym = divi;
		}
	}
	else if (ch == '!') {
		getch();
		if (ch == '=') {
			sym = neq;
			getch();
		}
		else {
			ERR.Err(34);
		}
	}
	else if (ch == '>') {
		getch();
		if (ch == '=') {
			sym = gte;
			getch();
		}
		else {
			sym = gt;
		}
	}
	else if (ch == '<') {
		getch();
		if (ch == '=') {
			sym = lte;
			getch();
		}
		else {
			sym = lt;
		}
	}
	else if (ch == '=') {
		getch();
		if (ch == '=') {
			sym = eql;
			getch();
		}
		else {
			sym = becomes;
		}
	}
	else if (ch == ',') {
		sym = comma;
		getch();
	}
	else if (ch == ';') {
		sym = semicolon;
		getch();
	}
	else if (ch == ':') {
		sym = colon;
		getch();
	}
	else if (ch == '(') {
		sym = lparent;
		getch();
	}
	else if (ch == ')') {
		sym = rparent;
		getch();
	}
	else if (ch == '[') {
		sym = lbrack;
		getch();
	}
	else if (ch == ']') {
		sym = rbrack;
		getch();
	}
	else if (ch == '{') {
		sym = lbrace;
		getch();
	}
	else if (ch == '}') {
		sym = rbrace;
		getch();
	}
	else if (ch == EOF) {
		sym = eofsy;
		return -1;
	}
	else {
		getch();
		sym = errorsy;
		ERR.Err(35);
	}

	PrintInfo();
	return 0;
}


void lexical::PrintInfo() {
	int i = 0;
	fprintf(log, "%s : ", symbolName[sym].c_str());
	if (DEBUG && LexDEBUG) {
		cout << symbolName[sym] << " : ";
	}
	if (sym == ident) {
		fprintf(log, "%s\n", token.c_str());
		if (DEBUG && LexDEBUG) {
			cout << token << endl;
		}
	}
	else if (sym == intcon) {
		fprintf(log, "%d\n", num);
		if (DEBUG && LexDEBUG) {
			cout << num << endl;
		}
	}
	else if (sym == charcon) {
		fprintf(log, "%c\n", num);
		if (DEBUG && LexDEBUG) {
			cout << char(num) << endl;
		}
	}
	else if (sym == stringcon) {
		fprintf(log, "%s\n", str.c_str());
		if (DEBUG && LexDEBUG) {
			cout << str << endl;
		}
	}
	else if (sym == lte) {
		fprintf(log, "<=\n");
		if (DEBUG && LexDEBUG) {
			cout << "<=" << endl;
		}
	}
	else if (sym == gte) {
		fprintf(log, ">=\n");
		if (DEBUG && LexDEBUG) {
			cout << ">=" << endl;
		}
	}
	else if (sym == eql) {
		fprintf(log, "==\n");
		if (DEBUG && LexDEBUG) {
			cout << "==" << endl;
		}
	}
	else if (sym == neq) {
		fprintf(log, "!=\n");
		if (DEBUG && LexDEBUG) {
			cout << "!=" << endl;
		}
	}
	else {
		for (i = 0; i<ReservedWords; i++) {
			if (wsym[i] == sym) {
				if (DEBUG && LexDEBUG) {
					cout << word[i] << endl;
				}
				fprintf(log, "%s\n", word[i].c_str());
				return;
			}
		}

		for (i = 32; i<ASCIILen; i++) {
			if (ssym[i] == sym) {
				if (DEBUG && LexDEBUG) {
					cout << char(i) << endl;
				}
				fprintf(log, "%c\n", i);
				return;
			}
		}
	}

}

int lexical::isLetter(char c) {
	if (c == '_' || ('A' <= c && c <= 'Z') || ('a' <= c && c <= 'z')) {
		return 1;
	}
	return 0;
}

int lexical::isDigit(char c) {
	if ('0' <= c && c <= '9') {
		return 1;
	}
	return 0;
}