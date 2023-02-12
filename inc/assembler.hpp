#ifndef _ASEMBLER_H__
#define _ASEMBLER_H__

#include <iostream>
#include "structs.hpp"
using namespace std;

/* Directives */

void assemblyLabel(string* label);
void assemblyGlobalDirective(LineArguments* arguments);
void assemblyExternDirective(LineArguments* arguments);
void assemblySectionDirective(string* sectionName);
void assemblyWordDirective(LineArguments* arguments);
void assemblySkipDirective(string* litteralToSkip);
void assemblyEndDirective();


/* Instructions */


/* FLOW CONTROL */
void assemblyHALT();
void assemblyINT(string* registerNum);
void assemblyIRET();
void assemblyCALL(LineArguments* arguments);
void assemblyRET();

/* JUMPS */
void assemblyJMP(LineArguments* arguments);
void assemblyJEQ(LineArguments* arguments);
void assemblyJNE(LineArguments* arguments);
void assemblyJGT(LineArguments* arguments);

/* VALUES MANIPULATION*/
void assemblyPUSH(string* registerNum);
void assemblyPOP(string* registerNum);
void assemblyXCHG(string* registerDstNum, string* registerSrcNum);

/* ARITMETIC */
void assemblyADD(string* registerDstNum, string* registerSrcNum);
void assemblySUB(string* registerDstNum, string* registerSrcNum);
void assemblyMUL(string* registerDstNum, string* registerSrcNum);
void assemblyDIV(string* registerDstNum, string* registerSrcNum);


/* LOGICAL */
void assemblyCMP(string* registerDstNum, string* registerSrcNum);
void assemblyNOT(string* registerNum);
void assemblyAND(string* registerDstNum, string* registerSrcNum);
void assemblyOR(string* registerDstNum, string* registerSrcNum);
void assemblyXOR(string* registerDstNum, string* registerSrcNum);
void assemblyTEST(string* registerDstNum, string* registerSrcNum);

/* SHIFTING */
void assemblySHL(string* registerDstNum, string* registerSrcNum);
void assemblySHR(string* registerDstNum, string* registerSrcNum);

void assemblyLDR(string* registerToLoad, LineArguments* srcOperand);
void assemblySTR(string* registerToLoad, LineArguments* srcOperand);


void printSymbolTable();
void printAllSections();

#endif 
