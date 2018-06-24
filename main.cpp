#include <iostream>
#include <string>
#include <fstream>
#include <map>
using namespace std;

#include "parser.h"

using namespace std;
int lineNumber;
bool e_flag = false;
string filename;
bool loud = false; //Loud?

void error(int linenum, const string& message) {
	//All linenumbers seem to be just one off
	linenum++;
	cout << filename << ":" << linenum << ":" << message << endl;
	e_flag = true;
}

int main(int argc, char *argv[]) {

	//bool t = false; //Verbose Output
	bool d = false; //Dump Tokens
	istream* instream = &cin;
	ifstream* prestream = new ifstream();
	for (int i = 1; i < argc; i++) {
		string arg(argv[i]);
		if (arg == "-d") {
			d = true;
			continue;
		}
		else if (arg == "-l") {
			loud = true;
			continue;
		}
		else if (arg[0] == '-') {
			cout << arg << " UNRECOGNIZED FLAG" << endl;
			return 0;
		}
		else if (instream != &cin) {
			cout << "TOO MANY FILES" << endl;
			return -1;
		}
		else {
			filename = arg;
			prestream->open(arg);
			if (prestream->is_open())
				instream = prestream;
			else {
				cout << arg << " FILE NOT FOUND" << endl;
				return 0;
			}

		}
	}
	if (instream->good()) {
		//If d, dump tokens to get a better idea of what's going on
		if (loud)
			cout << "Like a lunatic!" << endl;
		if (d) {
			cout << "Dump mode specified | Dumping: " << filename << endl;
			printTokens(instream);
			cout << "Exiting..." << endl;
			return 0;
		}
		ParseTree P = StartParse(instream);
		getCounters('S', &P, 0);
		evaluateTree(&P);

		//If an error has been tripped, return without any output
		if (e_flag)
			return 0;
		/*
		 int I = getCounters('I', &P, int(0));

		 if (I == -1)
		 return -1;
		 int S = getCounters('S', &P, 0);
		 int Pl = getCounters('+', &P, 0);
		 int A = getCounters('*', &P, 0);

		 cout << "Total number of identifiers: " << I << endl;
		 cout << "Total number of set: " << S << endl;
		 cout << "Total number of +: " << Pl << endl;
		 cout << "Total number of *: " << A << endl;
		 */
	}

	/*	ParseTree *tree = Prog(&cin);
	 if (tree == 0) {
	 // there was some kind of parse error
	 return 1;
	 }
	 */
	return 0;
}

