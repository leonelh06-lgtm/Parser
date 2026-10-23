#include <stdio.h>
#include <stdlib.h>

int main(){
    FILE* tokens = fopen("tokens.txt", "r");
    FILE* nametable = fopen("nametable.txt", "r");
    
    typedef struct{
        int tCode;
        int val;
    } Token;

    typedef struct {
        char name[13];
        int line;
        int col;
    } Identifier;

    typedef struct{
        int type;
        int name;
        int value;
        int level;
        int address;
        int mark;
    }  Symbol;
    
    int errCode = 0;
    if(tokens == NULL || nametable == NULL){
        errCode = 12;
    }



    fclose(tokens);
    fclose(nametable);
    return 0;
}