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

//Adds Symbol to symbol table
void addSymbol(int type, int nameIdx, int value, int level, int address, int mark);
//Declares action for a constant variable
void constDecleration();
//Declares action for a variable
void varDecleration();
//Contains structure of code block
void block();
//Contains structure of whole program
void program();
//Helper function to find symbolTable index
int findSymbol(int nameIdx);

void statement();

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
    fclose(tokens);

    //Scanning names into local nametable
    Identifier i;
    while(fscanf(nametable, "%d %s %d %d", &i.index, i.name, &i.line, &i.col) == 4){
        identifierTable[iCount] = i;
        iCount++;
    }
    fclose(nametable);

    //Scanning tokenTable to get symbolTable
    program();


   
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
    }

    //Consumes semi-colon
    currentToken++;
}

void block(){
    //Checks for const keyword
    if(currentToken < tCount && tokenTable[currentToken].tCode == 31){
        constDecleration();
    }
    //Checks for var keyword
    if(currentToken < tCount && tokenTable[currentToken].tCode == 32){
        varDecleration();
    }
}

void program(){
    block();
    //Checks if token is period
    if(currentToken < tCount && tokenTable[currentToken].tCode == 17){
        //Consumers period
        currentToken++;
    }

    else{
        //Error for when program doesn't end with period
        return;
    }

    if(currentToken != tCount){
        //Error for when program ends but tokens still remain after period
        return;
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
    
}
