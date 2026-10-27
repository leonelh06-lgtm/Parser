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

void condition();

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

    statement();
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
    if(currentToken > tCount){
        return;
    }

    //Checks for assignment statements
    if(currentToken < tCount && tokenTable[currentToken].tCode == 1){
        int nameIdx = tokenTable[currentToken].val;
        int symbolIdx = findSymbol(nameIdx);

        if(symbolIdx < 0){
            //Error: undeclared identifier
            return;
        }

        if(symbolTable[symbolIdx].type != 2){
            //Error: can't assign to number
            return;
        }

        //Consumes identifier
        currentToken++;
        //Checks for "="
        if(tokenTable[currentToken].tCode != 18){
            //Error: expected =
            return;
        }

        //Consumes :=
        currentToken++;

        expression();
    }

    //Looks for begin statements
    else if(currentToken < tCount && tokenTable[currentToken].tCode == 20){
        //Consumes begin
        currentToken++;
        statementList();
        //Looks for end
        if(currentToken < tCount && tokenTable[currentToken].tCode != 21){
            //Error missing end statement
            return;
        }
        //Consumes end
        currentToken++;
    }

    //Looks for If then statements
    else if(currentToken < tCount && tokenTable[currentToken].tCode == 22){
        //Consumes if and checks for condition
        currentToken++;
        condition();

        if(currentToken >= tCount || tokenTable[currentToken].tCode != 24){
            //Error: no then statment
            return;
        }
        //Consumes then
        currentToken++;

        statementList();

        if(currentToken < tCount && tokenTable[currentToken].tCode == 36){
            //Consumes else
            currentToken++;
            statementList();
        }
        
        //Checks for "fi"
        if(currentToken >= tCount || tokenTable[currentToken].tCode != 23){
            //Error: no fi to show statement end
            return;
        }
        //Consume fi
        currentToken++;
    }

    //Looks for while/do statements
    else if(currentToken < tCount && tokenTable[currentToken].tCode == 25){
        //Consumes while
        currentToken++;
        condition();

        if(currentToken < tCount && tokenTable[currentToken].tCode != 24){
            //Error: no then statmement
            return;
        }
        //Consume then
        currentToken++;
        statementList();
        //Checks for elihw finishing of while
        if(currentToken >= tCount || tokenTable[currentToken].tCode != 26){
            //Error: no elihw statmement
            return;
        }
        //Consumes od
        currentToken++;
    }

    //Looks for read
    else if(tokenTable[currentToken].tCode == 35){
        //Consume read
        currentToken++;

        //Checks if its not identifier
        if(currentToken >= tCount || tokenTable[currentToken].tCode != 1){
            //Error 2: Expected identifier
            return;
        }

        int nameIdx = tokenTable[currentToken].val;
        int symbolIdx = findSymbol(nameIdx);

        if(symbolIdx < 0){
            //Error 15 Undeclared identifer
            return;
        }

        //Makes sure identifier is a variable
        if (symbolTable[symbolIdx].type != 2){
            //Error 16: Can only read variables
        }
        //Consumes identifer
        currentToken++;
    }

    //Looks for write
    else if(tokenTable[currentToken].tCode == 34){
    // Consume write
    currentToken++;

    expression();
    }
}


void statementList(){
    statement();

    while(currentToken < tCount && tokenTable[currentToken].tCode == 16){
        //Consumes ";"
        currentToken++;
        statement();
    }
}

void factor(){
    if(currentToken >= tCount){
        //Error: Expected Factor
        return;
    }

    int tokenType = tokenTable[currentToken].tCode;

    //Checks for identifier
    if(tokenType == 1){
        int nameIdx = tokenTable[currentToken].val;

        if(findSymbol(nameIdx) < 0){
            //Error: Undeclared identifier
            return;
        }

        //Consumes identifier
        currentToken++;

    }

    //Checks for numbers
    else if(tokenType == 2){
        //Consumes number
        currentToken++;
    }

    //Handles "("
    else if(tokenType == 13){
        currentToken++;

        //Call expression
        expression();

        //Checks for valid closing parenthesis ")"
        if(tokenTable[currentToken].tCode == 14){
            //Consumes closing parenthesis
            currentToken++;
        }
        else{
            return;
            //Error: Invalid closing parenthesis
        }
    }

    else{
        //Invalid factor
        return;
    }
}

void expression(){
    //Consumes term in expression
    term();

    //Checks for "+" or "-"
    while(currentToken < tCount  && (tokenTable[currentToken].tCode == 3 || tokenTable[currentToken].tCode == 4)){
        //Consumes "+" or "-"
        currentToken++;
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
        currentToken++;
        factor();
    }
}

void condition(){
    expression();
    if(currentToken < tCount && (tokenTable[currentToken].tCode == 7 || tokenTable[currentToken].tCode == 8 || tokenTable[currentToken].tCode == 9 || tokenTable[currentToken].tCode == 10 || tokenTable[currentToken].tCode == 11 || tokenTable[currentToken].tCode == 12)){
        currentToken++;
        expression();
    }
    else{
        //Error: Invalid comparison opperator
        return;
    }
}
