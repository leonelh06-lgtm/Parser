#include <stdio.h>
#include <stdlib.h>
//Defines tokens
    typedef struct{
        int tCode;
        int val;
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
void addSymbol(int type, int name, int value, int level, int address, int mark);
void constDecleration();
void varDecleration();
void block();

int main(){
    FILE* tokens = fopen("tokens.txt", "r");
    FILE* nametable = fopen("nametable.txt", "r");

    if(tokens == NULL || nametable == NULL)
        return 1;
    
    

    //Scanning tokens into local tokentable
    Token t;
    while(fscanf(tokens, "%d", &t.tCode) == 1){
        if(t.tCode == 1 || t.tCode == 2){
            fscanf(tokens, "%d", &t.val);
        }
        else{
            t.val = 0;
        }
        tokenTable[tCount] = t;
        tCount++;
    }

    //Scanning names into local nametable
    Identifier i;
    while(fscanf(nametable, "%d %s %d %d", &i.index, i.name, &i.line, &i.col) == 4){
        identifierTable[iCount] = i;
        iCount++;
    }

    //Scanning tokenTable to get symbolTable
    


   
    return 0;
}

/*Adds symbols to symbol table (needs a function unlike identifier and tokens
since it requires context of tokens to decide symbol paramters)*/
void addSymbol(int type, int nameIdx, int value, int address){
    if(symbolCount >= 100)
        return;
    Symbol s;
    s.type = type;
    s.value = value;
    s.address = address;
    s.level = 0;
    s.mark = 0;
    s.nameIdx = nameIdx;
    symbolTable[symbolCount] = s;
    symbolCount++;
}

void constDecleration(){
    //Moves past const keyword
    currentToken++;
    //Gets nameTable index and moves past identifer
    int nameIdx = tokenTable[currentToken].val;
    currentToken++;

    //Moves past :=
    currentToken++;

    //gets the value assigned to the identifier then consumes value
    int value = tokenTable[currentToken].val;
    currentToken++;

    //We now have the tokenTable context to create the symbol
    addSymbol(1, nameIdx, value, 0);
    currentToken++;
}

void varDecleration(){
    //Moves past var keyword
    currentToken++;

    //Gets nametable index
    int nameIdx = tokenTable[currentToken].val;
    currentToken++;

    //Add to symbolTable then increment address for the next variable
    addSymbol(2, nameIdx, 0, nextAddress);
    nextAddress++;
    currentToken++;

    //Checks for multiple var's seperated by commas         tCode 15 = ","
    while(currentToken < tCount && tokenTable[currentToken].tCode == 15){
        //Just a repetition of adding a var

        //Consumes comma
        currentToken++;

        nameIdx = tokenTable[currentToken].val;
        currentToken++;

        addSymbol(2, nameIdx, 0, nextAddress);
        nextAddress++;
        currentToken++;
    }

    //Consumes semi-colon
    currentToken++;
}