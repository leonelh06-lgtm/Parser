/*
Homework:
parser - HW3 PL/0 parser and symbol table

Author(s): <Leonel Hernandez>

Language: C only

To Compile:
  gcc -Wall -Wextra -std=c11 -O2 parser.c -o parser

To Execute (on Eustis):
  ./lex <input_file> && ./parser

where:
  <input_file> is the path to a text file holding a PL/0 source program;
  ./lex writes tokens.txt and nametable.txt, and ./parser reads them from
  the current directory and takes no arguments

Notes:
  - Implements the recursive-descent parser and symbol table described in
    the homework instructions.
  - Prints four sections to standard output: Name Table, Parse Trace,
    Symbol Table and Result.
  - Stops at the first syntax or symbol-table error, reports it with its
    token number, and exits with status 1.
  - Exits with status 0 when the whole program is accepted.
  - Tested on Eustis.

Class: COP 3402 - Systems Software

Instructor: Jie Lin, Ph.D.

Due Date: See Webcourses
*/

#include <stdio.h>
#include <stdlib.h>
//Defines tokens
    typedef struct{
        int tCode;
        int val;
        char text[64];
    } Token;
//Defines identifiers
    typedef struct {
        int index;
        char name[13];
        int line;
        int col;
    } Identifier;
//Defines symbols
    typedef struct{
        int type;
        int nameIdx;
        int value;
        int level;
        int address;
        int mark;
    }  Symbol;


//Counting global variables
int tCount = 0;
int iCount = 0;
int symbolCount = 0;

int currentToken = 0;
int nextAddress = 3;

//Create Token/Identifier/Symbol tables
Token tokenTable[5000];
Identifier identifierTable[200];
Symbol symbolTable[100];

//Function defs

//Adds Symbol to symbol table
void addSymbol(int type, int nameIdx, int value, int address);
//Declares action for a constant variable
void constDecleration();
//Declares action for a variable
void varDecleration();
//Non-terminal for code block in program
void block();
//Non-terminal for whole program
void program();
//Helper function to find symbolTable index
int findSymbol(int nameIdx);
//Non-terminal for statement in block
void statement();
//Collection of statements
void statementList();
//Non-terminal for factor in term
void factor();
//Non terminal for expression in statement
void expression();
//Non-terminal for terms in expressions
void term();
//Non terminal for conditional expressions
void condition();
//Handles error and graceful exit logic
void error(int errCode);
//Gets identifiers string name for error output
const char* getTokenText(int tokenIdx);
//Creates output when a token is consumed
void consumeToken(const char* owner);
//Helper for parse output 
void traceLookup(int symbolIdx);
//Prints nameTable
void printNameTable();
//Prints symbolTable
void printSymbolTable();

int main(){
    FILE* tokens = fopen("tokens.txt", "r");
    FILE* nametable = fopen("nametable.txt", "r");

    if(tokens == NULL || nametable == NULL){
    printf("Name Table:\n\n");
    printf("Parse Trace:\n\n");
        error(18);
    }
    
    //Scanning tokens into local tokentable
    Token t;
    while(fscanf(tokens, "%d", &t.tCode) == 1){
        if(t.tCode == 1){
            fscanf(tokens, "%d", &t.val);
        }
        else if(t.tCode == 2){
            fscanf(tokens, "%63s", t.text);
            t.val = atoi(t.text);
        }
        else{
            t.val = 0;
        }
        tokenTable[tCount] = t;
        tCount++;
    }
    fclose(tokens);

    //Scanning names into local nametable
    Identifier i;
    while(fscanf(nametable, "%d %s %d %d", &i.index, i.name, &i.line, &i.col) == 4){
        identifierTable[iCount] = i;
        iCount++;
    }
    fclose(nametable);

    //Prints nameTable
    printNameTable();

    //Prints parse trace title
    printf("Parse Trace:\n\n");
    printf("token\tinput\ttext\tnonterminal\n");

    //Scanning tokenTable to get symbolTable
    program();

    //Prints symbolTable
    printSymbolTable();

    printf("Result:\n\n");
    printf("program accepted\n");

    return 0;
}

/*Adds symbols to symbol table (needs a function unlike identifier and tokens
since it requires context of tokens to decide symbol paramters)*/
void addSymbol(int type, int nameIdx, int value, int address){
    Symbol s;
    s.type = type;
    s.value = value;
    s.address = address;
    s.level = 0;
    s.mark = 0;
    s.nameIdx = nameIdx;
    symbolTable[symbolCount] = s;

    //Parse tracing for constant
    if(type == 1){
        printf("insert entry %d: constant %s, value %d, level 0\n", symbolCount, identifierTable[nameIdx].name, value);
    }
    //Parse tracing for variables
    else{
        printf("insert entry %d: variable %s, level 0, address %d\n", symbolCount, identifierTable[nameIdx].name, address);
    }
    symbolCount++;
}

void constDecleration(){
    //Moves past const keyword
    consumeToken("const-decl");

    while(1){
        //Checks if not identifier
        if(currentToken >= tCount || tokenTable[currentToken].tCode != 1){
            //Error 2: token is not identifier
            error(2);
        }

        //Gets nameTable index and moves past identifer
        int nameIdx = tokenTable[currentToken].val;

        consumeToken("const-decl");

        if(findSymbol(nameIdx) >= 0){
            //Error 14: Identifier already declared in block
            currentToken--;
            error(14);
        }


        //Checks if ":=" is missing
        if(currentToken >= tCount || tokenTable[currentToken].tCode != 19){
            //Error 3: Expected :=
            error(3);
        }
        //Consumes ":="
        consumeToken("const-decl");

        //Checks if the assigned value is not a constant
        if(currentToken >= tCount || tokenTable[currentToken].tCode != 2){
            //Error 3: Expected a number
            error(3);
        }

        //gets the value assigned to the identifier then consumes value
        int value = tokenTable[currentToken].val;
        
        //Consumes number
        consumeToken("const-decl");

        //We now have the tokenTable context to create the symbol
        addSymbol(1, nameIdx, value, 0);
        
        //Looks for comma
        if(currentToken < tCount && tokenTable[currentToken].tCode == 15){
            //Consumes comma keeps looping
            consumeToken("const-decl");
        }
        else{
            break;
        }
    }

    //Checks if semi-colon is missing
    if(currentToken >= tCount || tokenTable[currentToken].tCode != 16){
        //Error 4: Expected semi-colon
        error(4);
    }
    //Consumes semi-colon
    consumeToken("const-decl");
}

void varDecleration(){
    //Moves past var keyword
    consumeToken("var-decl");

    while(1){

        //Check for identifier
        if(currentToken >= tCount || tokenTable[currentToken].tCode != 1){
            //Error 2: Expected identifier
            error(2);
        }

        int nameIdx = tokenTable[currentToken].val;

        //Consume identifier
        consumeToken("var-decl");

        if(findSymbol(nameIdx) >= 0){
            //Error 14: Identifier already declared in block
            currentToken--;
            error(14);
        }

        

        // Check whether the symbol table is full
        if(symbolCount >= 100){
            // Point back to the identifier for the error message
            currentToken--;
            error(17);
        }

        //Adds var to symbol table
        addSymbol(2, nameIdx, 0, nextAddress);
        //Increments adress
        nextAddress++;

        //Check for more variables
        if(currentToken < tCount && tokenTable[currentToken].tCode == 15){
            //Consume comma
            consumeToken("var-decl");
        }

        else{
            break;
        }
    }

    //Checks for lack of semicolon
    if(currentToken >= tCount || tokenTable[currentToken].tCode != 16){
        //Error 4: Expected semi-colon
        error(4);
    }
    //Consume semi-colon
    consumeToken("var-decl");


} 

void block(){
    printf("enter level 0\n");

    //Checks for const keyword
    if(currentToken < tCount && tokenTable[currentToken].tCode == 31){
        constDecleration();
    }
    //Checks for var keyword
    if(currentToken < tCount && tokenTable[currentToken].tCode == 32){
        varDecleration();
    }

    statement();

    //Set mark of all used symbols to 1
    for(int i = 0; i < symbolCount; i++){
        if(symbolTable[i].mark == 0){
            symbolTable[i].mark = 1;
            printf("mark entry %d\n", i);
        }
    }

    printf("leave level 0\n");
}

void program(){
    block();
    //Checks if token is period
    if(currentToken < tCount && tokenTable[currentToken].tCode == 17){
        //Consumers period
        consumeToken("program");
    }

    else{
        //Error 1: doesn't end with period
        error(1);
    }

    if(currentToken != tCount){
        //Error 1: program ends but tokens still remain after period
        error(1);
    }
}

int findSymbol(int nameIdx){
    //Go through symbols and check if nameIdx is taken already or if variable already used
    //If not returns symbolTable index
    for(int i = symbolCount - 1; i >= 0; i--){
        if(symbolTable[i].nameIdx == nameIdx && symbolTable[i].mark == 0){
            return i;
        }
    }
    
    return -1;
}

void statement(){
    if(currentToken >= tCount){
        return;
    }

    //Checks for assignment statements
    if(currentToken < tCount && tokenTable[currentToken].tCode == 1){
        int nameIdx = tokenTable[currentToken].val;
        int symbolIdx = findSymbol(nameIdx);

        //Consumes identifier
        consumeToken("statement");

        if(symbolIdx < 0){
            //Error 15: undeclared identifier
            currentToken--;
            error(15);
        }

        //Generates output for assignmnet statements lookup
        traceLookup(symbolIdx);

        if(symbolTable[symbolIdx].type != 2){
            //Error 16: can't assign to number
            currentToken--;
            error(16);
        }

        //Checks for "="
        if(currentToken >= tCount || tokenTable[currentToken].tCode != 18){
            //Error 5: expected =
            error(5);
        }

        //Consumes =
        consumeToken("statement");

        expression();
    }

    //Looks for begin statements
    else if(currentToken < tCount && tokenTable[currentToken].tCode == 20){
        //Consumes begin
        consumeToken("statement");
        statementList();
        //Looks for end
        if(currentToken >= tCount || tokenTable[currentToken].tCode != 21){
            //Error 6: missing end statement
            error(6);
        }
        //Consumes end
        consumeToken("statement");
    }

    //Looks for If then statements
    else if(currentToken < tCount && tokenTable[currentToken].tCode == 22){
        //Consumes if and checks for condition
        consumeToken("statement");
        condition();

        if(currentToken >= tCount || tokenTable[currentToken].tCode != 24){
            //Error 7: no then statment
            error(7);
        }
        //Consumes then
        consumeToken("statement");

        statementList();

        if(currentToken < tCount && tokenTable[currentToken].tCode == 36){
            //Consumes else
            consumeToken("statement");
            statementList();
        }
        
        //Checks for "fi"
        if(currentToken >= tCount || tokenTable[currentToken].tCode != 23){
            //Error 8: no fi to show statement end
            error(8);
        }
        //Consume fi
        consumeToken("statement");
    }

    //Looks for while/do statements
    else if(currentToken < tCount && tokenTable[currentToken].tCode == 25){
        //Consumes while
        consumeToken("statement");
        condition();

        if(currentToken >= tCount || tokenTable[currentToken].tCode != 24){
            //Error 7: no then statmement
            error(7);
        }
        //Consume then
        consumeToken("statement");
        statementList();
        //Checks for elihw finishing of while
        if(currentToken >= tCount || tokenTable[currentToken].tCode != 26){
            //Error 8: no elihw statmement
            error(8);
        }
        //Consumes elihw
        consumeToken("statement");
    }

    //Looks for read
    else if(tokenTable[currentToken].tCode == 35){
        //Consume read
        consumeToken("statement");

        //Checks if its not identifier
        if(currentToken >= tCount || tokenTable[currentToken].tCode != 1){
            //Error 2: Expected identifier
            error(2);
        }

        int nameIdx = tokenTable[currentToken].val;
        int symbolIdx = findSymbol(nameIdx);

        //Consumes identifer
        consumeToken("statement");

        if(symbolIdx < 0){
            //Error 15 Undeclared identifer
            currentToken--;
            error(15);
        }

        //Generates lookup trace output
        traceLookup(symbolIdx);

        //Makes sure identifier is a variable
        if (symbolTable[symbolIdx].type != 2){
            //Error 16: Can only read variables
            currentToken--;
            error(16);
        }
    }

    //Looks for write
    else if(tokenTable[currentToken].tCode == 34){
    // Consume write
    consumeToken("statement");

    //Parses expression that is written
    expression();
    }

    //Checks for unsupported call and procdure statements
    else if(tokenTable[currentToken].tCode == 30){
    error(13);
    }
    else if(tokenTable[currentToken].tCode == 33){
        error(12);
    }
}

void statementList(){
    statement();

    while(currentToken < tCount && tokenTable[currentToken].tCode == 16){
        //Consumes ";"
        consumeToken("statement-list");
        //Parses each statement in statement list
        statement();
    }
}

void factor(){
    if(currentToken >= tCount){
        //Error 11: Expected Factor
        error(11);
    }

    int tokenType = tokenTable[currentToken].tCode;

    //Checks for identifier
    if(tokenType == 1){
        int nameIdx = tokenTable[currentToken].val;
        int symbolIdx = findSymbol(nameIdx);

        //Consumes identifier
        consumeToken("factor");

        if(symbolIdx < 0){
            //Error 15: Undeclared identifier
            currentToken--;
            error(15);
        }


        //Traces lookup for symbol
        traceLookup(symbolIdx);

    }

    //Checks for numbers
    else if(tokenType == 2){
        //Consumes number
        consumeToken("factor");
    }

    //Handles "("
    else if(tokenType == 13){
        consumeToken("factor");

        //Parse expression in parenthesis
        expression();

        //Checks for valid closing parenthesis ")"
        if(currentToken >= tCount || tokenTable[currentToken].tCode != 14){
            //Error 10: Invalid closing parenthesis
            error(10);
        }
        //Consumes )
        consumeToken("factor");

    }

    else{
        //Error 11: Invalid factor
        error(11);
    }
}

void expression(){

    //Checks for optional beggining sign symbol
    if(currentToken < tCount && (tokenTable[currentToken].tCode == 3 || tokenTable[currentToken].tCode == 4)){
        //Consumes sign symbol
        consumeToken("expression");
    }
    //Parses first term
    term();

    //Checks for "+" or "-"
    while(currentToken < tCount  && (tokenTable[currentToken].tCode == 3 || tokenTable[currentToken].tCode == 4)){
        //Consumes "+" or "-"
        consumeToken("expression");
        //Parses next term
        term();
    }

    //Since expression calls turn the recursive call for term will finish first given multiplication and division precedence
}

void term(){
    
    //Consumes the factor in term
    factor();
    //Checks for "*" or "/"
    while(currentToken < tCount && (tokenTable[currentToken].tCode == 5 || tokenTable[currentToken].tCode == 6)){
        //Consumes "*" or "/"
        consumeToken("term");
        //Parses factor
        factor();
    }
}

void condition(){
    //Parses expression
    expression();
    //Checks for comparison opperators
    if(currentToken < tCount && (tokenTable[currentToken].tCode == 7 || tokenTable[currentToken].tCode == 8 || tokenTable[currentToken].tCode == 9 || tokenTable[currentToken].tCode == 10 || tokenTable[currentToken].tCode == 11 || tokenTable[currentToken].tCode == 12)){
        consumeToken("condition");
        //Parses second expression
        expression();
    }
    else{
        //Error 9: Invalid comparison opperator
        error(9);
    }
}

void error(int errCode){
    //static string array to hold err msgs
static const char *messages[] = {
        "",
        "the program must end with a period, and nothing may follow it",
        "const, var, and read must be followed by an identifier",
        "a constant must be declared as name := number",
        "declarations must be separated by , and end with ;",
        "an identifier that starts a statement must be followed by =",
        "begin must be closed with end",
        "a condition after if or while must be followed by then",
        "if must be closed with fi, and while with elihw",
        "a condition must contain one of == != < <= > >=",
        "a left parenthesis must be closed with a right parenthesis",
        "a factor must be an identifier, a number, or (",
        "procedure declarations are not part of this grammar",
        "call statements are not part of this grammar",
        "identifier already declared in this block",
        "undeclared identifier",
        "only a variable may be assigned or read",
        "symbol table is full",
        "cannot open tokens.txt or nametable.txt"
    };
    
    //Prints symbolTable and result line
    if(errCode == 18){
    printf("\nSymbol Table:\n\n");
    }
    else{
        printSymbolTable();
    }

    printf("Result:\n\n");


    //Handles file opening issues
    if(errCode == 18){
        printf("Error 18: %s\n", messages[18]);
    }
    //Handles out of bounds tokens
    else if(currentToken >= tCount){
        printf("Error %d at end of tokens: %s\n", errCode, messages[errCode]);
    }
    else{
        printf("Error %d at token %d '%s': %s\n", errCode, currentToken + 1, getTokenText(currentToken), messages[errCode]);
    }

    exit(1);
}

const char* getTokenText(int tokenIdx){
    static char buffer[32];

    if(tokenIdx < 0 || tokenIdx >= tCount){
        return "";
    }

    Token t = tokenTable[tokenIdx];

    //Checks if identifier
    if(t.tCode == 1){
        int nameIdx = t.val;

        //Gets the string name of identifier
        for(int i = 0; i<iCount; i++){
            if(identifierTable[i].index == nameIdx){
                return identifierTable[i].name;
            }
        }
        return "";
    }

    //Checks if number (must be converted to string)
    if(t.tCode == 2){
        return tokenTable[tokenIdx].text;
    }

    // Operators, punctuation, and keywords
    static const char *tokenNames[] = {
        "", "", "",
        "+", "-", "*", "/",
        "==", "!=", "<", "<=", ">", ">=",
        "(", ")", ",", ";", ".", "=", ":=",
        "begin", "end", "if", "fi", "then",
        "while", "elihw", "do", "od", "odd",
        "call", "const", "var", "procedure",
        "write", "read", "else"
    };

    if(t.tCode >= 3 && t.tCode <= 36){
        return tokenNames[t.tCode];
    }

    return "";
}

void consumeToken(const char* owner){
    if(currentToken >= tCount)
        return;
    int code = tokenTable[currentToken].tCode;
    //If current token is identifier or number which have val field
    if(code == 1){
        //For Identifiers
        printf("%d\t%d %d\t%s\t%s\n", currentToken + 1, code, tokenTable[currentToken].val, getTokenText(currentToken), owner);
    }

    else if(code == 2){
        //For Numbers
        printf("%d\t%d %s\t%s\t%s\n", currentToken + 1, code, getTokenText(currentToken), getTokenText(currentToken), owner);
    }

    else{
        //Other tokens don't trace val
        printf("%d\t%d\t%s\t%s\n", currentToken + 1, code, getTokenText(currentToken), owner);
    }
    currentToken++;
}

void traceLookup(int symbolIdx){
    Symbol s = symbolTable[symbolIdx];
    const char* typeName;
    if(s.type == 1)
        typeName = "constant";
    else
        typeName = "variable";

    printf("lookup %s: entry %d, %s, level %d\n", identifierTable[s.nameIdx].name, symbolIdx, typeName, s.level);
}

void printNameTable(){
    printf("Name Table: \n\n");
    if(iCount > 0){
        printf("index\tname\tline\tcolumn\n");

        for(int i = 0; i<iCount; i++){
            Identifier id = identifierTable[i];
            printf("%d\t%s\t%d\t%d\n", id.index, id.name, id.line, id.col);
        }
    }     
    
    printf("\n");
}

void printSymbolTable(){
    printf("\nSymbol Table: \n\n");
    if(symbolCount > 0){
        printf("entry\tkind\tname\tvalue\tlevel\taddress\tmark\n");
        for(int i = 0; i<symbolCount; i++){
            Symbol s = symbolTable[i];
            if(s.type == 1){
                //Constants
                printf("%d\tconstant\t%s\t%d\t%d\t-\t%d\n", i, identifierTable[s.nameIdx].name, s.value, s.level, s.mark);
            }
            else{
                //Variables
                printf("%d\tvariable\t%s\t-\t%d\t%d\t%d\n", i, identifierTable[s.nameIdx].name, s.level, s.address, s.mark);
            }
        }
    }

    printf("\n");
}