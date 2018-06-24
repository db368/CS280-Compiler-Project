/*
 * parser.cpp
 *
 */

#include <map>
#include <string>
#include <iostream>
using std::string;
using std::endl;
using std::cout;
using std::map;
using std::stoi;
using std::pair;
#include "parser.h"
//  Identifier,   type     value
map<string, Value> mastertable;
static enum SemanticState {
	UNCHECKED, GOOD, BAD
} semanticstate = UNCHECKED;

class ParserToken {
private:
	Token tok;
	bool pushedBack;

public:
	ParserToken() :
			pushedBack(false) {
	}

	Token getToken(istream *in) {
		if (pushedBack) {
			pushedBack = false;
			return tok;
		}

		return ::getToken(in);
	}
	void pushbackToken(const Token& t) {
		if (pushedBack) {
			// error
		}
		tok = t;
		pushedBack = true;
	}
} ParserToken;

//Parse Tree
ostream& operator<<(ostream& out, const ParseTree PT) {
	if (PT.getLeft()) {
		out << "L" << *PT.getLeft() << "u";
	}
	if (PT.getRight()) {
		out << "R" << *PT.getRight() << "U";
	}
	out << "N";
	return out;
}

//Values

// \\-------------------End Operator Definition-------------------//
//Super secret developer function
void log(string step, Token type, int c) {
	if (!loud)
		return;

	switch (c) {
		case 0:
			cout << step << " " << type << " @ line " << type.GetLinenum() << endl;
			break;
		case 1:
			cout << step << endl;
			break;
	}
}
void log(string statement, ParseTree* PT) {
	log(PT, statement);
}
void log(ParseTree* PT, string statement) {
	if (!loud)
		return;

	cout << "> " << "line:" << PT->getLineNumber() << "|" << "type:" << PT->GetType() << "|" << statement << endl;
	return;
}
void log(string statement) {
	if (loud)
		cout << "> " << statement << endl;
}
void log(string statement, Value V) {
	if (!loud)
		return;
	cout << statement << "->";
	if (V.getType() == STRING_TYPE)
		cout << "STR | " << V.getStringValue();
	else if (V.getType() == INT_TYPE)
		cout << "INT |" << V.getIntValue();
	cout << endl;
	return;

}
void printTokens(istream* in) {
	Token t = T_INT;
	//cout << "Dumping Tokens..." << endl;
	while (t != T_DONE) {
		t = ParserToken.getToken(in);
		cout << t << endl;
	}
	//cout << "That's all of em" << endl;
	return;
}
bool checkSemantics(ParseTree* PT) { //Semantic Checks
	//If anything fails, set good to false but continue execution
	log("BEG", PT);
	bool good = true;
	switch (PT->GetType()) {
		//It's a Declare node
		//Check if it's already in the symbol table
		//If not, add it. If so, throw error
		case DECL_TYPE: {
			//check symb table

			if (mastertable.find(PT->GetStringValue()) != mastertable.end()) {
				string err = "variable " + PT->GetStringValue() + " was already declared";
				error(PT->getLineNumber(), err);
				good = false;
				break;
			}
			log("| !IN SYMB");
			//use getintvalue to determine type
			//1=string, 0=int
			int t = PT->GetIntValue();
			//It's a string
			if (t) {
				mastertable[PT->GetStringValue()] = Value(PT->getLineNumber(), "-1");
				log("| DEFINING " + PT->GetStringValue() + "  AS STRING");
			}
			//It's an int
			else {
				mastertable[PT->GetStringValue()] = Value(PT->getLineNumber(), -1);
				log("| DEFINING " + PT->GetStringValue() + " AS INT");
			}
			break;
		}
			//It's an ID, check if it has been declared
		case ID_TYPE:
			if (mastertable.find(PT->GetStringValue()) == mastertable.end()) {
				good = false;
				string err = "variable " + PT->GetStringValue() + " is used before being declared";
				error(PT->getLineNumber(), err);
			}
			break;
		case SET_TYPE: {
			if (mastertable.find(PT->GetStringValue()) == mastertable.end()) {
				good = false;
				string err = "variable " + PT->GetStringValue() + " is used before being declared";
				error(PT->getLineNumber(), err);
			}
			else {
				//Value V = PT->eval();
				//if (V.getType() != mastertable[PT->GetStringValue()].getType())
				//	error(PT->getLineNumber(), "Set type did not match actual type"); //TODO: Set the correct error message for this

			}
			break;
		}
		default:
			log("| NO BEHAVIOR");
			break;
	}
	log("| END");
	if (PT->getLeft())
		if (!checkSemantics(PT->getLeft()))
			good = false;
	if (PT->getRight())
		if (!checkSemantics(PT->getRight()))
			good = false;

	return good;
}

int evaluateTree(ParseTree* PT) {
	PT->eval();
	return 0;
}
int getCounters(char S, ParseTree* PT, int ic) {
	switch (semanticstate) {
		case UNCHECKED:
			if (checkSemantics(PT))
				semanticstate = GOOD;
			else {
				semanticstate = BAD;
				return -1;
			}
			break;
		case GOOD:
			break;
		case BAD:
			return -1;
	}
	switch (S) {
		case 'I':
			if (PT->GetType() == DECL_TYPE)
				ic++;
			break;
		case ('+'):
			if (PT->GetType() == ADDITION_TYPE)
				ic++;
			break;
		case ('*'):
			if (PT->GetType() == MULTIPLICATION_TYPE)
				ic++;
			break;
		case ('S'):
			if (PT->GetType() == SET_TYPE)
				ic++;
			break;
		default:
			break;
	}
	if (PT->getLeft()) {
		ic = getCounters(S, PT->getLeft(), ic);
	}
	if (PT->getRight()) {
		ic = getCounters(S, PT->getRight(), ic);
	}
	return ic;
}

ParseTree StartParse(istream* in) {
	ParseTree* PT = Prog(in);
	return *PT;
}
void error(const string& msg) {
	return;
}

ParseTree *Prog(istream* in) {
	return StmtList(in);
}
ParseTree * StmtList(istream* in) {
	ParseTree *stmt = Stmt(in);
	if (stmt == 0) {
		return 0;
	}
	Token TT = ParserToken.getToken(in);
	if (TT != T_SC)
		error(TT.GetLinenum(), "Syntax error semicolon required");
	log("LIST", TT);
	return new StatementList(stmt, StmtList(in));
}

ParseTree * Stmt(istream* in) {
	//Take a token to see where we're going
	//Push it back so the next function can use it
	Token TT = ParserToken.getToken(in);
	log("STMT", TT);
	if (TT == T_INT || TT == T_STRING) {
		//Decl doesn't really give a node(?) so call statement again
		ParserToken.pushbackToken(TT);
		return Decl(in);
	}
	//These methods will return a node
	else if (TT == T_PRINT || TT == T_PRINTLN) {
		ParserToken.pushbackToken(TT);
		return Print(in);
	}

	else if (TT == T_SET)
		return Set(in);

	//statement is invalid or does not exist
	return 0;
}

ParseTree * Decl(istream* in) {
	Token TT = ParserToken.getToken(in);
	//Determine whether it's an int or a string
	//Then check for an ID after it.
	log("DECL", TT);
	if (TT == T_INT) {
		TT = ParserToken.getToken(in);
		if (TT == T_ID) {
			return new DeclNode(TT.GetLinenum(), TT.GetLexeme(), 0);
		}
		else
			error(TT.GetLinenum(), "Expected value after Int");
	}
	else if (TT == T_STRING) {
		TT = ParserToken.getToken(in);
		if (TT == T_ID) {
			return new DeclNode(TT.GetLinenum(), TT.GetLexeme(), 1);
		}
		else
			error(TT.GetLinenum(), "Expected value after STR");
	}
	//This node has no definition
	//TT = ParserToken.getToken(in);
	//if (TT != T_SC)
	//error(TT.GetLinenum(), "Semicolon expected after Declaration.");
	return 0;
}

ParseTree * Set(istream* in) {
	Token TT = ParserToken.getToken(in);
	ParseTree* ret;
	log("SET ", TT);
	if (TT != T_ID)
		error(TT.GetLinenum(), "No Identifier Specified");
	ret = new SetNode(TT.GetLinenum(), TT.GetLexeme(), Expr(in));
	if (!ret->getLeft())
		error(TT.GetLinenum(), "Syntax error expression required");
	//if (ParserToken.getToken(in) != T_SC)
	//	error(TT.GetLinenum(), "Expected a semicolon after Set Statement");
	return ret;
}

ParseTree * Print(istream* in) {
	Token T = ParserToken.getToken(in);
	log("PRN", T);
	if (T == T_PRINT || T == T_PRINTLN) {
		ParseTree* print;
		if (T == T_PRINT)
			print = new PrintNode(T.GetLinenum(), Expr(in));
		else
			print = new PrintLNode(T.GetLinenum(), Expr(in));
		if (!print->getLeft())
			error(T.GetLinenum(), "Syntax error expression required");
		return print;
	}
	else
		ParserToken.pushbackToken(T);
	return 0;
}

ParseTree *Expr(istream* in) {

	ParseTree *t1 = Term(in);
	if (t1 == 0)
		return 0;

	for (;;) {
		Token op = ParserToken.getToken(in);

		log("EXPR", op);
		if (op != T_PLUS && op != T_MINUS) {
			ParserToken.pushbackToken(op);
			return t1;
		}

		ParseTree *t2 = Expr(in);
		if (t2 == 0) {
			error(op.GetLinenum(), "Syntax error expression required after + or - operator");
			return 0;
		}

		// combine t1 and t2 together
		if (op == T_PLUS)
			t1 = new Addition(op.GetLinenum(), t1, t2);
		else
			t1 = new Subtraction(op.GetLinenum(), t1, t2);
	}

	// should never get here...
	return 0;
}

ParseTree * Term(istream* in) {
	ParseTree *L = Primary(in);
	if (L == 0)
		return 0;
	for (;;) {
		Token TT = ParserToken.getToken(in);

		log("TERM", TT);
		if (TT == T_STAR || TT == T_SLASH) { //DONE  Define Multiplication/Division
			ParseTree *R = Primary(in);
			if (R == 0) {
				error(TT.GetLinenum(), "expression required after * or / operator");
				return 0;
			}

			if (TT == T_STAR)
				L = new Multiplication(TT.GetLinenum(), L, R);
			else
				L = new Division(TT.GetLinenum(), L, R);
		}
		else {
			ParserToken.pushbackToken(TT);
			return L;
		}
	}
	return 0;
}

ParseTree * Primary(istream* in) {
	Token TT = ParserToken.getToken(in);
	log("PRIM", TT);
	switch (TT.GetTokenType()) {
		case (T_ICONST):
			return (new IntegerConstant(TT));
			break;
		case (T_SCONST):
			return (new StringConstant(TT));
			break;
		case (T_ID): {
			//Semantic Checking is not meant to be done here
			return new IDNode(TT.GetLinenum(), TT.GetLexeme());
			break;
		}
		case (T_LPAREN): {
			ParseTree *L = Expr(in);

			if (L == 0) {
				error(TT.GetLinenum(), "No expression after Lparen");
				return 0;
			}
			TT = ParserToken.getToken(in);
			if (TT != T_RPAREN) {
				log("PUSHING BACK", TT);
				ParserToken.pushbackToken(TT);
				error(TT.GetLinenum(), "Syntax error right paren expected");
				return 0;
			}
			return L;
			break;
		}
			//	case (T_ERROR): {
		default: {
			error(TT.GetLinenum(), "Syntax error primary expected");
			//	break;
			//	default: {
			//		ParserToken.pushbackToken(TT);
			return 0;
			break;
		}
	}
	return 0;
}

