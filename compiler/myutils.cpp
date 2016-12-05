#include "stdafx.h"
#include "myutils.h"


myutils::myutils()
{
}


myutils::~myutils()
{
}

string myutils::int2str(int num) {
	int tmpnum = 0;
	if (num == 0)
		return "0";

	string str = "";
	tmpnum = num > 0 ? num : -1 * num;

	while (tmpnum)
	{
		str = (char)(tmpnum % 10 + '0') + str;
		tmpnum = tmpnum / 10;
	}

	if (num < 0)
		str = "-" + str;

	return str;
}

int myutils::str2int(string str)
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

bool myutils::isInt(string s)
{
	int i = 0;
	if (s.length() == 0) {
		return false;
	}
	else {
		if (s[0] == '+') {
			i = 1;
		}
		else if (s[0] == '-') {
			i = 1;
		}
		while (i < int(s.length())) {
			if ('0' <= s[i] && s[i] <= '9') {
				i++;
			}
			else {
				break;
			}
		}
		if (i == int(s.length())) {
			return true;
		}
		else {
			return false;
		}
	}
}

bool myutils::isIdent(string s)
{
	int i = 0;
	if (s.length() == 0) {
		return false;
	}
	else {
		if (s[0] == '_' || ('a' <= s[0] && s[0] <= 'z') || ('A' <= s[0] && s[0] <= 'Z')) {
			i = 1;
		}
		else {
			return false;
		}
		while (i < int(s.length())) {
			if (s[i] == '_' || ('a' <= s[i] && s[i] <= 'z') || ('A' <= s[i] && s[i] <= 'Z') || ('0' <= s[i] && s[i] <= '9')) {
				i++;
			}
			else {
				break;
			}
		}
		if (i == s.length()) {
			return true;
		}
		else {
			return false;
		}
	}
}

bool myutils::isTmpVar(string s)
{
	int i = 0;
	if (s.length() == 0) {
		return false;
	}
	else {
		if (s[0] == '$') {
			return isIdent(s.substr(1,s.length()-1));
		}
		else {
			return false;
		}
	}
}
