#include "header.h"

//Files
FILE *fileMCode;

//Global variables
symTable symbolTable[MAX_SYMBOL_TABLE_SIZE];
Token currentToken;
int symTablePos = 0;
int tokenTablePos = 0;

instruction MCode[MAX_CODE_LENGTH]; //MCodeTable
int MCodePos = 0; //currentMCodeTableIndex

int currentM = 0; //currentMAddress
int lexLevel = 0; //lexiLevel

void getToken();
void block();
void pushCode(int OP, int L, int M);
void constFound();
void pushSymTable(int kind, Token token, int L, int M, int num);
int  toInt(char *num);
void varFound();
void procedureFound();
void statement();
int  searchSym(char *name);
void expression();
void term();
void factor();
void condition();
void toFile();
void analyze();
void emptySyms(int l);

//Run the main section
void parser(int flag){

    fileMCode = fopen(nameMCode,"w");
    if(fileMCode == NULL)
        printError(ERROR_INVALID_FILE);

    analyze();
    printf("\nNo errors, program is syntactically correct.\n");
    toFile();
    fclose(fileMCode);
}

//Fetches the next token to use
void fetchToken(){
    currentToken = tokenList[tokenTablePos];
    tokenTablePos++;
}

//Do the bulk of the things
void analyze(){
    fetchToken(); //Grab first token to test
    block(); // program: block "."
    if(currentToken.type != periodsym){ //"."
        printError(ERROR_PERIOD_EXPECTED);
    }
}

//the bulkiness
void block(){
    int tempBlockPos = MCodePos, temPos;
    currentM = 0;

    pushCode(7,0,0); //Start.

    while(currentToken.type == constsym || currentToken.type == varsym){
    if(currentToken.type == constsym)
        constFound();
    if(currentToken.type == varsym)
        varFound();
    }

    temPos = currentM; //Store current MCode pos

    while(currentToken.type == procsym)
        procedureFound();

    MCode[tempBlockPos].M = MCodePos;

    pushCode(6,0,temPos + 4);
    statement();
    if(currentToken.type != periodsym)
        pushCode(2,0,0); //return proc???
    else
        pushCode(9,0,2);
}

//push the code
void pushCode(int OP, int L, int M){
    MCode[MCodePos].OP = OP;
    MCode[MCodePos].M = M;
    MCode[MCodePos].L = L;
    MCodePos++;
}

//deal with consts
void constFound(){
    Token tempT;
    do{
        fetchToken();
        if(currentToken.type != identsym)
            printError(34); //const/int/proc must have ident after

        strcpy(tempT.name, currentToken.name); //copy into temp

        fetchToken();
        if(currentToken.type != eqlsym)
            printError(3); //equals wanted after const declaration

        fetchToken();
        if(currentToken.type != numbersym)
            printError(2); //number wanted after equals in const

        pushSymTable(1, tempT, lexLevel, -5, toInt(currentToken.name));
        fetchToken();
    } while(currentToken.type == commasym);

    if(currentToken.type != semicolonsym)
        printError(19); //semicolon needed between statements

    fetchToken();
}

//Deal with vars
void varFound(){
    int run = 1;
    do{
        fetchToken();
        if(currentToken.type != identsym)
            printError(34); //const/int/proc must have ident after

        pushSymTable(2, currentToken, lexLevel, currentM+4, 0);
        fetchToken();
    } while(currentToken.type == commasym);

    if(currentToken.type != semicolonsym)
        printError(19); //semicolon needed between statements

    fetchToken();
}

//deal with procedure declarations
void procedureFound(){
    fetchToken();
    if(currentToken.type != identsym)
        printError(34); //const/int/proc must have ident after

    lexLevel++;

    pushSymTable(3, currentToken, lexLevel, currentM, -1);

    fetchToken();
    if(currentToken.type != semicolonsym)
        printError(19); //semicolon needed between statements

    fetchToken();
    block(); //run for the proc's insides
    lexLevel--;

    if(currentToken.type != semicolonsym)
        printError(19); //semicolon needed between statements

    fetchToken();
}

void statement(){
    int symbolPos, identPos, tempBPos, temPos, temPos2;

    if(currentToken.type == identsym){
        symbolPos = searchSym(currentToken.name);
        printf("%s at pos: %d.\n",currentToken.name,symbolPos);

        if(symbolPos == -1)
            printError(11); //undeclared variable found
        else if(symbolTable[symbolPos].kind == 1)
            printError(12); //assignment to const/proc not valid

        identPos = symbolTable[symbolPos].addr;

        fetchToken();
        if(currentToken.type != becomessym){
            if(currentToken.type == eqlsym)
                printError(13); //use := not =
            else
                printError(13); // := expected
        }

        fetchToken();
        expression();

        if(currentToken.type != semicolonsym)
            printError(19); //semicolon needed between statements

        pushCode(4, lexLevel-symbolTable[symbolPos].level, identPos);
    }
    else if(currentToken.type == callsym){
        fetchToken();

        symbolPos = searchSym(currentToken.name);

        if(symbolPos == -1)
            printError(11); //undeclared variable found
        else if(symbolTable[symbolPos].kind == 1)
            printError(12); //assignment to const/proc not valid

        fetchToken();

        pushCode(5, lexLevel, symbolTable[symbolPos].addr);

    }
    else if(currentToken.type == beginsym){
        fetchToken();
        statement();

        while(currentToken.type == semicolonsym){
            fetchToken();
            statement();
        }
        printf("on %d.\n", currentToken.type);
        if(currentToken.type != endsym)
            printError(17); //semicolon or } expected

        fetchToken();
        lexLevel++; //!?
        emptySyms(lexLevel);
        lexLevel--; //!?
    }
    else if(currentToken.type == ifsym){
        fetchToken();
        condition();
        if(currentToken.type != thensym)
            printError(16); // then expected after if

        fetchToken();
        tempBPos = MCodePos;

        pushCode(8,0,0);

        statement();
        MCode[tempBPos].M = MCodePos;

        fetchToken();
        if(currentToken.type == elsesym){
            MCode[tempBPos].M = MCodePos+1;

            tempBPos = MCodePos;

            pushCode(7,0,0);

            fetchToken();
            statement();
            MCode[tempBPos].M = MCodePos;
        }
    }
    else if(currentToken.type == whilesym){
        temPos = MCodePos;

        fetchToken();
        condition();

        temPos2 = MCodePos;

        pushCode(8,0,0);

        if(currentToken.type != dosym)
            printError(18); //do expected after while

        fetchToken();
        statement();

        pushCode(7,0,temPos);

        MCode[temPos2].M = MCodePos;
    }
    else if(currentToken.type == readsym){
        fetchToken();

        if(currentToken.type == identsym){
            symbolPos = searchSym(currentToken.name);
            if(symbolPos == -1)
                printError(11); //undeclared variable found
            fetchToken();

            pushCode(9,0,1); //read from screen

            pushCode(4,0,symbolTable[symbolPos].addr); //increment mcode
        }
    }
    else if(currentToken.type == writesym){
        fetchToken();

        if(currentToken.type == identsym){
            symbolPos = searchSym(currentToken.name);
            if(symbolPos == -1)
                printError(11); //undeclared variable found
            fetchToken();

            pushCode(3,0,symbolTable[symbolPos].addr); //read from screen

            pushCode(9,0,0); //output statement
        }
    }
}


//expression work
void expression(){
    int thisOp;

    if(currentToken.type == plussym || currentToken.type == minussym){
        thisOp = currentToken.type;

        if(thisOp == minussym)
            pushCode(2,0,1);
    }
    term();

    while(currentToken.type == plussym || currentToken.type == minussym){
        thisOp = currentToken.type;
        fetchToken();
        term();

        if(thisOp == plussym)
            pushCode(2,0,2);
        else
            pushCode(2,0,3);
    }
}

//terms and things
void term(){
    int thisOp;

    factor();

    while(currentToken.type == multsym || currentToken.type == slashsym){
        thisOp = currentToken.type;
        fetchToken();
        factor();

        if(thisOp == multsym)
            pushCode(2,0,4);
        else
            pushCode(2,0,5);
    }
}

//factors and fun
void factor(){
    int symPos;

    if(currentToken.type == identsym){
        symPos = searchSym(currentToken.name);

        if(symPos == -1)
            printError(11); //undeclared var

        if(symbolTable[symPos].kind == 1)
            pushCode(1, 0, symbolTable[symPos].val);
        else
            pushCode(3, lexLevel-symbolTable[symPos].level, symbolTable[symPos].addr);

        fetchToken();
    }
    else if(currentToken.type == numbersym){
        pushCode(1, 0, toInt(currentToken.name));
        fetchToken();
    }
    else if (currentToken.type == lparentsym){
        fetchToken();
        expression();

        if(currentToken.type != rparentsym)
            printError(22); //error: ) missing

        fetchToken();
    }
    else
        printError(23); //cannot begin with this symbol
}

void condition(){
    int thisOp;

    if(currentToken.type == oddsym){
        pushCode(2,0,6);
        fetchToken();
        expression();
    }
    else{
        expression();
        thisOp = currentToken.type;

        switch (currentToken.type) {
            case eqlsym:
                thisOp = 8;
                break;

            case neqsym:
                thisOp = 9;
                break;

            case lessym:
                thisOp = 10;
                break;

            case leqsym:
                thisOp = 11;
                break;

            case gtrsym:
                thisOp = 12;
                break;

            case geqsym:
                thisOp = 13;
                break;

            default:
                printError(20); //relational op needed.
                break;
        }

        fetchToken();
        expression();
        pushCode(2,0,thisOp);
    }
}

//find a variable in the symbol table
int searchSym(char *name){
    int i;
    printf("search from %d >= %d.\n", symTablePos-1, 0);
    for(i=symTablePos-1; i >= 0; i--){
        if(strcmp(name,symbolTable[i].name) == 0 && symbolTable[i].addr != -1)
            return i;
        if(symbolTable[i].addr == -1)
            printf("GAH!\n");
    }
    return -1; //not found :(
}

//mark old syms as invalid with -1
void emptySyms(int level){
    int i;
    printf("emptying %d to %d.\n",0, symTablePos-1);
    for(i=symTablePos-1; i >= 0; i--){
        if(symbolTable[i].level == level && symbolTable[i].kind != 3){
            printf("emptied %s.\n",symbolTable[i].name);
            symbolTable[i].addr = -1;
        }
    }
}

//Add to sym table
void pushSymTable(int kind, Token t, int L, int M, int num){
    symbolTable[symTablePos].kind = kind;
    strcpy(symbolTable[symTablePos].name,t.name);
    symbolTable[symTablePos].level = L;
    symbolTable[symTablePos].addr = M;
    if(kind == 1)
        symbolTable[symTablePos].val = num;
    else if (kind == 2)
        currentM++;
    symTablePos++;
}

int toInt(char *num){
    int returner = 0, i = 0;
    while(num[i] != '\0'){
        returner *= 10;
        returner += num[i] - '0';
        i++;
    }
    return returner;
}

void toFile(){
    int i;
    for(i=0; i< MCodePos; i++){
        fprintf(fileMCode,"%d %d %d\n",MCode[i].OP, MCode[i].L, MCode[i].M);
    }
}
