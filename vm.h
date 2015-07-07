//DONE!
#include "header.h"

//OpCode String Holders
const char* OPCODE_STRINGS[] = {"FCH", "LIT", "OPR", "LOD", "STO", "CAL", "INC", "JMP", "JPC", "SIO"};
const char* STACK_OPERATION_STRINGS[] = {"RET", "NEG", "ADD", "SUB", "MUL", "DIV", "ODD", "MOD", "EQL", "NEQ", "LSS", "LEQ", "GTR", "GEQ"};
const char* SIO_OPERATION_STRINGS[] = {"", "SOT", "SIN"};

enum OPCODE {FCH, LIT, OPR, LOD, STO, CAL, INC, JMP, JNC, SIO};
enum STACK_OPERATION {RET, NEG, ADD, SUB, MUL, DIV, ODD, MOD, EQL, NEQ, LSS, LEQ, GTR, GEQ};
enum SIO_OPERATION {SOT = 1, SIN};

//Struct
struct instruction{
	int OP;
	int L;
	int M;
};

//Register
int BP = 1;
int SP = 0;
int PC = 0;
struct instruction IR;

//Global Arrays
int stack[MAX_STACK_HEIGHT];
struct instruction code[MAX_CODE_LENGTH];

//Files
FILE *fileCode;
FILE *fileTrace;

//Other
int codeSize = 0;

//Functions
void loadFile();
void writeCode();
void runCode(int flag);
void fetch_cycle();
void execute_cycle();
void operate();
int base(int level, int b);
void printStack();

void vm(int flag){
    stack[1] = 0;
    stack[2] = 0;
    stack[3] = 0;

    fileCode = fopen(nameMCode,"r");
    if(fileCode == NULL)
        printError(ERROR_INVALID_FILE);

    loadFile();

    fileTrace = fopen(nameTrace,"w");
    if(fileTrace == NULL)
        printError(ERROR_INVALID_FILE);

    writeCode(flag);

    runCode(flag);
}

void loadFile(){
    int OP, L, M, i = 0;
    while(fscanf(fileCode,"%d",&OP) != EOF){
        if(i>MAX_CODE_LENGTH)
            printError(ERROR_CODE_OVERFLOW);

        fscanf(fileCode, "%d", &L);
        fscanf(fileCode, "%d", &M);

        code[i].OP = OP;
        code[i].L = L;
        code[i].M = M;

        i++;
    }
    codeSize = i;
}

void writeCode(int flag) {
	int i;
	fprintf(fileTrace,"Line  OP   L  M\n");
	for(i = 0; i < codeSize; i++){
		fprintf(fileTrace,"%4d  %s  %d  %2d\n", i, OPCODE_STRINGS[code[i].OP], code[i].L, code[i].M);
	}
	if(flag)
        printf("\nStack trace:\n                   pc  bp  sp  stack\nInitial values     %2d   %d  %2d\n", PC, BP, SP);
	fprintf(fileTrace,"\n\n");
	fprintf(fileTrace,"                   pc  bp  sp  stack\nInitial values     %2d   %d  %2d\n", PC, BP, SP);
}

void runCode(int flag){
    while (BP > 0){
        if (PC < codeSize){
            if(flag)
                printf("%4d  %s  %d  %2d ", PC, OPCODE_STRINGS[code[PC].OP], code[PC].L, code[PC].M);
            fprintf(fileTrace,"%4d  %s  %d  %2d ", PC, OPCODE_STRINGS[code[PC].OP], code[PC].L, code[PC].M);

            fetch_cycle();
            execute_cycle();

            if(flag)
                printf("  %2d   %d  %2d  ", PC, BP, SP);
            fprintf(fileTrace,"  %2d   %d  %2d  ", PC, BP, SP);
            printStack(flag);
            if(flag)
                printf("\n");
            fprintf(fileTrace,"\n");
        }
        else
            exit(-99);
    }
}

void fetch_cycle(){
	IR = code[PC];
	PC++;
}

void execute_cycle(){
	switch (IR.OP) {
		case FCH:	//Fetch, not really used here
			break;
		case LIT:	//push literal onto stack
			SP++;
			stack[SP] = IR.M;
			break;
		case OPR:	//Operate on stack
			operate();
			break;
		case LOD:	//push onto stack
			SP++;
			stack[SP] = stack[base(IR.L, BP) + IR.M];
			break;
		case STO:	//pop the value off of the stack and store at offset IR.M
			stack[base(IR.L, BP) + IR.M] = stack[SP];
			SP--;
			break;
		case CAL:	//call proced. at IR.M
		    stack[SP + 1] = 0; // return value (FV)
            stack[SP + 2] = base(IR.L, BP); // static link (SL)
            stack[SP + 3] = BP; // dynamic link (DL)
            stack[SP + 4] = PC; // return address (RA)
            BP = SP + 1;
            PC = IR.M;
			break;
		case INC:	//Increment stack pointer (sp)
			SP += IR.M;
			break;
		case JMP:	//Jump to the instruction at IR.M
			PC = IR.M;
			break;
		case JNC:	//Jump to the instruction at IR.M if == 0
			if(stack[SP] == 0){
				PC = IR.M;
				SP--;
			}
			break;
		case SIO:	//perform standard IO op, depending on the instruction register
			if(IR.M == 0){
                printf(" %d ", stack[SP]);
                SP--;
			}
            else if(IR.M == 1){
                //read(stack[sp]); //nothing for now
                SP++;
            }
            else if(IR.M == 2){
                printError(HALT);
            }
			break;
		case 10:
		    break;	//nothing for now
		default:
			exit(-99);
    }
}

void operate(){
	switch (IR.M){
		case RET:
			SP = BP - 1;
			PC = stack[SP + 4];
			BP = stack[SP + 3];
			break;
		case NEG:
			stack[SP] *= -1;
			break;
		case ADD:
			SP--;
			stack[SP] = stack[SP] + stack[SP + 1];
			break;
		case SUB:
			SP--;
			stack[SP] = stack[SP] - stack[SP + 1];
			break;
		case MUL:
			SP--;
			stack[SP] = stack[SP] * stack[SP + 1];
			break;
		case DIV:
			SP--;
			stack[SP] = stack[SP] / stack[SP + 1];
			break;
		case ODD:
			stack[SP] %= 2;
			break;
		case MOD:
			SP--;
			stack[SP] %= stack[SP + 1];
			break;
		case EQL:
			SP--;
			stack[SP] = stack[SP] == stack[SP+1];
			break;
		case NEQ:
			SP--;
			stack[SP] = stack[SP] != stack[SP + 1];
			break;
		case LSS:
			SP--;
			stack[SP] = stack[SP] < stack[SP + 1];
			break;
		case LEQ:
			SP--;
			stack[SP] = stack[SP] <= stack[SP + 1];
			break;
		case GTR:
			SP--;
			stack[SP] = stack[SP] > stack[SP + 1];
			break;
		case GEQ:
			SP--;
			stack[SP] = stack[SP] >= stack[SP + 1];
			break;
		default:
			printError(27);
	}
}

int base (int level, int base){
	while(level > 0){
		base = stack[base + 2];
		level--;
	}
	return base;
}

void printStack(int flag){
	int this_BP = BP;
	int num_BPs = 1;
	int BPs[MAX_LEXI_LEVELS];
	int i = 1;
	BPs[0] = 1;

	while (this_BP > 1){
        BPs[i++] = this_BP;//Set to pos i in BP array
        this_BP = stack[this_BP + 2];//Advance to previous BP number
	}
    num_BPs = i-1;

	for(i = 1; i <= SP; i++) {
		if (i == BPs[num_BPs] && i != 1) {
            if(flag)
                printf("| ");
			fprintf(fileTrace,"| ");
			num_BPs--;
		}
		if(flag)
            printf("%d ", stack[i]);
		fprintf(fileTrace,"%d ", stack[i]);
	}
}

void printError(int n){
    switch(n){
        case 1:
            printf("\nAn error has occurred: Use ""="" instead of "":="".\n");
            break;
        case 2:
            printf("\nAn error has occurred: ""="" must be followed by a number.\n");
            break;
        case 3:
            printf("\nAn error has occurred: Identifier must be followed by ""="".\n");
            break;
        case 4:
            printf("\nAn error has occurred: ""const"",""var"",""procedure"" must be followed by ident.\n");
            break;
        case 5:
            printf("\nAn error has occurred: "";"" or "","" missing.\n");
            break;
        case 6: //UNUSED
            printf("\nAn error has occurred: Incorrect sym after ""procedure"" declaration.\n");
            break;
        case 7:
            printf("\nAn error has occurred: Statement expected.\n");
            break;
        case 8: //UNUSED
            printf("\nAn error has occurred: Incorrect sym after ""procedure"" declaration.\n");
            break;
        case 9:
            printf("\nAn error has occurred: ""."" expected.\n");
            break;
        case 10:
            printf("\nAn error has occurred: "";"" between statements expected.\n");
            break;
        case 11: //TODO
            printf("\nAn error has occurred: Undeclared identifier.\n");
            break;
        case 12: // TODO
            printf("\nAn error has occurred: Assignment to ""const"" or ""procedure"".\n");
            break;
        case 13: //TODO
            printf("\nAn error has occurred: Assignment op expected.\n");
            break;
        case 14: //UNUSED
            printf("\nAn error has occurred: identifier after ""call"" expected.\n");
            break;
        case 15: //TODO
            printf("\nAn error has occurred: ""const"" or ""var"" call is unused.\n");
            break;
        case 16:
            printf("\nAn error has occurred: ""then"" expected.\n");
            break;
        case 17:
            printf("\nAn error has occurred: ""}"" expected.\n");
            break;
        case 18:
            printf("\nAn error has occurred: ""do"" expected.\n");
            break;
            //TODO: 19-rest
        case 31:
            printf("\nOut of Memory. Exiting . . .\n");
            break;
        case 32:
            printf("\nHalt has occurred.\n");
            break;
        default:
            printf("\nAn error has occurred: .\n");
        }
    exit(n);
}