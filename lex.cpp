#include <iostream>
#include <cctype>
#include <regex>
#include "lex.h"
using namespace std;

enum State{
    //begin state of a Lex object
    IN_BEGIN, 
    //inside int/str/id/comment lexeme
    IN_INTEGER, IN_STRING, IN_ID, IN_COMMENT,
    //interim states
    ESCAPE, POSSIBLE_COMMENT,
    //Symbol or Keyword
    SYMBOL
};

bool isSymbol(char& c);
Token symbolToken(char& c);
bool isKeyword(string lexeme);
Token keywordToken(string lexeme);

Lex getNextToken (istream& in, int& linenumber) {
    string currentLexeme = "";
    State currentState = IN_BEGIN;
    char c;

    //loop over the input, character by character
    while ((c = in.get())){
        switch (currentState){
            case IN_BEGIN: {
                //return DONE token at eof
                if (in.eof()){
                    Lex returnLex(DONE, "", linenumber);
                    return returnLex;
                }
                //deal with new line
                else if (c == '\n'){
                    linenumber++;
                    continue;
                }
                //skip spaces
                else if (isspace(c)){
                    continue;
                }
                //integer beginning 
                else if (isdigit(c)){
                    currentLexeme = c;
                    currentState = IN_INTEGER;
                    continue;
                }
                //identifier beginning 
                else if (isalpha(c)){
                    currentLexeme = c;
                    currentState = IN_ID;
                    continue;
                }
                //comment beginning
                else if (c == '/'){
                    currentLexeme = c;
                    currentState = POSSIBLE_COMMENT;
                    continue;
                }
                //symbol
                else if (isSymbol(c)){
                    currentLexeme = c;
                    currentState = SYMBOL;
                    continue;
                }
                //string beginning
                else if (c == '"'){
                    currentState = IN_STRING;
                    continue;
                }

                //bad input
                currentLexeme = c;
                Lex returnLex(ERR, currentLexeme, linenumber);
                return returnLex;
            }
            case IN_INTEGER: {
                //continue int sequence
                if (isdigit(c)){
                    currentLexeme += c;
                    continue; 
                }

                //succesfully reached end of an int sequence
                in.unget();
                Lex returnLex(INT, currentLexeme, linenumber); 
                return returnLex;
            }
            case IN_STRING: {
                //escape sequence
                if (c == '\\'){
                    currentState = ESCAPE;
                    continue;
                }

                if (c == '"'){
                    //succesfully reached end of a string sequence
                    Lex returnLex(STR, currentLexeme, linenumber);
                    currentLexeme = "";
                    return returnLex;
                }
                else if (c == '\n'){
                    //error, newline
                    currentLexeme += c;
                    Lex returnLex(ERR, "\"" + currentLexeme, linenumber);
                    linenumber++;
                    return returnLex;
                }
                else if (in.eof()){
                    //bad value
                    Lex returnLex(ERR, "\"" + currentLexeme, linenumber);
                    return returnLex;
                }

                //continue string sequence
                currentLexeme += c;
                continue;
            }
            case ESCAPE: {
                if (c == 'n'){ //newline character
                    currentLexeme += '\n';
                    continue;
                }
                else if (c == '\\'){ //backslash character
                    currentLexeme += '\\';
                    continue;
                }
                
                //user escaped a regular character
                currentLexeme += c;
                currentState = IN_STRING;
                continue;
            }
            case POSSIBLE_COMMENT: {
                //start of a comment
                if (c == '/'){
                    currentState = IN_COMMENT;
                    continue;
                }
                else if (c == '\n'){ //newline, end of comment
                    Lex returnLex(SLASH, currentLexeme, linenumber);
                    linenumber++;
                    return returnLex;
                }

                //just a SLASH symbol
                in.unget();
                Lex returnLex(SLASH, currentLexeme, linenumber);
                return returnLex;
            }
            case IN_COMMENT:
                //end of comment on a line
                if (c == '\n'){
                    linenumber++;
                    currentLexeme = "";
                    currentState = IN_BEGIN;
                    continue;
                }

                //continue comment sequence
                continue;
            case IN_ID: {
                //continue identifier sequence
                if (isalnum(c)){
                    currentLexeme += c;
                    continue;
                }

                //end of identifier sequence
                in.unget();

                //check for possible keywords
                if (isKeyword(currentLexeme)){
                    Lex returnLex(keywordToken(currentLexeme), currentLexeme, linenumber);
                    return returnLex;
                }

                //return identifier
                Lex returnLex(ID, currentLexeme, linenumber);
                return returnLex;
            }
            case SYMBOL: {
                //disregard next character until next state
                in.unget();
                //return symbol Lex
                char sym = currentLexeme[0];
                Lex returnLex(symbolToken(sym), currentLexeme, linenumber);
                return returnLex;
            }
        }
    }

    //shouldn't get here
    Lex returnLex(ERR, c + "", -1);
    return returnLex;
}

bool isSymbol(char& c){
    switch (c){
        case '+':   //fallthrough
        case '-':   //fallthrough
        case '*':   //fallthrough
        case '/':   //fallthrough
        case '!':   //fallthrough
        case '(':   //fallthrough
        case ')':   //fallthrough
        case ';':   //fallthrough
            return true;
        default:
            return false;
    }
}

Token symbolToken(char& c){ 
    map<char, Token> charTokens;
    charTokens['+'] = PLUS;
    charTokens['-'] = MINUS;
    charTokens['*'] = STAR;
    charTokens['/'] = SLASH;
    charTokens['!'] = BANG;
    charTokens['('] = LPAREN;
    charTokens[')'] = RPAREN;
    charTokens[';'] = SC;

    for (auto itr = charTokens.begin(); itr != charTokens.end(); itr++){
        //return the appropriate token for the given keyword string
        if (c == itr->first){
            return itr->second;
        }
    }

    //shouldn't get here
    return ERR;
}

bool isKeyword(string lexeme){
    regex lcKeywordRegex("(print|let|if|loop|begin|end)");
    return regex_match(lexeme, lcKeywordRegex);
}

Token keywordToken(string lexeme){
    map<string, Token> keywordTokens;
    keywordTokens["print"] = PRINT;
    keywordTokens["let"] = LET;
    keywordTokens["if"] = IF;
    keywordTokens["loop"] = LOOP;
    keywordTokens["begin"] = BEGIN;
    keywordTokens["end"] = END;
    for (auto itr = keywordTokens.begin(); itr != keywordTokens.end(); itr++){
        //return the appropriate token for the given keyword string
        if (lexeme.compare(itr->first) == 0){
            return itr->second;
        }
    }

    //shouldn't get here
    return ERR;
}