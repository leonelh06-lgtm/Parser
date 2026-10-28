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

int main(){
    FILE* tokens = fopen("tokens.txt", "r");
    FILE* nametable = fopen("nametable.txt", "r");

    if(tokens == NULL || nametable == NULL)
        error(18);
    
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
        error(17);
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
        printf("insert entry %d: constant %s, level 0, value %d\n", symbolCount, identifierTable[nameIdx].name, value);
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

        if(findSymbol(nameIdx) >= 0){
            //Error 14: Identifier already declared in block
            error(14);
        }

        consumeToken("const-decl");

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

        if(findSymbol(nameIdx) >= 0){
            //Error 14: Identifier already declared in block
            error(14);
        }
        //Consume identifier
        consumeToken("var-decl");

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

        if(symbolIdx < 0){
            //Error 15: undeclared identifier
            error(15);
        }

        //Generates output for assignmnet statements lookup
        traceLookup(symbolIdx);

        if(symbolTable[symbolIdx].type != 2){
            //Error 16: can't assign to number
            error(16);
        }

        //Consumes identifier
        consumeToken("statement");
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

        if(symbolIdx < 0){
            //Error 15 Undeclared identifer
            error(15);
        }

        //Generates lookup trace output
        traceLookup(symbolIdx);

        //Makes sure identifier is a variable
        if (symbolTable[symbolIdx].type != 2){
            //Error 16: Can only read variables
            error(16);
        }
        //Consumes identifer
        consumeToken("statement");
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
        if(symbolIdx < 0){
            //Error 15: Undeclared identifier
            error(15);
        }

        //Traces lookup for symbol
        traceLookup(symbolIdx);

        //Consumes identifier
        consumeToken("factor");

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

    //Handles file opening issue
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
        snprintf(buffer, sizeof(buffer), "%d", t.val);
        return buffer;
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

    printf("%d %d %d %s %s\n", /*Simulates 1 based array like how the assignment asks*/currentToken + 1, tokenTable[currentToken].tCode, tokenTable[currentToken].val, getTokenText(currentToken),owner);

    currentToken++;
}

void traceLookup(int symbolIdx){
    Symbol s = symbolTable[symbolIdx];

    printf("lookup %s: entry %d\n", identifierTable[s.nameIdx].name, symbolIdx);
}