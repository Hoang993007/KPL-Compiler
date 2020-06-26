/* 
 * @copyright (c) 2008, Hedspi, Hanoi University of Technology
 * @author Huu-Duc Nguyen
 * @version 1.0
 */

#include <stdio.h>
#include "reader.h"
#include "codegen.h"  

#define CODE_SIZE 10000
extern SymTab* symtab;

extern Object* readiFunction;
extern Object* readcFunction;
extern Object* writeiProcedure;
extern Object* writecProcedure;
extern Object* writelnProcedure;

CodeBlock* codeBlock;


int test=1;

//level của scope hiện tại luôn tính bằng 0.
//Level của 1 scope không cố định
//càng ra ngoài (càng ra scope rộng hơn) thì level càng tăng
int computeNestedLevel(Scope* scope) {
  int level = 0;
  Scope* tmp = symtab->currentScope;
  while (tmp != scope) {
    tmp = tmp->outer;
    level ++;
  }
  return level;
}

//Sinh ra địa chỉ cho biến
void genVariableAddress(Object* var) {
  int level = computeNestedLevel(VARIABLE_SCOPE(var));
  int offset = VARIABLE_OFFSET(var);

  if(test==1){
    printf("genVariableAddress---------------------\n");
    printf("Object name: %s\n", var->name);
    printf("Level: %d\n", level);
    printf("Offset: %d\n", offset);
    printf("------------------------\n");
  }

  genLA(level, offset);
}

//sinh ra địa chỉ cho giá trị của biến
void genVariableValue(Object* var) {
  int level = computeNestedLevel(VARIABLE_SCOPE(var));
  int offset = VARIABLE_OFFSET(var);

  if(test==1){
    printf("genVariableValue---------------------\n");
    printf("Object name: %s\n", var->name);
    printf("Level: %d\n", level);
    printf("Offset: %d\n", offset);
    printf("------------------------\n");
  }

  genLV(level, offset);
}

//sinh ra địa chỉ cho đối số
void genParameterAddress(Object* param) {
  int level = computeNestedLevel(PARAMETER_SCOPE(param));
  int offset = PARAMETER_OFFSET(param);

  if(test==1){
    printf("genParameterAddress---------------------\n");
    printf("Object name: %s\n", param->name);
    printf("Level: %d\n", level);
    printf("Offset: %d\n", offset);
    printf("------------------------\n");
  }

  genLA(level, offset);
}

//sinh ra địa chỉ cho giá trị của đối số
void genParameterValue(Object* param) {
  int level = computeNestedLevel(PARAMETER_SCOPE(param));
  int offset = PARAMETER_OFFSET(param);

  if(test==1){
    printf("genParameterValue---------------------\n");
    printf("Object name: %s\n", param->name);
    printf("Level: %d\n", level);
    printf("Offset: %d\n", offset);
    printf("------------------------\n");
  }

  genLV(level, offset);
}

void genReturnValueAddress(Object* func) {
  int level = computeNestedLevel(FUNCTION_SCOPE(func));
  int offset = RETURN_VALUE_OFFSET;

  if(test==1){
    printf("genReturnValueAddress---------------------\n");
    printf("Object name: %s\n", func->name);
    printf("Level: %d\n", level);
    printf("Offset: %d\n", offset);
    printf("------------------------\n");
  }

  genLA(level, offset);
}

void genReturnValueValue(Object* func) {
  int level = computeNestedLevel(FUNCTION_SCOPE(func));
  int offset = RETURN_VALUE_OFFSET;

  if(test==1){
    printf("genReturnValueValue---------------------\n");
    printf("Object name: %s\n", func->name);
    printf("Level: %d\n", level);
    printf("Offset: %d\n", offset);
    printf("------------------------\n");
  }

  genLV(level, offset);
}

void genPredefinedProcedureCall(Object* proc) {
  if (proc == writeiProcedure)
    genWRI();
  else if (proc == writecProcedure)
    genWRC();
  else if (proc == writelnProcedure)
    genWLN();
}

void genProcedureCall(Object* proc) {
  int level = computeNestedLevel(proc->procAttrs->scope->outer);

  if(test==1){
    printf("genProcedureCall---------------------\n");
    printf("Object name: %s\n", proc->name);
    printf("Level: %d\n", level);
    printf("------------------------\n");
  }

  genCALL(level, proc->procAttrs->codeAddress);
}

void genPredefinedFunctionCall(Object* func) {
  if (func == readiFunction)
    genRI();
  else if (func == readcFunction)
    genRC();
}

void genFunctionCall(Object* func) {
  int level = computeNestedLevel(func->funcAttrs->scope->outer);

  if(test==1){
    printf("genFunctionCall---------------------\n");
    printf("Object name: %s\n", func->name);
    printf("Level: %d\n", level);
    printf("------------------------\n");
  }

  genCALL(level, func->funcAttrs->codeAddress);
}

void genLA(int level, int offset) {
  emitLA(codeBlock, level, offset);
}

void genLV(int level, int offset) {
  emitLV(codeBlock, level, offset);
}

void genLC(WORD constant) {
  emitLC(codeBlock, constant);
}

void genLI(void) {
  emitLI(codeBlock);
}

void genINT(int delta) {
  emitINT(codeBlock,delta);
}

void genDCT(int delta) {
  emitDCT(codeBlock,delta);
}

Instruction* genJ(CodeAddress label) {
  Instruction* inst = codeBlock->code + codeBlock->codeSize;
  emitJ(codeBlock,label);
  return inst;
}

Instruction* genFJ(CodeAddress label) {
  Instruction* inst = codeBlock->code + codeBlock->codeSize;
  emitFJ(codeBlock, label);
  return inst;
}

void genHL(void) {
  emitHL(codeBlock);
}

void genST(void) {
  emitST(codeBlock);
}

void genCALL(int level, CodeAddress label) {
  emitCALL(codeBlock, level, label);
}

void genEP(void) {
  emitEP(codeBlock);
}

void genEF(void) {
  emitEF(codeBlock);
}

void genRC(void) {
  emitRC(codeBlock);
}

void genRI(void) {
  emitRI(codeBlock);
}

void genWRC(void) {
  emitWRC(codeBlock);
}

void genWRI(void) {
  emitWRI(codeBlock);
}

void genWLN(void) {
  emitWLN(codeBlock);
}

void genAD(void) {
  emitAD(codeBlock);
}

void genSB(void) {
  emitSB(codeBlock);
}

void genML(void) {
  emitML(codeBlock);
}

void genDV(void) {
  emitDV(codeBlock);
}

void genNEG(void) {
  emitNEG(codeBlock);
}

void genCV(void) {
  emitCV(codeBlock);
}

void genEQ(void) {
  emitEQ(codeBlock);
}

void genNE(void) {
  emitNE(codeBlock);
}

void genGT(void) {
  emitGT(codeBlock);
}

void genGE(void) {
  emitGE(codeBlock);
}

void genLT(void) {
  emitLT(codeBlock);
}

void genLE(void) {
  emitLE(codeBlock);
}

void updateJ(Instruction* jmp, CodeAddress label) {
  jmp->q = label;
}

void updateFJ(Instruction* jmp, CodeAddress label) {
  jmp->q = label;
}

CodeAddress getCurrentCodeAddress(void) {
  return codeBlock->codeSize;
}

int isPredefinedFunction(Object* func) {
  return ((func == readiFunction) || (func == readcFunction));
}

int isPredefinedProcedure(Object* proc) {
  return ((proc == writeiProcedure) || (proc == writecProcedure) || (proc == writelnProcedure));
}

void initCodeBuffer(void) {
  codeBlock = createCodeBlock(CODE_SIZE);
}

void printCodeBuffer(void) {
  printCodeBlock(codeBlock);
}

void cleanCodeBuffer(void) {
  freeCodeBlock(codeBlock);
}

int serialize(char* fileName) {
  FILE* f;

  f = fopen(fileName, "wb");
  if (f == NULL) return IO_ERROR;
  saveCode(codeBlock, f);
  fclose(f);
  return IO_SUCCESS;
}
