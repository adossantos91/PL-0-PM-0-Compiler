#include "header.h"

//Globals
symTable symbolTable[MAX_SYMBOL_TABLE_SIZE];
Token currentToken;
int symTablePos = 0, tokenTablePos = 0;

//Various
instruction MCode[MAX_CODE_LENGTH];
int MCodePos = 0, currentM = 0, lexLevel = 0;
int column = 0, row = 1;
int counted = 0, numProcedures = 0;
int varLevel = 0, constLevel = 0, varNum = 0, constNum = 0;
char currentProc[identMax];

//Procedures
void analyze();
void runBlock();
void constFound();
void varFound();
void statementFound();
void expressionFound();
void termFound();
void factorFound();
void operationFound();
void procedureFound();
void voidSyms(int level);
void pushSymTable(int kind, Token t, int L, int M, int num);
void fetchToken();
void printMCode(int flag);
void toCode(int OP, int L, int M);
void toFile();

//Run the main section
void parser(int flag, int flag2){
    int i;

    fileMCode = fopen(nameMCode,"w");
    if(fileMCode == NULL)
        printError(1);

    analyze();
    printf("\n=============================================\nNo errors, program is syntactically correct.\n=============================================\n\n");

    if(flag2){
        printf("=============================================\nSymbol Table: kind name L M val\n=============================================\nKind       Name  L Pos Val\n");
        for(i=0; i<symTablePos;i++){
            printf("%4d %10s %2d %3d %3d\n",symbolTable[i].kind,symbolTable[i].name,symbolTable[i].level,symbolTable[i].addr,symbolTable[i].val);
        }
        printf("\n");
    }
    toFile();
    fclose(fileMCode);
    printMCode(flag);
}
//Do the bulk of the things
void analyze(){
    fetchToken(); //Grab first token to test
    runBlock(); // program: block "."
    if(currentToken.type != periodsym){ //"."
        printf("\nError: Line:%d, column:%d :: ",row,column);
        printError(24);
    }
}
//the real bulkiness
void runBlock(){
    counted = 0;
    int tempBlockPos = MCodePos, temPos;
    currentM = 0;

    toCode(7,0,0); //Start.

    while(currentToken.type == constsym || currentToken.type == varsym){
    if(currentToken.type == constsym)
        constFound();
    if(currentToken.type == varsym)
        varFound();
    counted++;
    }

    temPos = currentM; //Store current MCode pos

    while(currentToken.type == procsym)
        procedureFound();

    MCode[tempBlockPos].M = MCodePos;

    toCode(6,0,temPos + 4);
    statementFound();
    if(currentToken.type != periodsym && currentToken.type == semicolonsym){
        toCode(2,0,0); //return proc
        voidSyms(lexLevel);
    }
    else
        toCode(9,0,2);
}
//deal with consts
void constFound(){
    Token tempT;
    int returner = 0;
    if(constNum >= 1){
        if(constLevel == lexLevel){
            printf("\nError: Line:%d, column:%d :: ",row,column);
            printError(25);
        }
    }
    constNum++;
    constLevel = lexLevel;
    do{
        fetchToken();
        if(currentToken.type != identsym){
            printf("\nError: Line:%d, column:%d :: ",row,column);
            printError(8); //const/int/proc must have ident after
        }
        returner = searchSym(currentToken.name, lexLevel);
        if(returner != -1 && symbolTable[returner].level == lexLevel){
            printf("\nError: Line:%d, column:%d :: ",row,column);
            printError(26); //semicolon needed between statements
        }

        strcpy(tempT.name, currentToken.name); //copy into temp

        fetchToken();
        if(currentToken.type != eqlsym){
            printf("\nError: Line:%d, column:%d :: ",row,column);
            printError(4); //equals wanted after const declaration
        }

        fetchToken();
        if(currentToken.type != numbersym){
            printf("\nError: Line:%d, column:%d :: ",row,column);
            printError(5); //number wanted after equals in const
        }

        pushSymTable(1, tempT, constLevel, -5, toInt(currentToken.name));
        fetchToken();
    } while(currentToken.type == commasym);

    if(currentToken.type != semicolonsym){
        printf("\nError: Line:%d, column:%d :: ",row,column);
        printError(13); //semicolon needed between statements
    }

    fetchToken();
}
//Deal with vars
void varFound(){
    int returner = 0;
    if(varNum >= 1){
        if(varLevel == lexLevel){
            printf("\nError: Line:%d, column:%d :: ",row,column);
            printError(25);
        }
    }
    varNum++;
    varLevel = lexLevel;

    do{
        fetchToken();
        if(currentToken.type != identsym){
            printf("\nError: Line:%d, column:%d :: ",row,column);
            printError(8); //const/int/proc must have ident after
        }//Test to see if it exists already

        returner = searchSym(currentToken.name, lexLevel);
        if(returner != -1 && symbolTable[returner].level == lexLevel){
            printf("\nError: Line:%d, column:%d :: ",row,column);
            printError(26); //semicolon needed between statements
        }
        pushSymTable(2, currentToken, lexLevel, currentM+4, 0);
        fetchToken();
    } while(currentToken.type == commasym);

    if(currentToken.type != semicolonsym){
        printf("\nError: Line:%d, column:%d :: ",row,column);
        printError(13); //semicolon needed between statements
    }

    fetchToken();
}
//Deal with a statement line
void statementFound(){
    int symPos, identPos, tempBPos, temPos, temPos2;

    if(currentToken.type == identsym){
        symPos = searchSym(currentToken.name, lexLevel);

        if(symPos == -1){
            printf("\nError: Line:%d, column:%d :: ",row,column);
            printf("Identifier '%s': ", currentToken.name);
            printError(15); //undeclared variable found
        }
        else if(symbolTable[symPos].kind == 1){
            printf("\nError: Line:%d, column:%d :: ",row,column);
            printError(12); //assignment to const/proc not valid
        }

        identPos = symbolTable[symPos].addr;

        fetchToken();
        if(currentToken.type != becomessym){
            if(currentToken.type == eqlsym){
                printf("\nError: Line:%d, column:%d :: ",row,column);
                printError(3); //use := not =
            }
            else{
                printf("\nError: Line:%d, column:%d :: ",row,column);
                printError(9); // := expected
            }
        }

        fetchToken();
        expressionFound();

        if(currentToken.type != semicolonsym){
            printf("\nError: Line:%d, column:%d :: ",row,column);
            printError(13); //semicolon needed between statements
        }

        toCode(4, lexLevel-symbolTable[symPos].level, identPos);
    }
    else if(currentToken.type == callsym){
        fetchToken();

        if(currentToken.type != identsym){
            printf("\nError: Line:%d, column:%d :: ",row,column);
            printError(10); //assignment to const/proc not valid
        }

        symPos = searchSym(currentToken.name, lexLevel);

        if(symPos == -1){
            printf("\nError: Line:%d, column:%d :: ",row,column);
            printf("Identifier '%s': ", currentToken.name);
            printError(15); //undeclared variable found
        }
        else if(symbolTable[symPos].kind == 1){
            printf("\nError: Line:%d, column:%d :: ",row,column);
            printError(12); //assignment to const/proc not valid
        }

        fetchToken();

        toCode(5, lexLevel, symbolTable[symPos].addr);

    }
    else if(currentToken.type == beginsym){
        fetchToken();
        statementFound();

        while(currentToken.type == semicolonsym){
            fetchToken();
            statementFound();
        }
        if(currentToken.type != endsym){
            printf("\nError: Line:%d, column:%d :: ",row,column);
            printError(14); //endsym expected
        }

        fetchToken();
    }
    else if(currentToken.type == ifsym){
        fetchToken();
        operationFound();
        if(currentToken.type != thensym){
            printf("\nError: Line:%d, column:%d :: ",row,column);
            printError(6); // then expected after if
        }

        fetchToken();
        tempBPos = MCodePos;

        toCode(8,0,0);

        statementFound();
        MCode[tempBPos].M = MCodePos;

        fetchToken();

        if(currentToken.type != elsesym){
            tokenTablePos--;
            tokenTablePos--;
            currentToken.type = tokenList[tokenTablePos].type;
            strcpy(currentToken.name,tokenList[tokenTablePos].name);
            while(currentToken.type == newlinesym){
                tokenTablePos--;
                currentToken.type = tokenList[tokenTablePos].type;
                strcpy(currentToken.name,tokenList[tokenTablePos].name);
            }
            column--;
        }

        if(currentToken.type == elsesym){
            MCode[tempBPos].M = MCodePos+1;

            tempBPos = MCodePos;

            toCode(7,0,0);

            fetchToken();
            statementFound();
            MCode[tempBPos].M = MCodePos;
        }
    }
    else if(currentToken.type == whilesym){
        temPos = MCodePos;

        fetchToken();
        operationFound();

        temPos2 = MCodePos;

        toCode(8,0,0);

        if(currentToken.type != dosym){
            printf("\nError: Line:%d, column:%d :: ",row,column);
            printError(7); //do expected after while
        }

        fetchToken();
        statementFound();

        toCode(7,0,temPos);

        MCode[temPos2].M = MCodePos;
    }
    else if(currentToken.type == readsym){
        fetchToken();

        if(currentToken.type == identsym){
            symPos = searchSym(currentToken.name, lexLevel);
            if(symPos == -1){
                printf("\nError: Line:%d, column:%d :: ",row,column);
                printf("Identifier '%s': ", currentToken.name);
                printError(15); //undeclared variable found
            }
            fetchToken();

            toCode(9,0,1); //read from screen

            toCode(4,0,symbolTable[symPos].addr); //increment mcode
        }
    }
    else if(currentToken.type == writesym){
        fetchToken();

        if(currentToken.type == identsym){
            symPos = searchSym(currentToken.name, lexLevel);
            if(symPos == -1){
                printf("\nError: Line:%d, column:%d :: ",row,column);
                printf("Identifier '%s': ", currentToken.name);
                printError(15); //undeclared variable found
            }
            fetchToken();
            if(symbolTable[symPos].kind == 1)
                toCode(1,0,symbolTable[symPos].val); //if constant
            else
                toCode(3,0,symbolTable[symPos].addr); //read from screen

            toCode(9,0,0); //output
        }
        else{
            printf("\nError: Line:%d, column:%d :: ",row,column);
            printError(10); //undeclared variable found
        }
    }
}
//Expression work
void expressionFound(){
    int thisOp;

    if(currentToken.type == plussym || currentToken.type == minussym){
        thisOp = currentToken.type;

        if(thisOp == minussym){
            fetchToken();
            termFound();
            toCode(2,0,1);
        }
    }
    else{
        termFound();
    }

    while(currentToken.type == plussym || currentToken.type == minussym){
        thisOp = currentToken.type;
        fetchToken();
        termFound();

        if(thisOp == plussym)
            toCode(2,0,2);
        else
            toCode(2,0,3);
    }
}
//terms and things
void termFound(){
    int thisOp;

    factorFound();

    while(currentToken.type == multsym || currentToken.type == slashsym){
        thisOp = currentToken.type;
        fetchToken();
        factorFound();

        if(thisOp == multsym)
            toCode(2,0,4);
        else
            toCode(2,0,5);
    }
}
//factors and fun
void factorFound(){
    int symPos;

    if(currentToken.type == identsym){
        symPos = searchSym(currentToken.name, lexLevel);

        if(symPos == -1){
            printf("\nError: Line:%d, column:%d :: ",row,column);
            printf("Identifier '%s': ", currentToken.name);
            printError(15); //undeclared variable found
        }

        if(symbolTable[symPos].kind == 1)
            toCode(1, 0, symbolTable[symPos].val);
        else
            toCode(3, lexLevel-symbolTable[symPos].level, symbolTable[symPos].addr);

        fetchToken();
    }
    else if(currentToken.type == numbersym){
        toCode(1, 0, toInt(currentToken.name));
        fetchToken();
    }
    else if (currentToken.type == lparentsym){
        fetchToken();
        expressionFound();

        if(currentToken.type != rparentsym){
            printf("\nError: Line:%d, column:%d :: ",row,column);
            printError(16); //error: ) missing
        }

        fetchToken();
    }
    else{
        printf("\nError: Line:%d, column:%d :: '%s'",row,column,currentToken.name);
        printError(14); //cannot begin with this symbol
    }
}
//Mathematical operation
void operationFound(){
    int thisOp;
    if(currentToken.type == oddsym){
        toCode(2,0,6);
        fetchToken();
        expressionFound();
    }
    else{
        expressionFound();
        thisOp = currentToken.type;

        switch (currentToken.type) {
            case becomessym:
                printf("\nError: Line:%d, column:%d :: ",row,column);
                printError(2);
                break;
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
                printf("\nError: Line:%d, column:%d :: ",row,column);
                printError(11); //relational op needed.
                break;
        }
        fetchToken();
        expressionFound();
        toCode(2,0,thisOp);
    }
}
//deal with procedure declarations
void procedureFound(){
    numProcedures++;
    fetchToken();
    if(currentToken.type != identsym){
        printf("\nError: Line:%d, column:%d :: ",row,column);
        printError(8); //const/int/proc must have ident after
    }
    strcpy(currentProc,currentToken.name);

    pushSymTable(3, currentToken, lexLevel, MCodePos, -1);

    lexLevel++;
    if(lexLevel > MAX_LEXI_LEVELS){
        printf("\nError: :: ");
        printError(23);
    }
    numProcedures++;
    varNum = 0;

    fetchToken();
    if(currentToken.type != semicolonsym){
        printf("\nError: Line:%d, column:%d :: ",row,column);
        printError(13); //semicolon needed between statements
    }

    fetchToken();
    runBlock(); //run for the proc's insides
    lexLevel--;

    if(currentToken.type != semicolonsym){
        printf("\nError: Line:%d, column:%d :: ",row,column);
        printError(13); //semicolon needed between statements
    }
    strcpy(currentProc," ");
    fetchToken();
}
//find a variable in the symbol table
int searchSym(char *name, int level){
    int i;
    while(level != -1){
        for(i=symTablePos-1; i >= 0; i--){
            if((strcmp(name,symbolTable[i].name) == 0) && (symbolTable[i].addr != -1) && (symbolTable[i].level == level)){
                return i;
            }
        }
        level--;
    }
    return -1; //not found :(
}
//mark old syms as invalid with -1
void voidSyms(int level){
    int i;
    for(i=symTablePos-1; i >= 0; i--){
        if(symbolTable[i].level == level && symbolTable[i].kind != 3 && symbolTable[i].addr != -1){
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
    else if (kind == 3){
        procedures[procPos][0] = M;
        procedures[procPos][1] = L+1;
        procPos++;
    }
    symTablePos++;
}
//Fetches the next token to use
void fetchToken(){
    currentToken = tokenList[tokenTablePos];
    tokenTablePos++;
    if(currentToken.type == newlinesym){
        while(currentToken.type == newlinesym){
            row++;
            column = 0;
            fetchToken();
        }
    }
    else
        column++;
}
//Print the MCode to the screen if needed
void printMCode(int flag){
    char c;
    fileMCode = fopen(nameMCode,"r");
    if(fileMCode == NULL)
        printError(1);
    if(flag){
        printf("========================\nGenerated Machine Code\n========================\n");
        c = fgetc(fileMCode);
        while(c != EOF){
            printf("%c",c);
            c = fgetc(fileMCode);
        }
        printf("\n\n");
    }
    fclose(fileMCode);
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
//push the code
void toCode(int OP, int L, int M){
    MCode[MCodePos].OP = OP;
    MCode[MCodePos].M = M;
    MCode[MCodePos].L = L;
    MCodePos++;
}

void toFile(){
    int i;
    for(i=0; i< MCodePos; i++){
        fprintf(fileMCode,"%d %d %d\n",MCode[i].OP, MCode[i].L, MCode[i].M);
    }
}
