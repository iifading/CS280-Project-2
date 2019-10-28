#include "lex.h"
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <regex>
#include <fstream>
using namespace std;

void printToken(Lex currentLex);
extern Lex getNextToken(istream& in, int& linenumber);
static string tokenArray[19] = {"PRINT", "LET", "IF", "LOOP", "BEGIN", "END",
                            "ID", "INT", "STR", "PLUS", "MINUS", "STAR", 
                            "SLASH", "BANG", "LPAREN", "RPAREN", "SC", 
                            "ERR", "DONE"};

int main(int argc, char* argv[]){
    //Determine what command line arguments were given by the user
    //and store presence as a bool
    bool vPassed = false;
    bool constsPassed = false;
    bool idsPassed = false;
    bool filenamePassed = false;
    string filename;

    //loop over cmd line args
    for(int i = 1; i < argc; i++) {
        string arg = argv[i];
        regex filenameRegex("[a-z_A-Z0-9.]+");
        if (arg[0] == '-'){
            //check for specific command-line arguments
            if (arg.compare("-v") == 0) { vPassed = true; }
            else if (arg.compare("-consts") == 0) { constsPassed = true; }
            else if (arg.compare("-ids") == 0) { idsPassed = true; }
            else { 
                cout << "UNRECOGNIZED FLAG " << arg << endl; 
                return 0;
            }
        }

        if (regex_match(arg, filenameRegex)) {
            if (filenamePassed) { 
                cout << "ONLY ONE FILE NAME ALLOWED" << endl;
                return 0;
            }

            filenamePassed = true;
            filename = arg;
        }
    }//for
    
    
                        /***** Begin Lexical Analysis *****/
    vector<Lex> lexList;
    int lineNum = 0;

    //validate file is openable, then perform analysis on file
    if (filenamePassed) {
        ifstream inFileStream;
        inFileStream.open(filename);
        if (inFileStream.fail()){
            cout << "CANNOT OPEN " << filename << endl;
            return 0;
        }

        //grab tokens from file
        Lex currentLex = getNextToken(inFileStream, lineNum);
        while (currentLex.GetToken() != ERR && currentLex.GetToken() != DONE){
            //add each Lex object to the list as found and print out the
            //token with the lexeme if argument -v specified so
            lexList.push_back(currentLex);
            if (vPassed){
               printToken(currentLex);
            }

            currentLex = getNextToken(inFileStream, lineNum);
        }

        //Error handling
        if (currentLex.GetToken() == ERR){
            if (lineNum == 0) { lineNum++; }
            printToken(currentLex);
            return 0;
        }
    }
    else { 
        //read from std in
        Lex currentLex = getNextToken(std::cin, lineNum);
        while (currentLex.GetToken() != ERR && currentLex.GetToken() != DONE){
            //add each Lex object to the list and print out the
            //token with the lexeme if argument specified so
            lexList.push_back(currentLex);
            if (vPassed) {
               printToken(currentLex);
            }

            currentLex = getNextToken(cin, lineNum);
        }

        //Error handling
        if (currentLex.GetToken() == ERR){
            if (lineNum == 0) { lineNum++; }
            printToken(currentLex);
            return 0;
        }
    }//Lex analysis

    if (constsPassed){
        /*Print all unique STRs in abc order      (#1)
        One string per line.
        Then, print all integer constants,
        in numerical order, one string per line
        Print nothing if STRs or INTs are 
        absent
        */
    
        //populate two lists of strings and integer constants from
        //the list of Lex objects we gathered earlier during analysis

        map<string, int> strMap;
        map<int, int> intMap;
        for (auto itr = lexList.begin(); itr != lexList.end(); itr++){
            if (itr->GetToken() == STR){
                strMap[itr->GetLexeme()] = 1;

            }
            else if (itr->GetToken() == INT){
                intMap[stoi(itr->GetLexeme())] = 1;
            }
        }

        for (auto itr = strMap.begin(); itr != strMap.end(); itr++){
            if (itr == strMap.begin()){
                cout << "STRINGS:" << endl;
            }
            cout << itr->first << endl;
        }

        for (auto itr = intMap.begin(); itr != intMap.end(); itr++){
            if (itr == intMap.begin()){
                cout << "INTEGERS:" << endl;
            }
            cout << itr->first << endl;
        }
    }//-consts

    if (idsPassed){
        /*Print all identifiers in abc order      (#2)
        as a comma-separated list
        Print nothing if IDs are absent
        */

       //populate a sorted map of identifiers from lexList
        map<string, int> idMap;
        for (auto itr = lexList.begin(); itr != lexList.end(); itr++){
            if (itr->GetToken() == ID){
                idMap[itr->GetLexeme()] = 1;
            }
        }

        int count = 0;
        //print a comma-separated list of sorted identifiers
        for (auto itr = idMap.begin(); itr != idMap.end(); itr++){
            if (itr == idMap.begin()){
                cout << "IDENTIFIERS: ";
            }
            //last element formatting
            if (count == idMap.size() - 1){
                cout << itr->first << endl;
                break;
            }
            cout << itr->first << ", ";
            count++;
        }
    }//-ids
    
    //print out summary
    cout << "Lines: " << lineNum << endl;
    if (lineNum > 0) {
        cout << "Tokens: " << lexList.size() << endl;
    }
    
    return 0;
}

void printToken(Lex currentLex){
    //if the current token is ID, INT, STR, or ERR,
    //print out the lexeme with the appropriate formatting.
    //otherwise, print just the token
    switch(currentLex.GetToken()){
        case ID:
        case INT:
        case STR:
            cout << tokenArray[currentLex.GetToken()] << "(" << currentLex.GetLexeme() << ")" << endl;
            break;
        case ERR:
            cout << "Error on line " << currentLex.GetLinenum() + 1 << " (" << currentLex.GetLexeme() << ")" << endl;
            break;
        default:
            cout << tokenArray[currentLex.GetToken()] << endl;
            break;
    }
}
