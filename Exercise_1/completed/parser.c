/* 
 * @copyright (c) 2008, Hedspi, Hanoi University of Technology
 * @author Huu-Duc Nguyen
 * @version 1.0
 */
#include <stdio.h>
#include <stdlib.h>

#include "reader.h"
#include "scanner.h"
#include "parser.h"
#include "semantics.h"
#include "error.h"
#include "debug.h"
#include "codegen.h"

int selfCheck=1;

Token *currentToken;
Token *lookAhead;

extern Type* intType;
extern Type* charType;
extern SymTab* symtab;

void scan(void) {
  Token* tmp = currentToken;
  currentToken = lookAhead;
  lookAhead = getValidToken();
  free(tmp);
}

void eat(TokenType tokenType) {
  if (lookAhead->tokenType == tokenType) {
    //    printToken(lookAhead);
    scan();
  } else missingToken(tokenType, lookAhead->lineNo, lookAhead->colNo);
}

void compileProgram(void) {

if(selfCheck==1)
printf("compileProgram \n");

  Object* program;

  eat(KW_PROGRAM);
  eat(TK_IDENT);

  program = createProgramObject(currentToken->string);
  program->progAttrs->codeAddress = getCurrentCodeAddress();
  enterBlock(program->progAttrs->scope);

  eat(SB_SEMICOLON);

  compileBlock();
  eat(SB_PERIOD);

if(selfCheck==1)
printf("genHL \n");
  genHL();

  exitBlock();
}

void compileConstDecls(void) {

if(selfCheck==1)
printf("compileConstDecls \n");

  Object* constObj;
  ConstantValue* constValue;

  if (lookAhead->tokenType == KW_CONST) {
    eat(KW_CONST);
    do {
      eat(TK_IDENT);
      checkFreshIdent(currentToken->string);
      constObj = createConstantObject(currentToken->string);
      declareObject(constObj);
      
      eat(SB_EQ);
      constValue = compileConstant();
      constObj->constAttrs->value = constValue;
      
      eat(SB_SEMICOLON);
    } while (lookAhead->tokenType == TK_IDENT);
  }
}

void compileTypeDecls(void) {

if(selfCheck==1)
printf("compileTypeDecls \n");

  Object* typeObj;
  Type* actualType;

  if (lookAhead->tokenType == KW_TYPE) {
    eat(KW_TYPE);
    do {
      eat(TK_IDENT);
      
      checkFreshIdent(currentToken->string);
      typeObj = createTypeObject(currentToken->string);
      declareObject(typeObj);
      
      eat(SB_EQ);
      actualType = compileType();
      typeObj->typeAttrs->actualType = actualType;
      
      eat(SB_SEMICOLON);
    } while (lookAhead->tokenType == TK_IDENT);
  } 
}

void compileVarDecls(void) {

if(selfCheck==1)
printf("compileVarDecls \n");

  Object* varObj;
  Type* varType;

  if (lookAhead->tokenType == KW_VAR) {
    eat(KW_VAR);
    do {
      eat(TK_IDENT);
      checkFreshIdent(currentToken->string);
      varObj = createVariableObject(currentToken->string);
      eat(SB_COLON);
      varType = compileType();
      varObj->varAttrs->type = varType;
      declareObject(varObj);      
      eat(SB_SEMICOLON);
    } while (lookAhead->tokenType == TK_IDENT);
  } 
}

void compileBlock(void) {

if(selfCheck==1)
printf("compileBlock \n");

  Instruction* jmp;
  
if(selfCheck==1)
printf("genJ \n");
  jmp = genJ(DC_VALUE);

  compileConstDecls();
  compileTypeDecls();
  compileVarDecls();
  compileSubDecls();

  updateJ(jmp,getCurrentCodeAddress());
if(selfCheck==1)
printf("genINT \n");
  genINT(symtab->currentScope->frameSize);

  eat(KW_BEGIN);
  compileStatements();
  eat(KW_END);
}

void compileSubDecls(void) {

if(selfCheck==1)
printf("compileSubDecls \n");

  while ((lookAhead->tokenType == KW_FUNCTION) || (lookAhead->tokenType == KW_PROCEDURE)) {
    if (lookAhead->tokenType == KW_FUNCTION)
      compileFuncDecl();
    else compileProcDecl();
  }
}

void compileFuncDecl(void) {

if(selfCheck==1)
printf("compileFuncDecl \n");

  Object* funcObj;
  Type* returnType;

  eat(KW_FUNCTION);
  eat(TK_IDENT);

  checkFreshIdent(currentToken->string);
  funcObj = createFunctionObject(currentToken->string);
  funcObj->funcAttrs->codeAddress = getCurrentCodeAddress();
  declareObject(funcObj);

  enterBlock(funcObj->funcAttrs->scope);
  
  compileParams();

  eat(SB_COLON);
  returnType = compileBasicType();
  funcObj->funcAttrs->returnType = returnType;

  eat(SB_SEMICOLON);

  compileBlock();

if(selfCheck==1)
printf("genEF \n");
  genEF();
  eat(SB_SEMICOLON);

  exitBlock();
}

void compileProcDecl(void) {

if(selfCheck==1)
printf("compileProcDecl \n");

  Object* procObj;

  eat(KW_PROCEDURE);
  eat(TK_IDENT);

  checkFreshIdent(currentToken->string);
  procObj = createProcedureObject(currentToken->string);
  procObj->procAttrs->codeAddress = getCurrentCodeAddress();
  declareObject(procObj);

  enterBlock(procObj->procAttrs->scope);

  compileParams();

  eat(SB_SEMICOLON);
  compileBlock();

if(selfCheck==1)
printf("genEP \n");
  genEP();
  eat(SB_SEMICOLON);

  exitBlock();
}

ConstantValue* compileUnsignedConstant(void) {

if(selfCheck==1)
printf("compileUnsignedConstant \n");

  ConstantValue* constValue;
  Object* obj;

  switch (lookAhead->tokenType) {
  case TK_NUMBER:
    eat(TK_NUMBER);
    constValue = makeIntConstant(currentToken->value);
    break;
  case TK_IDENT:
    eat(TK_IDENT);

    obj = checkDeclaredConstant(currentToken->string);
    constValue = duplicateConstantValue(obj->constAttrs->value);

    break;
  case TK_CHAR:
    eat(TK_CHAR);
    constValue = makeCharConstant(currentToken->string[0]);
    break;
  default:
    error(ERR_INVALID_CONSTANT, lookAhead->lineNo, lookAhead->colNo);
    break;
  }
  return constValue;
}

ConstantValue* compileConstant(void) {

if(selfCheck==1)
printf("compileConstant \n");

  ConstantValue* constValue;

  switch (lookAhead->tokenType) {
  case SB_PLUS:
    eat(SB_PLUS);
    constValue = compileConstant2();
    break;
  case SB_MINUS:
    eat(SB_MINUS);
    constValue = compileConstant2();
    constValue->intValue = - constValue->intValue;
    break;
  case TK_CHAR:
    eat(TK_CHAR);
    constValue = makeCharConstant(currentToken->string[0]);
    break;
  default:
    constValue = compileConstant2();
    break;
  }
  return constValue;
}

ConstantValue* compileConstant2(void) {

if(selfCheck==1)
printf("compileConstant2 \n");

  ConstantValue* constValue;
  Object* obj;

  switch (lookAhead->tokenType) {
  case TK_NUMBER:
    eat(TK_NUMBER);
    constValue = makeIntConstant(currentToken->value);
    break;
  case TK_IDENT:
    eat(TK_IDENT);
    obj = checkDeclaredConstant(currentToken->string);
    if (obj->constAttrs->value->type == TP_INT)
      constValue = duplicateConstantValue(obj->constAttrs->value);
    else
      error(ERR_UNDECLARED_INT_CONSTANT,currentToken->lineNo, currentToken->colNo);
    break;
  default:
    error(ERR_INVALID_CONSTANT, lookAhead->lineNo, lookAhead->colNo);
    break;
  }
  return constValue;
}

Type* compileType(void) {

if(selfCheck==1)
printf("compileType \n");

  Type* type;
  Type* elementType;
  int arraySize;
  Object* obj;

  switch (lookAhead->tokenType) {
  case KW_INTEGER: 
    eat(KW_INTEGER);
    type =  makeIntType();
    break;
  case KW_CHAR: 
    eat(KW_CHAR); 
    type = makeCharType();
    break;
  case KW_ARRAY:
    eat(KW_ARRAY);
    eat(SB_LSEL);
    eat(TK_NUMBER);

    arraySize = currentToken->value;

    eat(SB_RSEL);
    eat(KW_OF);
    elementType = compileType();
    type = makeArrayType(arraySize, elementType);
    break;
  case TK_IDENT:
    eat(TK_IDENT);
    obj = checkDeclaredType(currentToken->string);
    type = duplicateType(obj->typeAttrs->actualType);
    break;
  default:
    error(ERR_INVALID_TYPE, lookAhead->lineNo, lookAhead->colNo);
    break;
  }
  return type;
}

Type* compileBasicType(void) {

if(selfCheck==1)
printf("compileBasicType \n");

  Type* type;

  switch (lookAhead->tokenType) {
  case KW_INTEGER: 
    eat(KW_INTEGER); 
    type = makeIntType();
    break;
  case KW_CHAR: 
    eat(KW_CHAR); 
    type = makeCharType();
    break;
  default:
    error(ERR_INVALID_BASICTYPE, lookAhead->lineNo, lookAhead->colNo);
    break;
  }
  return type;
}

void compileParams(void) {

if(selfCheck==1)
printf("compileParams \n");

  if (lookAhead->tokenType == SB_LPAR) {
    eat(SB_LPAR);
    compileParam();
    while (lookAhead->tokenType == SB_SEMICOLON) {
      eat(SB_SEMICOLON);
      compileParam();
    }
    eat(SB_RPAR);
  }
}

void compileParam(void) {

if(selfCheck==1)
printf("compileParam \n");

  Object* param;
  Type* type;
  enum ParamKind paramKind = PARAM_VALUE;

  if (lookAhead->tokenType == KW_VAR) {
    paramKind = PARAM_REFERENCE;
    eat(KW_VAR);
  }

  eat(TK_IDENT);
  checkFreshIdent(currentToken->string);
  param = createParameterObject(currentToken->string, paramKind);
  eat(SB_COLON);
  type = compileBasicType();
  param->paramAttrs->type = type;
  declareObject(param);
}

void compileStatements(void) {

if(selfCheck==1)
printf("compileStatements \n");

  compileStatement();
  while (lookAhead->tokenType == SB_SEMICOLON) {
    eat(SB_SEMICOLON);
    compileStatement();
  }
}

void compileStatement(void) {

if(selfCheck==1)
printf("compileStatement \n");

  switch (lookAhead->tokenType) {
  case TK_IDENT:
    compileAssignSt();
    break;
  case KW_CALL:
    compileCallSt();
    break;
  case KW_BEGIN:
    compileGroupSt();
    break;
  case KW_IF:
    compileIfSt();
    break;
  case KW_WHILE:
    compileWhileSt();
    break;
  case KW_FOR:
    compileForSt();
    break;
    // EmptySt needs to check FOLLOW tokens
  case SB_SEMICOLON:
  case KW_END:
  case KW_ELSE:
    break;
    // Error occurs
  default:
    error(ERR_INVALID_STATEMENT, lookAhead->lineNo, lookAhead->colNo);
    break;
  }
}

Type* compileLValue(void) {

if(selfCheck==1)
printf("compileLValue \n");

  Object* var;
  Type* varType;

  eat(TK_IDENT);
  
  var = checkDeclaredLValueIdent(currentToken->string);

  switch (var->kind) {
  case OBJ_VARIABLE:
if(selfCheck==1)
printf("genVariableAddress \n");
    genVariableAddress(var);

    if (var->varAttrs->type->typeClass == TP_ARRAY) {
      varType = compileIndexes(var->varAttrs->type);
    }
    else
      varType = var->varAttrs->type;
    break;
  case OBJ_PARAMETER:
    if (var->paramAttrs->kind == PARAM_VALUE){
if(selfCheck==1)
printf("genParameterAddress \n");
      genParameterAddress(var);}
    else {
if(selfCheck==1)
printf("genParameterValue \n");
genParameterValue(var);}

    varType = var->paramAttrs->type;
    break;
  case OBJ_FUNCTION:
if(selfCheck==1)
printf("genReturnValueAddress \n");
    genReturnValueAddress(var);
    varType = var->funcAttrs->returnType;
    break;
  default: 
    error(ERR_INVALID_LVALUE,currentToken->lineNo, currentToken->colNo);
  }

  return varType;
}

void compileAssignSt(void) {

if(selfCheck==1)
printf("compileAssignSt \n");

  Type* varType;
  Type* expType;

  varType = compileLValue();
  
  eat(SB_ASSIGN);
  expType = compileExpression();
  checkTypeEquality(varType, expType);

if(selfCheck==1)
printf("genST \n");
  genST();
}

void compileCallSt(void) {

if(selfCheck==1)
printf("compileCallSt \n");

  Object* proc;

  eat(KW_CALL);
  eat(TK_IDENT);

  proc = checkDeclaredProcedure(currentToken->string);

  
  if (isPredefinedProcedure(proc)) {
    compileArguments(proc->procAttrs->paramList);
if(selfCheck==1)
printf("genPredefinedProcedureCall \n");
    genPredefinedProcedureCall(proc);
  } else {
if(selfCheck==1)
printf("genINT \n");
    genINT(RESERVED_WORDS);
    compileArguments(proc->procAttrs->paramList);
if(selfCheck==1)
printf("genDCT \n");
    genDCT( RESERVED_WORDS + proc->procAttrs->paramCount);
if(selfCheck==1)
printf("genProcedureCall \n");
    genProcedureCall(proc);
  }
}

void compileGroupSt(void) {

if(selfCheck==1)
printf("compileGroupSt \n");

  eat(KW_BEGIN);
  compileStatements();
  eat(KW_END);
}

void compileIfSt(void) {

if(selfCheck==1)
printf("compileIfSt \n");

  Instruction* fjInstruction;
  Instruction* jInstruction;

  eat(KW_IF);
  compileCondition();
  eat(KW_THEN);

if(selfCheck==1)
printf("genFJ \n");
  fjInstruction = genFJ(DC_VALUE);
  compileStatement();
  if (lookAhead->tokenType == KW_ELSE) {
if(selfCheck==1)
printf("genJ \n");
    jInstruction = genJ(DC_VALUE);
    updateFJ(fjInstruction, getCurrentCodeAddress());
    eat(KW_ELSE);
    compileStatement();
    updateJ(jInstruction, getCurrentCodeAddress());
  } else {
    updateFJ(fjInstruction, getCurrentCodeAddress());
  }
}

void compileWhileSt(void) {

if(selfCheck==1)
printf("compileWhileSt \n");

  CodeAddress beginWhile;
  Instruction* fjInstruction;

  beginWhile = getCurrentCodeAddress();
  eat(KW_WHILE);
  compileCondition();
if(selfCheck==1)
printf("genFJ \n");
  fjInstruction = genFJ(DC_VALUE);
  eat(KW_DO);
  compileStatement();
if(selfCheck==1)
printf("genJ \n");
  genJ(beginWhile);
  updateFJ(fjInstruction, getCurrentCodeAddress());
}

void compileForSt(void) {

if(selfCheck==1)
printf("compileForSt \n");

  CodeAddress beginLoop;
  Instruction* fjInstruction;
  Type* varType;
  Type *type;

  eat(KW_FOR);

  varType = compileLValue();
  eat(SB_ASSIGN);
if(selfCheck==1)
printf("genCV \n");
  genCV();
  type = compileExpression();
  checkTypeEquality(varType, type);
if(selfCheck==1)
printf("genST \n");
  genST();
if(selfCheck==1)
printf("genCV \n");
  genCV();
if(selfCheck==1)
printf("genLI \n");
  genLI();
  beginLoop = getCurrentCodeAddress();
  eat(KW_TO);

  type = compileExpression();
  checkTypeEquality(varType, type);
if(selfCheck==1)
printf("genLE \n");
  genLE();
if(selfCheck==1)
printf("genFJ \n");
  fjInstruction = genFJ(DC_VALUE);

  eat(KW_DO);
  compileStatement();

if(selfCheck==1)
printf("genCV \n");
  genCV();
if(selfCheck==1)
printf("genCV \n");  
  genCV();
if(selfCheck==1)
printf("genLI \n");
  genLI();
if(selfCheck==1)
printf("genLC \n");
  genLC(1);
if(selfCheck==1)
printf("genAD \n");
  genAD();
if(selfCheck==1)
printf("genST \n");
  genST();

if(selfCheck==1)
printf("genCV \n");
  genCV();
if(selfCheck==1)
printf("genLI \n");
  genLI();

if(selfCheck==1)
printf("genJ \n");
  genJ(beginLoop);
  updateFJ(fjInstruction, getCurrentCodeAddress());
if(selfCheck==1)
printf("genDCT \n");
  genDCT(1);

}

void compileArgument(Object* param) {

if(selfCheck==1)
printf("compileArgument \n");

  Type* type;

  if (param->paramAttrs->kind == PARAM_VALUE) {
    type = compileExpression();
    checkTypeEquality(type, param->paramAttrs->type);
  } else {
    type = compileLValue();
    checkTypeEquality(type, param->paramAttrs->type);
  }
}

void compileArguments(ObjectNode* paramList) {

if(selfCheck==1)
printf("compileArguments \n");

  ObjectNode* node = paramList;

  switch (lookAhead->tokenType) {
  case SB_LPAR:
    eat(SB_LPAR);
    if (node == NULL)
      error(ERR_PARAMETERS_ARGUMENTS_INCONSISTENCY, currentToken->lineNo, currentToken->colNo);
    compileArgument(node->object);
    node = node->next;

    while (lookAhead->tokenType == SB_COMMA) {
      eat(SB_COMMA);
      if (node == NULL)
	error(ERR_PARAMETERS_ARGUMENTS_INCONSISTENCY, currentToken->lineNo, currentToken->colNo);
      compileArgument(node->object);
      node = node->next;
    }

    if (node != NULL)
      error(ERR_PARAMETERS_ARGUMENTS_INCONSISTENCY, currentToken->lineNo, currentToken->colNo);
    
    eat(SB_RPAR);
    break;
    // Check FOLLOW set 
  case SB_TIMES:
  case SB_SLASH:
  case SB_PLUS:
  case SB_MINUS:
  case KW_TO:
  case KW_DO:
  case SB_RPAR:
  case SB_COMMA:
  case SB_EQ:
  case SB_NEQ:
  case SB_LE:
  case SB_LT:
  case SB_GE:
  case SB_GT:
  case SB_RSEL:
  case SB_SEMICOLON:
  case KW_END:
  case KW_ELSE:
  case KW_THEN:
    break;
  default:
    error(ERR_INVALID_ARGUMENTS, lookAhead->lineNo, lookAhead->colNo);
  }
}

void compileCondition(void) {

if(selfCheck==1)
printf("compileCondition \n");

  Type* type1;
  Type* type2;
  TokenType op;

  type1 = compileExpression();
  checkBasicType(type1);

  op = lookAhead->tokenType;
  switch (op) {
  case SB_EQ:
    eat(SB_EQ);
    break;
  case SB_NEQ:
    eat(SB_NEQ);
    break;
  case SB_LE:
    eat(SB_LE);
    break;
  case SB_LT:
    eat(SB_LT);
    break;
  case SB_GE:
    eat(SB_GE);
    break;
  case SB_GT:
    eat(SB_GT);
    break;
  default:
    error(ERR_INVALID_COMPARATOR, lookAhead->lineNo, lookAhead->colNo);
  }

  type2 = compileExpression();
  checkTypeEquality(type1,type2);

  switch (op) {
  case SB_EQ:
if(selfCheck==1)
printf("genEQ \n");
    genEQ();
    break;
  case SB_NEQ:
if(selfCheck==1)
printf("genNE \n");
    genNE();
    break;
  case SB_LE:
if(selfCheck==1)
printf("genLE \n");
    genLE();
    break;
  case SB_LT:
if(selfCheck==1)
printf("break \n");
    genLT();
    break;
  case SB_GE:
if(selfCheck==1)
printf("genGE \n");
    genGE();
    break;
  case SB_GT:
if(selfCheck==1)
printf("genGT \n");
    genGT();
    break;
  default:
    break;
  }

}

Type* compileExpression(void) {

if(selfCheck==1)
printf("compileExpression \n");

  Type* type;
  
  switch (lookAhead->tokenType) {
  case SB_PLUS:
    eat(SB_PLUS);
    type = compileExpression2();
    checkIntType(type);
    break;
  case SB_MINUS:
    eat(SB_MINUS);
    type = compileExpression2();
    checkIntType(type);
if(selfCheck==1)
printf("genNEG \n");
    genNEG();
    break;
  default:
    type = compileExpression2();
  }
  return type;
}

Type* compileExpression2(void) {

if(selfCheck==1)
printf("compileExpression2 \n");

  Type* type;

  type = compileTerm();
  type = compileExpression3(type);

  return type;
}


Type* compileExpression3(Type* argType1) {

if(selfCheck==1)
printf("compileExpression3 \n");

  Type* argType2;
  Type* resultType;

  switch (lookAhead->tokenType) {
  case SB_PLUS:
    eat(SB_PLUS);
    checkIntType(argType1);
    argType2 = compileTerm();
    checkIntType(argType2);

if(selfCheck==1)
printf("genAD \n");
    genAD();

    resultType = compileExpression3(argType1);
    break;
  case SB_MINUS:
    eat(SB_MINUS);
    checkIntType(argType1);
    argType2 = compileTerm();
    checkIntType(argType2);

if(selfCheck==1)
printf("genSB \n");
    genSB();

    resultType = compileExpression3(argType1);
    break;
    // check the FOLLOW set
  case KW_TO:
  case KW_DO:
  case SB_RPAR:
  case SB_COMMA:
  case SB_EQ:
  case SB_NEQ:
  case SB_LE:
  case SB_LT:
  case SB_GE:
  case SB_GT:
  case SB_RSEL:
  case SB_SEMICOLON:
  case KW_END:
  case KW_ELSE:
  case KW_THEN:
    resultType = argType1;
    break;
  default:
    error(ERR_INVALID_EXPRESSION, lookAhead->lineNo, lookAhead->colNo);
  }
  return resultType;
}

Type* compileTerm(void) {

if(selfCheck==1)
printf("compileTerm \n");

  Type* type;
  type = compileFactor();
  type = compileTerm2(type);

  return type;
}

Type* compileTerm2(Type* argType1) {

if(selfCheck==1)
printf("compileTerm2 \n");

  Type* argType2;
  Type* resultType;

  switch (lookAhead->tokenType) {
  case SB_TIMES:
    eat(SB_TIMES);
    checkIntType(argType1);
    argType2 = compileFactor();
    checkIntType(argType2);

if(selfCheck==1)
printf("genML \n");
    genML();

    resultType = compileTerm2(argType1);
    break;
  case SB_SLASH:
    eat(SB_SLASH);
    checkIntType(argType1);
    argType2 = compileFactor();
    checkIntType(argType2);

if(selfCheck==1)
printf("genDV \n");
    genDV();

    resultType = compileTerm2(argType1);
    break;
    // check the FOLLOW set
  case SB_PLUS:
  case SB_MINUS:
  case KW_TO:
  case KW_DO:
  case SB_RPAR:
  case SB_COMMA:
  case SB_EQ:
  case SB_NEQ:
  case SB_LE:
  case SB_LT:
  case SB_GE:
  case SB_GT:
  case SB_RSEL:
  case SB_SEMICOLON:
  case KW_END:
  case KW_ELSE:
  case KW_THEN:
    resultType = argType1;
    break;
  default:
    error(ERR_INVALID_TERM, lookAhead->lineNo, lookAhead->colNo);
  }
  return resultType;
}

Type* compileFactor(void) {

if(selfCheck==1)
printf("compileFactor \n");

  Type* type;
  Object* obj;

  switch (lookAhead->tokenType) {
  case TK_NUMBER:
    eat(TK_NUMBER);
    type = intType;
if(selfCheck==1)
printf("genLC \n");
    genLC(currentToken->value);
    break;
  case TK_CHAR:
    eat(TK_CHAR);
    type = charType;
if(selfCheck==1)
printf("genLC \n");
    genLC(currentToken->value);
    break;
  case TK_IDENT:
    eat(TK_IDENT);
    obj = checkDeclaredIdent(currentToken->string);

    switch (obj->kind) {
    case OBJ_CONSTANT:
      switch (obj->constAttrs->value->type) {
      case TP_INT:
	type = intType;
if(selfCheck==1)
printf("genLC \n");
	genLC(obj->constAttrs->value->intValue);
	break;
      case TP_CHAR:
	type = charType;
if(selfCheck==1)
printf("genLC \n");
	genLC(obj->constAttrs->value->charValue);
	break;
      default:
	break;
      }
      break;
    case OBJ_VARIABLE:
      if (obj->varAttrs->type->typeClass == TP_ARRAY) {
if(selfCheck==1)
printf("genVariableAddress \n");
	genVariableAddress(obj);
	type = compileIndexes(obj->varAttrs->type);
if(selfCheck==1)
printf("genLI \n");
	genLI();
      } else {
	type = obj->varAttrs->type;
if(selfCheck==1)
printf("genVariableValue \n");
	genVariableValue(obj);
      }
      break;
    case OBJ_PARAMETER:
      type = obj->paramAttrs->type;
if(selfCheck==1)
printf("genParameterValue \n");
      genParameterValue(obj);
      if (obj->paramAttrs->kind == PARAM_REFERENCE){
if(selfCheck==1)
printf("genLI \n");
	genLI();}
      break;
    case OBJ_FUNCTION:
      if (isPredefinedFunction(obj)) {
	compileArguments(obj->funcAttrs->paramList);
if(selfCheck==1)
printf("genPredefinedFunctionCall \n");
	genPredefinedFunctionCall(obj);
      } else {
if(selfCheck==1)
printf("genINT \n");
	genINT(4);
	compileArguments(obj->funcAttrs->paramList);
if(selfCheck==1)
printf("genDCT \n");
	genDCT(4+obj->funcAttrs->paramCount);
if(selfCheck==1)
printf("genFunctionCall \n");
	genFunctionCall(obj);
      }
      type = obj->funcAttrs->returnType;
      break;
    default: 
      error(ERR_INVALID_FACTOR,currentToken->lineNo, currentToken->colNo);
      break;
    }
    break;
  case SB_LPAR:
    eat(SB_LPAR);
    type = compileExpression();
    eat(SB_RPAR);
    break;
  default:
    error(ERR_INVALID_FACTOR, lookAhead->lineNo, lookAhead->colNo);
  }
  
  return type;
}

Type* compileIndexes(Type* arrayType) {

if(selfCheck==1)
printf("compileIndexes \n");

  Type* type;

  
  while (lookAhead->tokenType == SB_LSEL) {
    eat(SB_LSEL);
    type = compileExpression();
    checkIntType(type);
    checkArrayType(arrayType);

if(selfCheck==1)
printf("genLC \n");
    genLC(sizeOfType(arrayType->elementType));
if(selfCheck==1)
printf("genML \n");
    genML();
if(selfCheck==1)
printf("genAD \n");
    genAD();

    arrayType = arrayType->elementType;
    eat(SB_RSEL);
  }
  checkBasicType(arrayType);
  return arrayType;
}

int compile(char *fileName) {
  if (openInputStream(fileName) == IO_ERROR)
    return IO_ERROR;

  currentToken = NULL;
  lookAhead = getValidToken();

  initSymTab();

  compileProgram();


  cleanSymTab();
  free(currentToken);
  free(lookAhead);
  closeInputStream();
  return IO_SUCCESS;

}
