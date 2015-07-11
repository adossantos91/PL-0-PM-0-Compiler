#include "header.h"

//enums table
typedef enum{
	nulsym = 1, identsym, numbersym, plussym, minussym, multsym, slashsym,
	oddsym, eqlsym, neqsym, lessym, leqsym, gtrsym, geqsym, lparentsym,
	rparentsym, commasym, semicolonsym, periodsym, becomessym, beginsym, endsym,
	ifsym, thensym, whilesym, dosym, callsym, constsym, varsym, procsym, writesym,
	readsym, elsesym, errsym, newlinesym
} token;

//Files
FILE *fileCode;
FILE *fileCleanCode;
FILE *fileLexTable;
FILE *fileLexTableList;

int tokenPos = 0;

//Functions
void removeComments();
void printLexeme(int flag);
void printTable();
void printList(int flag);


void *scanner(int flag){ //if flag is true (-l) print list of lexemes to screen

    fileCode = fopen(nameCode,"r");
    if(fileCode == NULL)
        printError(1);

    removeComments();
    printLexeme(flag);
    //printf("closing\n");
    fclose(fileCode);
}

void removeComments(){

    char numHolder[numMax];
    char varHolder[identMax];
    int numLines = 0;
    int count = 0;


    char scanner;
    int flag = 0;
    int ignore_flag = 0;

    fileCleanCode = fopen(nameCleanCode,"w");
    if(fileCleanCode == NULL)
        printError(1);


    fscanf(fileCode, "%c", &scanner);
    while (scanner != EOF){
        if(flag == 1){ //if comment ignoring is active
            scanner = fgetc(fileCode);
            if(scanner == 10){
                tokenList[tokenPos++].type = newlinesym;
            }
            else if (scanner == '*'){
                scanner = fgetc(fileCode);
                if(scanner == '/'){
                    flag = 0;
                    scanner = fgetc(fileCode);
                }
            }
        }
        else{
            if((int)scanner == 32 || ((int)scanner == 9)){ //space or tab
                fprintf(fileCleanCode,"%c",scanner);
                scanner = fgetc(fileCode);
            } //is space, do nothing.
            else if(((int)scanner == 10) || ((int)scanner == 59)){ //is newline or ';'
                fprintf(fileCleanCode,"%c", scanner);
                if(scanner == 10){
                    tokenList[tokenPos++].type = newlinesym;
                }
                if(scanner == 59){
                    strcpy(tokenList[tokenPos].name, ";");
                    tokenList[tokenPos++].type = semicolonsym;
                }

                numLines++;
                scanner = fgetc(fileCode);
                while(((int)scanner == 10) || ((int)scanner == 59) || ((int)scanner == 32) || ((int)scanner == 9) || ((int)scanner == 3)){
                    fprintf(fileCleanCode,"%c",scanner);
                    if(scanner == 59){
                        strcpy(tokenList[tokenPos].name, ";");
                        tokenList[tokenPos++].type = semicolonsym;
                    }
                    if(scanner == 10){
                        tokenList[tokenPos++].type = newlinesym;
                    }
                    scanner = fgetc(fileCode);
                }
            }

            else if (((int)scanner >= 58 && (int)scanner <= 62) || ((int)scanner >= 40 && (int)scanner <= 47)) { //CASE: is a symbol
                switch(scanner){
                    case '+' :
                        strcpy(tokenList[tokenPos].name, "+");
                        tokenList[tokenPos++].type = plussym;
                        fprintf(fileCleanCode,"%c", scanner);
                        scanner = fgetc(fileCode);
                        break;
                    case '-' :
                        strcpy(tokenList[tokenPos].name, "-");
                        tokenList[tokenPos++].type = minussym;
                        fprintf(fileCleanCode,"%c", scanner);
                        scanner = fgetc(fileCode);
                        break;
                    case '*' :
                        strcpy(tokenList[tokenPos].name, "*");
                        tokenList[tokenPos++].type = multsym;
                        fprintf(fileCleanCode,"%c", scanner);
                        scanner = fgetc(fileCode);
                        break;
                    case '/' :
                        //might be a comment, stay tuned
                        scanner = fgetc(fileCode);
                        if(scanner == '*')
                            flag = 1;
                        else{
                            strcpy(tokenList[tokenPos].name, "/");
                            tokenList[tokenPos++].type = slashsym;
                            fprintf(fileCleanCode,"/");
                        }
                        break;
                    case '=' :
                        strcpy(tokenList[tokenPos].name, "=");
                        tokenList[tokenPos++].type = eqlsym;
                        fprintf(fileCleanCode,"%c", scanner);
                        scanner = fgetc(fileCode);
                        break;
                    case ',' :
                        strcpy(tokenList[tokenPos].name, ",");
                        tokenList[tokenPos++].type = commasym;
                        fprintf(fileCleanCode,"%c", scanner);
                        scanner = fgetc(fileCode);
                        break;
                    case '.' :
                        strcpy(tokenList[tokenPos].name, ".");
                        tokenList[tokenPos++].type = periodsym;
                        fprintf(fileCleanCode,"%c", scanner);
                        scanner = fgetc(fileCode);
                        break;
                    case '>' :
                        scanner = fgetc(fileCode);
                        if (scanner == '='){ //matches '>='
                            strcpy(tokenList[tokenPos].name, ">=");
                            tokenList[tokenPos++].type = geqsym;
                            fprintf(fileCleanCode,">=");
                            scanner = fgetc(fileCode);
                        }
                        else{
                            strcpy(tokenList[tokenPos].name, ">");
                            tokenList[tokenPos++].type = gtrsym;
                            fprintf(fileCleanCode,">");
                        }
                        break;
                    case '(' :
                        strcpy(tokenList[tokenPos].name, "(");
                        tokenList[tokenPos++].type = lparentsym;
                        fprintf(fileCleanCode,"(");
                        scanner = fgetc(fileCode);
                        break;
                    case ')' :
                        strcpy(tokenList[tokenPos].name, ")");
                        tokenList[tokenPos++].type = rparentsym;
                        fprintf(fileCleanCode,"%c", scanner);
                        scanner = fgetc(fileCode);
                        break;
                    case '<' :
                        scanner = fgetc(fileCode);
                        if (scanner == '='){ //matches '<='
                            strcpy(tokenList[tokenPos].name, "<=");
                            tokenList[tokenPos++].type = leqsym;
                            fprintf(fileCleanCode,"<=");
                            scanner = fgetc(fileCode);
                        }
                        else if (scanner == '>'){ //matches <>
                            strcpy(tokenList[tokenPos].name, "<>");
                            tokenList[tokenPos++].type = neqsym;
                            fprintf(fileCleanCode,"<>");
                            scanner = fgetc(fileCode);
                        }
                        else{
                            strcpy(tokenList[tokenPos].name, "<");
                            tokenList[tokenPos++].type = lessym;
                            fprintf(fileCleanCode,"<");
                        }
                        break;
                    case ':' :
                        scanner = fgetc(fileCode);
                        if (scanner == '='){ //matches ':='
                            strcpy(tokenList[tokenPos].name, ":=");
                            tokenList[tokenPos++].type = becomessym;
                            fprintf(fileCleanCode,":=");
                            scanner = fgetc(fileCode);
                        }
                        else
                            printError(18);
                        break;
                    default:
                        printError(18);
                }
                if(scanner == EOF)
                    break;
            }

            else if ((int)scanner <= 57 && (int)scanner >= 48){ //CASE: is a number
               numHolder[0] = '\0';
               numHolder[0] = scanner;
               count = 1;
               ignore_flag = 0;

                while((int)scanner <= 57 && (int)scanner >= 48){
                    if(count >= numMax)
                        printError(20);
                    scanner = fgetc(fileCode);
                    if(((int)scanner >= 58 && (int)scanner <= 62) || ((int)scanner >= 40 && (int)scanner <= 47) || ((int)scanner == 32) || ((int)scanner == 10) || ((int)scanner == 9) || ((int)scanner == 59)){//not a letter. must break!
                        ignore_flag = 1;
                    }
                    if(!ignore_flag)
                        numHolder[count++] = scanner;
                }
                numHolder[count] = '\0';
                strcpy(tokenList[tokenPos].name,numHolder);
                tokenList[tokenPos++].type = numbersym;
                fprintf(fileCleanCode,"%s", numHolder);

                if(!ignore_flag)
                    scanner = fgetc(fileCode);
            }

            else if ((int)scanner <= 122 && (int)scanner >= 97){ //CASE: is a letter (lowercase only)
                varHolder[0] = '\0';
                varHolder[0] = scanner;
                count = 1;
                ignore_flag = 0;


                while((((int)scanner <= 57 && (int)scanner >= 48) || ((int)scanner <= 122 && (int)scanner >= 97 )) && (int)scanner > 32){
                    if(count >= identMax)
                        printError(21);
                    if((int)scanner < 32)
                        break;
                    scanner = fgetc(fileCode);
                    if(((int)scanner >= 58 && (int)scanner <= 62) || ((int)scanner >= 40 && (int)scanner <= 47) || ((int)scanner == 32) || ((int)scanner == 10) || ((int)scanner == 9) || ((int)scanner == 59)){//not a letter. must break!
                        ignore_flag = 1;
                    }
                    if(!ignore_flag)
                        varHolder[count++] = scanner;
                }
                varHolder[count] = '\0';
                if((int)varHolder[count-1] < 32)
                    varHolder[count-1] = '\0';
                //figure out if you have a reserved name
                if (strcmp(varHolder, "begin") == 0){
                    strcpy(tokenList[tokenPos].name, "begin");
                    tokenList[tokenPos++].type = beginsym;
                }
                else if (strcmp(varHolder, "call") == 0){
                    strcpy(tokenList[tokenPos].name, "call");
                    tokenList[tokenPos++].type = callsym;
                }
                else if (strcmp(varHolder, "const") == 0){
                    strcpy(tokenList[tokenPos].name, "const");
                    tokenList[tokenPos++].type = constsym;
                }
                else if (strcmp(varHolder, "var") == 0){
                    strcpy(tokenList[tokenPos].name, "var");
                    tokenList[tokenPos++].type = varsym;
                }
                else if (strcmp(varHolder, "do") == 0){
                    strcpy(tokenList[tokenPos].name, "do");
                    tokenList[tokenPos++].type = dosym;
                }
                else if (strcmp(varHolder, "else") == 0){
                    strcpy(tokenList[tokenPos].name, "else");
                    tokenList[tokenPos++].type = elsesym;
                }
                else if (strcmp(varHolder, "end") == 0){
                    strcpy(tokenList[tokenPos].name, "end");
                    tokenList[tokenPos++].type = endsym;
                }
                else if (strcmp(varHolder, "if") == 0){
                    strcpy(tokenList[tokenPos].name, "if");
                    tokenList[tokenPos++].type = ifsym;
                }
                else if (strcmp(varHolder, "write") == 0){
                    strcpy(tokenList[tokenPos].name, "write");
                    tokenList[tokenPos++].type = writesym;
                }
                else if (strcmp(varHolder, "procedure") == 0){
                    strcpy(tokenList[tokenPos].name, "procedure");
                    tokenList[tokenPos++].type = procsym;
                }
                else if (strcmp(varHolder, "then") == 0){
                    strcpy(tokenList[tokenPos].name, "then");
                    tokenList[tokenPos++].type = thensym;
                }
                else if (strcmp(varHolder, "while") == 0){
                    strcpy(tokenList[tokenPos].name, "while");
                    tokenList[tokenPos++].type = whilesym;
                }
                else if (strcmp(varHolder, "read") == 0){
                    strcpy(tokenList[tokenPos].name, "read");
                    tokenList[tokenPos++].type = readsym;
                }
                else if (strcmp(varHolder, "odd") == 0){
                    strcpy(tokenList[tokenPos].name, "odd");
                    tokenList[tokenPos++].type = oddsym;
                }
                else{
                    strcpy(tokenList[tokenPos].name,varHolder);
                    tokenList[tokenPos++].type = identsym;
                    }
                fprintf(fileCleanCode,"%s", varHolder);

                if(!ignore_flag)
                    scanner = fgetc(fileCode);
            }
            else //might be invalid symbol
                scanner = EOF;
        }
    }
    fclose(fileCleanCode);
}

void printLexeme(int flag){
    fileLexTable = fopen(nameLexTable,"w");
    if(fileLexTable == NULL)
        printError(23);

    printTable();

    fclose(fileLexTable);

    fileLexTableList = fopen(nameLexTableList,"w");
    if(fileLexTableList == NULL)
        printError(23);

    printList(flag);

    fclose(fileLexTableList);
}

void printTable(){
    int i;

    fprintf(fileLexTable,"lexeme      token type\n");
    for(i=0; i<tokenPos; i++){
        if(tokenList[i].type == newlinesym){}
        else
            fprintf(fileLexTable,"%-11s %d\n", tokenList[i].name, tokenList[i].type);
    }
}

void printList(int flag){
    int i;
    if(flag)
        printf("\nLexeme List:\n");
    for(i=0; i<tokenPos; i++){
        if(tokenList[i].type == newlinesym){}
        else{
            fprintf(fileLexTableList,"%d ", tokenList[i].type);
            if(flag)
                printf("%d ", tokenList[i].type);
            if(tokenList[i].type == identsym || tokenList[i].type == numbersym){
                fprintf(fileLexTableList,"%s ", tokenList[i].name);
                if(flag)
                    printf("%s ", tokenList[i].name);
            }
        }
    }
    if(flag)
        printf("\n");
}
