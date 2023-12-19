#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <regex.h>

// Definition of REGEX Matches

#define FILEEXTENSIONMATCH "([A-Za-z0-9]+[.]s$)|([A-Za-z0-9]+[.]asm$)|([A-Za-z0-9]+[.]mips$)|([A-Za-z0-9]+[.]spim$)"
#define OPERATIONMATCH "(sub)|(add)|(lw)|(addi)|(j)|(sw)|(slt)|(or)|(and)|(nand)|(nor)|(beq)"
#define RTYPEMATCH "([a-z]+ [$][a-z0-9]+[,] [$][a-z0-9]+[,] [$][a-z0-9]+)"
#define JTYPEMATCH "([j] 0x[0-9]{7})"
#define ITYPEMATCH "([a-z]+ [$][a-z0-9]+[,] [$][a-z0-9]+[,] 0x[0-9]{4})"
#define MEMTYPEMATCH "([a-z]+ [$][a-z0-9]+[,] 0x[0-9]{4}[(][$][a-z0-9]+[)])"
#define IMMD16MATCH "0x[0-9]{4}"
#define JADDRMATCH "0x[0-9]{7}"


// Definition of instruction set constants
#define ADDFUNCT 32
#define ANDFUNCT 36
#define ORFUNCT 37
#define SUBFUNCT 34
#define SLTFUNCT 42
#define NANDFUNCT 38
#define NORFUNCT  39
#define BEQ 4
#define LW 35
#define SW 43 
#define JUMP 2
#define ADDI 8

// Definition of default sizes
#define OPCODESIZE 6
#define JUMPADDSIZE 26
#define REGSIZE 5
#define FUNCTSIZE 6
#define IMMDSIZE 16
#define LINESIZE 1000
#define MEMSIZE 256
#define HASHTABLECAPACITY 32

// Struct Definitions

typedef struct
Instruction
{

    u_int8_t opcode;
    u_int8_t funct;
    u_int8_t rs;
    u_int8_t rt;
    u_int8_t rd;
    u_int8_t shamt;
    u_int16_t immd16;
    int jumpAddress;
    int instruction;


} instruction_t;

typedef struct 
Match 
{
    u_int8_t status;
    char * pmatch;
    u_int8_t instType;

} match_t;

typedef struct
HashItem
{
    char * key;
    int value;

} regItem_t;

typedef struct 
HashTable
{
    regItem_t ** regBank;
    int size;
    int count;

} hashtable_t;

// Hash Table function signatures
unsigned long hash_function(char* str);
int searchKey(hashtable_t * table, char * key);
hashtable_t * initializeTable(hashtable_t * table);
regItem_t * create_item(char* key, unsigned int value);
void printRegBank(hashtable_t * table);


// Function signatures
char * toBinaryString(int decimal, int bitsNumber);
int toDecimal(char * binaryString);
int saveResult(int * memory, char * filename);
int regMatch(char * matchString, char * matchRegex, match_t * matched);
int lineLexicAnalyze(char * line, match_t * myMatch);
int setupFile(char * filename);
int readLine(FILE * fp, char * line);
int setFunct(instruction_t * inst,  match_t * match);
int setReg(instruction_t * inst, char * line, char * reg, hashtable_t * table);
char * regPosition(char * line);
int setOpcode(instruction_t * inst, match_t * match);
int setImmd16(instruction_t * inst, match_t * match);
int setJumpAddress(instruction_t * inst, match_t * match);

// To use compiler, ran it with args (file.s ramfilename)
int main(int argc, char ** argv){

    if(argc != 3){
        printf("Wrong arguments passed to main \n");
        return 1;
    }

    match_t * myMatch =  (match_t *) malloc(sizeof(match_t) * 1);

    if(regMatch(argv[1], FILEEXTENSIONMATCH, myMatch)){
        printf("No assembly file was passed as argument 1 \n");
        return 1;
    }

    int * memory = (int *) malloc(sizeof(int) * MEMSIZE);

    memset(memory, 0, sizeof(int) * MEMSIZE);

    setupFile(argv[2]);

    FILE * filePointer;

    filePointer = fopen(argv[1], "r");

    char * line = (char *) malloc(LINESIZE);

    memset(line, 0, LINESIZE);

    instruction_t * instruction = (instruction_t *) malloc(sizeof(instruction_t));

    hashtable_t * hashTable = (hashtable_t *) malloc(sizeof(hashtable_t));

    hashTable = initializeTable(hashTable);

    unsigned int linePos;

    char * lineError = (char *) malloc(LINESIZE);

    int counter = 0;

    int errorMatch;

    char * instructionC = (char *) malloc(32);

    while(!feof(filePointer)){
        readLine(filePointer, line);
        memset(instructionC, 0, 32);
        memset(instruction, 0, sizeof(instruction_t));
        
        if(lineLexicAnalyze(line, myMatch)){
            printf("Error in lexic analysis");
        }

        printf("instruction %d: %s\n",counter + 1, line);

        switch (myMatch->instType)
        {
        case 0: // R-TYPE
            instruction->opcode = 0x0;
            strncpy(lineError, line, 30);
            regMatch(line, OPERATIONMATCH, myMatch);
            if(setFunct(instruction, myMatch)){
                printf("error in setting funct in line %s", lineError);
            }
            line = regPosition(line);
            if(setReg(instruction, line, "rd", hashTable)){
                printf("error in setting rd register in line %s", lineError);
            }
            line = regPosition(line);
            if(setReg(instruction, line, "rs", hashTable)){
                printf("error in setting rs register in line %s", lineError);
            }
            line = regPosition(line);
            if(setReg(instruction, line, "rt", hashTable)){
                printf("error in setting rt register in line %s", lineError);
            }
            instruction->shamt = 0x0;

            strcat(instructionC, toBinaryString(instruction->opcode, OPCODESIZE));
            strcat(instructionC, toBinaryString(instruction->rs, REGSIZE));
            strcat(instructionC, toBinaryString(instruction->rt, REGSIZE));
            strcat(instructionC, toBinaryString(instruction->rd, REGSIZE));
            strcat(instructionC, toBinaryString(instruction->shamt, REGSIZE));
            strcat(instructionC, toBinaryString(instruction->funct, FUNCTSIZE));

            instruction->instruction = toDecimal(instructionC);

            printf("Instruction opcode: %d\n", instruction->opcode);
            printf("Instruction rs: %d\n", instruction->rs);
            printf("Instruction rt: %d\n", instruction->rt);
            printf("Instruction rd: %d\n", instruction->rd);
            printf("Instruction shamt: %d\n", instruction->shamt);
            printf("Instruction funct: %d\n\n", instruction->funct);

            memory[counter] = instruction->instruction;

            break;
        
        case 1: // I-TYPE
            strncpy(lineError, line, 30);
            regMatch(line, OPERATIONMATCH, myMatch);
            if(setOpcode(instruction, myMatch)){
                printf("error in setting opcode in line %s", lineError);
            }
            instruction->funct = 0x0;
            line = regPosition(line);
            if(setReg(instruction, line, "rs", hashTable)){
                printf("error in setting rs register in line %s", lineError);
            }
            line = regPosition(line);
            if(setReg(instruction, line, "rt", hashTable)){
                printf("error in setting rs register in line %s", lineError);
            }
            instruction->rd = 0x0;
            regMatch(line, IMMD16MATCH, myMatch);
            setImmd16(instruction, myMatch);

            strcat(instructionC, toBinaryString(instruction->opcode, OPCODESIZE));
            strcat(instructionC, toBinaryString(instruction->rs, REGSIZE));
            strcat(instructionC, toBinaryString(instruction->rt, REGSIZE));
            instruction->shamt = 0x0;
            strcat(instructionC, toBinaryString(instruction->immd16, IMMDSIZE));

            instruction->instruction = toDecimal(instructionC);

            printf("Instruction opcode: %d\n", instruction->opcode);
            printf("Instruction rs: %d\n", instruction->rs);
            printf("Instruction rt: %d\n", instruction->rt);
            printf("Instruction immd16: %d\n\n", instruction->immd16);

            memory[counter] = instruction->instruction;
            
            break;
        
        case 2: // J-TYPE
            strncpy(lineError, line, 30);
            regMatch(line, OPERATIONMATCH, myMatch);
            if(setOpcode(instruction, myMatch)){
                printf("error in setting opcode in line %s", lineError);
            }
            instruction->funct = 0x0;
            instruction->rs = 0x0;
            instruction->rt = 0x0;
            instruction->rd = 0x0;
            instruction->shamt = 0x0;
            instruction->immd16 = 0x0;
            regMatch(line, JADDRMATCH, myMatch);
            setJumpAddress(instruction, myMatch);

            strcat(instructionC, toBinaryString(instruction->opcode, OPCODESIZE));
            strcat(instructionC, toBinaryString(instruction->jumpAddress, JUMPADDSIZE));

            instruction->instruction = toDecimal(instructionC);

            printf("Instruction opcode: %d\n", instruction->opcode);
            printf("Instruction jump address: %d\n\n", instruction->jumpAddress);

            memory[counter] = instruction->instruction;
            break;
        
        case 3: // MEM-TYPE
            strncpy(lineError, line, 30);
            regMatch(line, OPERATIONMATCH, myMatch);
            if(setOpcode(instruction, myMatch)){
                printf("error in setting opcode in line %s", lineError);
            }
            instruction->funct = 0x0;
            line = regPosition(line);
            if(setReg(instruction, line, "rt", hashTable)){
                printf("error in setting rs register in line %s", lineError);
            }
            regMatch(line, IMMD16MATCH, myMatch);
            setImmd16(instruction, myMatch);
            line = regPosition(line);
            if(setReg(instruction, line, "rs", hashTable)){
                printf("error in setting rs register in line %s", lineError);
            }
            instruction->rd = 0x0;
            instruction->shamt = 0x0;
            instruction->funct = 0x0;
            strcat(instructionC, toBinaryString(instruction->opcode, OPCODESIZE));
            strcat(instructionC, toBinaryString(instruction->rs, REGSIZE));
            strcat(instructionC, toBinaryString(instruction->rt, REGSIZE));
            strcat(instructionC, toBinaryString(instruction->immd16, IMMDSIZE));
            
            instruction->instruction = toDecimal(instructionC);

            printf("Instruction opcode: %d\n", instruction->opcode);
            printf("Instruction rs: %d\n", instruction->rs);
            printf("Instruction rt: %d\n", instruction->rt);
            printf("Instruction immd16: %d\n\n", instruction->immd16);

            memory[counter] = instruction->instruction;

            break;
        
        default:
            printf("Error occurred in %s", line);
            break;
        }

        printf("instruction hexa: %08x\n", memory[counter]);
        printf("=================\n");
        printf("Aperte enter para continuar...");
        getchar();

        counter++;
    }

    saveResult(memory, argv[2]);

}

char * toBinaryString(int decimal, int bitsNumber){

    char * binaryOut = (char *) malloc(bitsNumber + 1);

    int remainder;

    for(int i = 1; i <= bitsNumber + 1; i++){

        remainder = decimal % 2;
        if(remainder == 0){
            binaryOut[bitsNumber - i] = '0';
        }
        else
        {
            binaryOut[bitsNumber - i] = '1';
        }
        decimal = decimal / 2;
    }
    binaryOut[bitsNumber] = '\0';

    return binaryOut;

}

int toDecimal(char * binaryString){
    int result = 0;
    for (int i = 0; binaryString[i] != '\0'; i++) {
        result <<= 1;  // Left shift by 1 bit
        if (binaryString[i] == '1') {
            result |= 1;  // Set the least significant bit
        }
    }
    return result;
    
}

int saveResult(int * memory, char * filename){

    FILE * filePointer;

    filePointer = fopen(filename, "a");

    if(filePointer == NULL){
        return 1;
    }

    for(unsigned int counter = 0; counter < MEMSIZE; counter++){
        if(counter % 8 == 0){
            fprintf(filePointer, "\n%04x : %08x", counter, memory[counter]);
            continue;
        }

        fprintf(filePointer, " %08x", memory[counter]);
    }
    
    return 0;
    
}

int setupFile(char * filename){
    FILE * filePointer;

    filePointer = fopen(filename, "w");

    if(filePointer == NULL){
        filePointer = fopen(filename, "w");
        fprintf(filePointer, "v3.0 raw");
        return 0;
    }

    fprintf(filePointer, "v3.0 raw");
    fclose(filePointer);
    return 0;
    
}

int regMatch(char * matchString, char * matchRegex, match_t * matched){
    
    regex_t * reg = (regex_t *) malloc(sizeof(regex_t));
    regmatch_t * pmatch = (regmatch_t *) malloc(sizeof(regmatch_t));

    char * result = (char *) malloc(20);
    memset(result, 0, 20);

    int len;

    if(regcomp(reg, matchRegex, REG_EXTENDED | REG_ICASE) != 0){
        matched->status = 1;
        matched->pmatch = (char *) NULL;
        matched->instType = 0;

        printf("Error in regex comp on %s", matchString);
        return 1;
    }

    if(regexec(reg, matchString, 1, pmatch, 0) != 0){
        matched->status = 1;
        matched->pmatch = (char *) NULL;
        matched->instType = 0;
        return 1;

    }

    matched->status = 0;

    len = pmatch[0].rm_eo - pmatch[0].rm_so;
    memcpy(result, matchString + pmatch[0].rm_so, len);

    matched->pmatch = result;

    matched->instType = 0;

    return 0;
    
}

int lineLexicAnalyze(char * line, match_t * myMatch){

    char rType[] = RTYPEMATCH; // counter == 0
    char iType[] = ITYPEMATCH; // counter == 1
    char jType[] = JTYPEMATCH; // counter == 2
    char memType[] = MEMTYPEMATCH; // counter == 3
    char * myCArray[] = {rType, iType, jType, memType};
    int counter = 0;

    do
    {
        
        if(!(regMatch(line, myCArray[counter], myMatch))){
            break;
        }
        counter++;
    }while(counter < 4);

    if(counter == 4){
        myMatch->status = 1;
        return 1;
    }

    myMatch->instType = counter;
    return 0;

}

int readLine(FILE * fp, char * line){

    fgets(line, LINESIZE, fp);

}

int setFunct(instruction_t * inst, match_t * match){
    if(strcmp(match->pmatch, "add") == 0){
        inst->funct = 0x20;
        return 0;
    }
    if(strcmp(match->pmatch, "sub") == 0){
        inst->funct = 0x22;
        return 0;
    }
    if(strcmp(match->pmatch, "slt") == 0){
        inst->funct = 0x2A;
        return 0;
    }
    if(strcmp(match->pmatch, "or") == 0){
        inst->funct = 0x25;
        return 0;
    }
    if(strcmp(match->pmatch, "nor") == 0){
        inst->funct = 0x27;
        return 0;
    }
    if(strcmp(match->pmatch, "nand") == 0){
        inst->funct = 0x26;
        return 0;
    }
    if(strcmp(match->pmatch, "and") == 0){
        inst->funct = 0x24;
        return 0;
    }

    printf("Set Funct error");
    exit(1);
}

int searchKey(hashtable_t * table, char * key){
    
    for(int i = 0; i < HASHTABLECAPACITY; i++){
        if(strcmp(table->regBank[i]->key, key) == 0){
            return table->regBank[i]->value;
        }
    }
}

hashtable_t * initializeTable(hashtable_t * table){

    char * myRegNames[] = { "ze", 
        "at", "v0", "v1", "a0", "a1", "a2", "a3", "t0", "t1", "t2", "t3", "t4", "t5", "t6", "t7", "s0", "s1", "s2", "s3",
        "s4", "s5", "s6", "s7", "t8", "t9", "k0" , "k1", "gp", "sp", "fp", "ra"
    };

    regItem_t ** regBank = (regItem_t **) malloc(sizeof(regItem_t *) *  32);

    for(unsigned int i = 0; i < 32; i++){
        regBank[i] = create_item(myRegNames[i], i);
    }

    table->regBank = regBank;
    table->size = HASHTABLECAPACITY;
    table->count = 0;

}

int setReg(instruction_t * inst, char * line, char * reg, hashtable_t * table){
    
    char * regName = (char *) malloc(3);
    memset(regName, 0, 3);

    if(strcmp("rs", reg) == 0){
        memcpy(regName, line, 2);

        inst->rs = searchKey(table, regName);
        return 0;
    }
    else if(strcmp("rt", reg) == 0){
        memcpy(regName, line, 2);

        inst->rt = searchKey(table, regName);
        return 0;
    }
    else if(strcmp("rd", reg) == 0){
        memcpy(regName, line, 2);

        inst->rd = searchKey(table, regName);
        return 0;
    }

    return 1;

}

char * regPosition(char * line){
    
    unsigned int counter = 0;

    while(1){
        if(line[counter] == '$'){
            counter = counter + 1;
            break;
        }
        counter++;
    }


    line = line + counter;

    return line;

}

regItem_t * create_item(char* key, unsigned int value)
{
    // Creates a pointer to a new HashTable item.
    regItem_t * item =  (regItem_t *) malloc(sizeof(regItem_t));
    item->key = (char *) malloc(strlen(key) + 1);
    item->value = value;
    strncpy(item->key, key, strlen(key) + 1);
    return item;
}

int setOpcode(instruction_t * inst, match_t * match){
    if(strcmp(match->pmatch, "beq") == 0){
        inst->opcode = 0x4;
        return 0;
    }
    else if(strcmp(match->pmatch, "lw") == 0){
        inst->opcode = 0x23;
        return 0;
    }
    else if(strcmp(match->pmatch, "sw") == 0){
        inst->opcode = 0x2B;
        return 0;
    }
    else if(strcmp(match->pmatch, "addi") == 0){
        inst->opcode =8;
        return 0;
    }
    else if(strcmp(match->pmatch, "j") == 0){
        inst->opcode = 0x2;
        return 0;
    }

    printf("Set Opcode error");
    exit(1);
}

int setImmd16(instruction_t * inst, match_t * match){

    char * immd16C = (char *) malloc(8);

    char * pmatch;

    pmatch = match->pmatch;

    pmatch = pmatch + 2;

    memset(immd16C, 0, 8);

    strncpy(immd16C, pmatch, 4);

    inst->immd16 = strtol(immd16C, NULL, 16);

    return 0;

}

int setJumpAddress(instruction_t * inst, match_t * match){

    char * JAddrC = (char *) malloc(10);

    char * pmatch;

    pmatch = match->pmatch;

    pmatch = pmatch + 2;

    memset(JAddrC, 0, 10);

    strncpy(JAddrC, pmatch, 7);

    inst->jumpAddress = strtol(JAddrC, NULL, 16);

    return 0;



}

void printRegBank(hashtable_t * table){

    for(int i = 0; i < 32; i++){
        printf("Reg name: %s\n", table->regBank[i]->key);
        printf("Reg value: %d\n",table->regBank[i]->value);
    }
}   