/*
 * parser.h
 *
 *  Created on: Oct 23, 2017
 *      Author: gerardryan
 */

#ifndef PARSER_H_
#define PARSER_H_

#include <iostream>
using std::istream;

#include <string>
#include <sstream>
using std::string;
using std::stoi;
using std::cout;

#include "lexer.h"
extern bool e_flag;
extern void error(int linenum, const string& message);
static std::stringstream ss;

enum TypeForNode { INT_TYPE, STRING_TYPE, ERROR_TYPE , ID_TYPE, ADDITION_TYPE, MULTIPLICATION_TYPE, DIVISION_TYPE, SUBTRACTION_TYPE, SET_TYPE, DECL_TYPE, PRINTLN_TYPE, PRINT_TYPE};
#include <algorithm>
#include <string>
static std::string remove_char(std::string str, const char ch )
{
    // remove all occurrences of char ch from str
    str.erase(std::remove(str.begin(), str.end(), ch), str.end() ) ;
    return str ;
}
extern void log(string statement);

class Value{
	int linenum;
	int intvalue;
	string stringvalue;
	TypeForNode type;

public:
	Value(int N, string s){linenum = N, intvalue = 0; stringvalue = remove_char(s, '\"'); type = STRING_TYPE;}
	Value(int N, int i){linenum = N, intvalue = i; stringvalue = ""; type=INT_TYPE;}
	Value(){linenum = 0, intvalue = 0, stringvalue = ""; type=ERROR_TYPE;}

	TypeForNode getType() const {return type;}
	int getIntValue() const {return intvalue;}
	string getStringValue() const {return stringvalue;}
	int getLineNumber(){return linenum;}
	Value operator+(Value V2) {
		if getType() == ERROR_TYPE  || V2.getType() == ERROR_TYPE)
		switch (getType()) {
			//If the first is an int,
			//then it's only valid if
			//the second is also an int
			case T_INT:
				if (V2.getType() != INT_TYPE)
					break; //("Tried to add an int and a string!");
				else
					return Value(linenum, getIntValue() + V2.getIntValue());
				break;
			case T_STRING:
				if (V2.getType() != STRING_TYPE)
					break; //("Tried to add a string and an int!");
				else
					return Value(linenum, getStringValue() + V2.getStringValue());
				break;
			default:
				cout << "BROKE";
		}
		return Value();
	}

	//Operator -, only defined for INT
	Value operator-(Value V2) {
		log("SUB");
		if (getType() == INT_TYPE && V2.getType() == INT_TYPE)
			return Value(linenum, getIntValue() - V2.getIntValue());
		error(getLineNumber(), "type error");

		return Value();
	}
	Value operator*(Value V2) {
		if (getType() == INT_TYPE && V2.getType() == INT_TYPE)
			return Value(linenum, getIntValue() * V2.getIntValue());
		else if (getType() == INT_TYPE && V2.getType() == STRING_TYPE) {
			int i = getIntValue();
			string r = V2.getStringValue();
			string b = r;
			for (int k = 1; k < i; k++)
				r += b;
			return Value(linenum, r);
		}
		else if (getType() == STRING_TYPE && V2.getType() == INT_TYPE) {
			int i = V2.getIntValue();
			string r = getStringValue();
			string b = r;
			for (int k = 1; k < i; k++)
				r += b;
			return Value(linenum, r);
		}
		error(getLineNumber(), "type error");
		return Value();
	}
	Value operator/(Value V2) {
		if (getType() == INT_TYPE && V2.getType() == INT_TYPE) {

			if (V2.getIntValue() != 0) //ToDo: Proper Error Statement
				return Value(linenum, getIntValue() / V2.getIntValue());
		}
		else if (getType() == STRING_TYPE && V2.getType() == STRING_TYPE) {
			string s1 = getStringValue();
			string s2 = V2.getStringValue();
			string::size_type l = s1.find(s2);
			if (l != string::npos){
				log("l = npos");
				s1.erase(s1.begin() + l, s1.begin()+l+s2.length());
				return Value(linenum, s1);
			}
			else
				return Value(linenum, s1);

		}
		error(getLineNumber(), "type error");
		return Value();
	}

};
static ostream& operator<<(ostream& out, Value V) {
	if (V.getType() == STRING_TYPE)
		out << V.getStringValue();
	else if(V.getType() == INT_TYPE)
		out << V.getIntValue();
	else
		log("Print fial");
	return out;
}
class ParseTree {
	int			linenumber;
	ParseTree	*left;
	ParseTree	*right;

public:
	ParseTree(int n, ParseTree *l = 0, ParseTree *r = 0) : linenumber(n), left(l), right(r) {}
	virtual ~ParseTree() {}

	ParseTree* getLeft() const { return left; }
	ParseTree* getRight() const { return right; }
	int getLineNumber() const { return linenumber; }

	virtual TypeForNode GetType() const { return ERROR_TYPE; }
	virtual int GetIntValue() const { return 0;}//throw "no integer value"; }
	virtual string GetStringValue() const { throw "no string value"; }
	virtual Value eval() {if (getLeft()) getLeft()->eval();  if (getRight()) getRight()->eval(); return Value();}

};
extern map<string, Value> mastertable;
extern void log(string step, Token type, int c = 0);
extern void log(string statement, ParseTree* PT);
extern void log(ParseTree* PT, string statement = "");
extern void log(string statmenet, Value V);

//Other Nodes
class StatementList : public ParseTree {
public:
	StatementList(ParseTree *first, ParseTree *rest) : ParseTree(0, first, rest) {}
	virtual Value eval() override {
		log(this);
		getLeft()->eval();
		if (getRight())
			getRight()->eval();
		log("Printing statement");
		if (!e_flag){
			cout << ss.str();
			e_flag = true;
		}
		return Value();
	}

};

class IDNode : public ParseTree {
	string Identifier;
public:
	IDNode(int n, string identifier, ParseTree *l = 0, ParseTree *r = 0) : ParseTree(n, l, r) {Identifier = identifier;}
	virtual TypeForNode GetType() const { return ID_TYPE; }
	virtual string GetStringValue() const override {return Identifier;}
	virtual Value eval() override{log(this); return mastertable[Identifier];}

};
class PrintNode : public ParseTree {
public:
	PrintNode(int line, ParseTree *l) : ParseTree(line, l){}
	virtual Value eval() override {
		log(this);
		if (getLeft()) {
			Value V = getLeft()->eval();
			if (V.getType() == ERROR_TYPE) {
				//error(getLineNumber(), "type error");
				return Value();
			}

		ss << getLeft()->eval();
		}
		return Value();
	}
	virtual TypeForNode GetType() const override{return PRINT_TYPE;}
};

class PrintLNode : public PrintNode{
public:
	PrintLNode(int line, ParseTree *l) : PrintNode(line, l){}
	virtual Value eval() override{log(this); ss << getLeft()->eval() << '\n'; return Value();}
	virtual TypeForNode GetType() const override{return PRINTLN_TYPE;}

};

class DeclNode : public ParseTree {
	int dt;
	string id;
public:
	DeclNode(int line, string ID, char D) : ParseTree(line) { dt = D; id = ID;}
	TypeForNode GetType() const override { return DECL_TYPE; }
	virtual int GetIntValue() const override {return dt;}
	virtual string GetStringValue() const override{return id;}

};

class SetNode : public ParseTree {
	string id;
	Value set;
public:
	SetNode(int line, string ID, ParseTree* l ) : ParseTree(line, l) { id = ID;}
	virtual TypeForNode GetType() const override { return SET_TYPE; }
	virtual string GetStringValue() const override {return id;}
	virtual Value eval() override {
		log(this);
		log(GetStringValue());
		Value V = getLeft()->eval();
		if (V.getType() == mastertable[GetStringValue()].getType())
			mastertable[GetStringValue()] = V;
		else{
			log("_-",V);
			log("_-", mastertable[id]);;

			error(getLineNumber(), "type error");
		}
		return getLeft()->eval();
	}
};

// Operators
class Addition : public ParseTree {
public:
	Addition(int line, ParseTree *op1, ParseTree *op2) : ParseTree(line, op1, op2) {}
	TypeForNode GetType() const override { return ADDITION_TYPE; }
	virtual Value eval() override {log(this);return (getLeft()->eval() + getRight()->eval());}
};

class Subtraction : public ParseTree {
public:
	Subtraction(int line, ParseTree *op1, ParseTree *op2) : ParseTree(line, op1, op2) {}
	TypeForNode GetType() const { return SUBTRACTION_TYPE; }
	virtual Value eval() override {log(this); return getLeft()->eval() - getRight()->eval();}

};

class Division : public ParseTree {
public:
	Division(int line, ParseTree * op1, ParseTree *op2) : ParseTree(line, op1, op2){}
	TypeForNode GetType() const override { return DIVISION_TYPE; }
	virtual Value eval() override {log(this); return getLeft()-> eval() / getRight()->eval();}
};

class Multiplication : public ParseTree {
public:
	Multiplication(int line, ParseTree * op1, ParseTree *op2) : ParseTree(line, op1, op2){}
	TypeForNode GetType() const override { return MULTIPLICATION_TYPE; }
	virtual Value eval() override {log(this); return getLeft()-> eval() * getRight()->eval();}
};


//Constants
class IntegerConstant : public ParseTree {
	int	value;

public:
	IntegerConstant(const Token& tok) : ParseTree(tok.GetLinenum()) {
		value = stoi( tok.GetLexeme() );
	}

	virtual TypeForNode GetType() const override { return INT_TYPE; }
	int GetIntValue() const { return value; }
	virtual Value eval() override{log(this); return Value(getLineNumber(), GetIntValue());}

};
class StringConstant : public ParseTree {
	string	value;

public:
	StringConstant(const Token& tok) : ParseTree(tok.GetLinenum()) {
		value = tok.GetLexeme();
	}

	virtual TypeForNode GetType() const { return STRING_TYPE; }
	string GetStringValue() const { return value; }
	virtual Value eval() override {log(this); return Value(getLineNumber(), GetStringValue());}
};
extern bool loud;

extern ParseTree *	Prog(istream* in);
extern ParseTree *	StmtList(istream* in);
extern ParseTree *	Stmt(istream* in);
extern ParseTree *	Decl(istream* in);
extern ParseTree *	Set(istream* in);
extern ParseTree *	Print(istream* in);
extern ParseTree *	Expr(istream* in);
extern ParseTree *	Term(istream* in);
extern ParseTree *	Primary(istream* in);
extern ParseTree 		StartParse(istream* in);
extern int getCounters(char S,ParseTree* PT, int I);
extern ostream& operator<<(ostream& out, const ParseTree PT);
extern void printTokens(istream* in);
extern int evaluateTree(ParseTree* PT);


#endif /* PARSER_H_ */
