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

int checkError=0;

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

//EXERCISE 4 ------------------------------------------------------------------------
void eat(TokenType tokenType) {
  if(checkError==1)
    printf("eat\n");
  
  if (lookAhead->tokenType == tokenType) {
    printToken(lookAhead);
    scan();
  } else typeTransform(tokenType);
}

//EXERCISE 4 ---------------------------------------------------------------------
void typeTransform(TokenType tokenType) {
  if(checkError==1)
    printf("typeTransform\n");
    
  if(tokenType == TK_DOUBLE){
    if (lookAhead->tokenType == TK_INT) {
      lookAhead->tokenType =TK_DOUBLE;
      lookAhead->doubleValue = (double)(lookAhead->intValue);//ep kieu
      scan();
    }
  }  else missingToken(tokenType, lookAhead->lineNo, lookAhead->colNo);
}

void compileProgram(void) {
  if(checkError==1)
    printf("compileProgram\n");
    
  Object* program;

  eat(KW_PROGRAM);
  eat(TK_IDENT);

  program = createProgramObject(currentToken->string);
  enterBlock(program->progAttrs->scope);

  eat(SB_SEMICOLON);

  compileBlock();
  eat(SB_PERIOD);

  exitBlock();
}

void compileBlock(void) {
  if(checkError==1)
    printf("compileBlock\n");
    
  Object* constObj;
  ConstantValue* constValue;

  if (lookAhead->tokenType == KW_CONST) {
    eat(KW_CONST);

    do {
      eat(TK_IDENT);
      
      checkFreshIdent(currentToken->string);
      constObj = createConstantObject(currentToken->string);
      
      eat(SB_EQ);
      constValue = compileConstant();
      
      constObj->constAttrs->value = constValue;
      declareObject(constObj);
      
      eat(SB_SEMICOLON);
    } while (lookAhead->tokenType == TK_IDENT);

    compileBlock2();
  } 
  else compileBlock2();
}

void compileBlock2(void) {
  if(checkError==1)
    printf("compileBlock2\n");
    
  Object* typeObj;
  Type* actualType;

  if (lookAhead->tokenType == KW_TYPE) {
    eat(KW_TYPE);

    do {
      eat(TK_IDENT);
      
      checkFreshIdent(currentToken->string);
      typeObj = createTypeObject(currentToken->string);
      
      eat(SB_EQ);
      actualType = compileType();
      
      typeObj->typeAttrs->actualType = actualType;
      declareObject(typeObj);
      
      eat(SB_SEMICOLON);
    } while (lookAhead->tokenType == TK_IDENT);

    compileBlock3();
  } 
  else compileBlock3();
}

void compileBlock3(void) {
  if(checkError==1)
    printf("compileBlock3\n");
    
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

    compileBlock4();
  } 
  else compileBlock4();
}

void compileBlock4(void) {
  if(checkError==1)
    printf("compileBlock4\n");
    
  compileSubDecls();
  compileBlock5();
}

void compileBlock5(void) {
  if(checkError==1)
    printf("compileBlock5\n");
    
  eat(KW_BEGIN);
  compileStatements();
  eat(KW_END);
}

void compileSubDecls(void) {
  if(checkError==1)
    printf("compileSubDecls\n");
    
  while ((lookAhead->tokenType == KW_FUNCTION) || (lookAhead->tokenType == KW_PROCEDURE)) {
    if (lookAhead->tokenType == KW_FUNCTION)
      compileFuncDecl();
    else compileProcDecl();
  }
}

void compileFuncDecl(void) {
  if(checkError==1)
    printf("compileFuncDecl\n");
    
  Object* funcObj;
  Type* returnType;

  eat(KW_FUNCTION);
  eat(TK_IDENT);

  checkFreshIdent(currentToken->string);
  funcObj = createFunctionObject(currentToken->string);
  declareObject(funcObj);

  enterBlock(funcObj->funcAttrs->scope);
  
  compileParams();

  eat(SB_COLON);
  returnType = compileBasicType();
  funcObj->funcAttrs->returnType = returnType;

  eat(SB_SEMICOLON);
  compileBlock();
  eat(SB_SEMICOLON);

  exitBlock();
}

void compileProcDecl(void) {
  if(checkError==1)
    printf("compileProcDecl\n");
    
  Object* procObj;

  eat(KW_PROCEDURE);
  eat(TK_IDENT);

  checkFreshIdent(currentToken->string);
  procObj = createProcedureObject(currentToken->string);
  declareObject(procObj);

  enterBlock(procObj->procAttrs->scope);

  compileParams();

  eat(SB_SEMICOLON);
  compileBlock();
  eat(SB_SEMICOLON);

  exitBlock();
}

//EXERCISE 4 --------------------------------------------------------------------------------------------------
ConstantValue* compileUnsignedConstant(void) {
  if(checkError==1)
    printf("checkError\n");
    
  ConstantValue* constValue;
  Object* obj;

  switch (lookAhead->tokenType) {
  case TK_INT:
    eat(TK_INT);
    constValue = makeIntConstant(currentToken->intValue);
    break;
    
  case TK_DOUBLE:
    eat(TK_DOUBLE);
    constValue = makeDoubleConstant(currentToken->doubleValue);
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
  case TK_STRING:
    eat(TK_STRING);
    constValue = makeStringConstant(currentToken->stringValue);
    break;
  default:
    error(ERR_INVALID_CONSTANT, lookAhead->lineNo, lookAhead->colNo);
    break;
  }
  return constValue;
}

//EXERCISE 4 -----------------------------------------------------------------------------------------------------
ConstantValue* compileConstant(void) {
  if(checkError==1)
    printf("compileConstant\n");
    
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
  case TK_STRING:
    eat(TK_STRING);
    constValue = makeStringConstant(currentToken->stringValue);
    break;
  default:
    constValue = compileConstant2();
    break;
  }
  return constValue;
}

//EXERCISE 4 -------------------------------------
ConstantValue* compileConstant2(void) {
  if(checkError==1)
    printf("compileConstant2\n");
    
  ConstantValue* constValue;
  Object* obj;

  switch (lookAhead->tokenType) {
  case TK_INT:
    eat(TK_INT);
    constValue = makeIntConstant(currentToken->intValue);
    break;
  case TK_DOUBLE:
    eat(TK_DOUBLE);
    constValue = makeDoubleConstant(currentToken->doubleValue);
    break;
  case TK_IDENT:
    eat(TK_IDENT);
    obj = checkDeclaredConstant(currentToken->string);
    if (obj->constAttrs->value->type == TP_INT){
      constValue = duplicateConstantValue(obj->constAttrs->value);
    } else if (obj->constAttrs->value->type == TP_DOUBLE){
      constValue = duplicateConstantValue(obj->constAttrs->value);
    }  else
      error(ERR_UNDECLARED_DOUBLE_CONSTANT,currentToken->lineNo, currentToken->colNo);
     
    break;
  default:
    error(ERR_INVALID_CONSTANT, lookAhead->lineNo, lookAhead->colNo);
    break;
  }
  return constValue;
}

//EXERCISE 4 --------------------------------------------------------------------------------------------
Type* compileType(void) {
  if(checkError==1)
    printf("compileType\n");
    
  Type* type;
  Type* elementType;
  int arraySize;
  Object* obj;

  switch (lookAhead->tokenType) {
  case KW_INTEGER: 
    eat(KW_INTEGER);
    type =  makeIntType();
    break;
  case KW_DOUBLE: 
    eat(KW_DOUBLE);
    type =  makeDoubleType();
    break;
  case KW_CHAR: 
    eat(KW_CHAR); 
    type = makeCharType();
    break;
  case KW_STRING: 
    eat(KW_STRING);
    type =  makeStringType();
    break;
  case KW_ARRAY:
    eat(KW_ARRAY);
    eat(SB_LSEL);
    eat(TK_INT);

    arraySize = currentToken->intValue;

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

//EXERCISE 4 ------------------------------------------------------------------------------------------------------------
Type* compileBasicType(void) {
  if(checkError==1)
    printf("compileBasicType\n");
    
  Type* type;

  switch (lookAhead->tokenType) {
  case KW_INTEGER: 
    eat(KW_INTEGER); 
    type = makeIntType();
    break;
  case KW_DOUBLE: 
    eat(KW_DOUBLE); 
    type = makeDoubleType();
    break;
  case KW_CHAR: 
    eat(KW_CHAR); 
    type = makeCharType();
    break;
  case KW_STRING: 
    eat(KW_STRING); 
    type = makeStringType();
    break;
  default:
    error(ERR_INVALID_BASICTYPE, lookAhead->lineNo, lookAhead->colNo);
    break;
  }
  return type;
}

void compileParams(void) {
  if(checkError==1)
    printf("compileParams\n");
    
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
  if(checkError==1)
    printf("compileParam\n");
    
  Object* param;
  Type* type;
  enum ParamKind paramKind;

  switch (lookAhead->tokenType) {
  case TK_IDENT:
    paramKind = PARAM_VALUE;
    break;
  case KW_VAR:
    eat(KW_VAR);
    paramKind = PARAM_REFERENCE;
    break;
  default:
    error(ERR_INVALID_PARAMETER, lookAhead->lineNo, lookAhead->colNo);
    break;
  }

  eat(TK_IDENT);
  checkFreshIdent(currentToken->string);
  param = createParameterObject(currentToken->string, paramKind, symtab->currentScope->owner);
  eat(SB_COLON);
  type = compileBasicType();
  param->paramAttrs->type = type;
  declareObject(param);
}

void compileStatements(void) {
  if(checkError==1)
    printf("compileStatements\n");
    
  compileStatement();
  while (lookAhead->tokenType == SB_SEMICOLON) {
    eat(SB_SEMICOLON);
    compileStatement();
  }
}

void compileStatement(void) {
  if(checkError==1)
    printf("compileStatement\n");
    
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
  if(checkError==1)
    printf("compileLValue\n");
    
  // parse a lvalue (a variable, an array element, a parameter, the current function identifier)
  Object* var = NULL;
  Type* varType = NULL;

  eat(TK_IDENT);
  // check if the identifier is a function identifier, or a variable identifier, or a parameter  
  var = checkDeclaredLValueIdent(currentToken->string);
  if (var->kind == OBJ_VARIABLE)
    varType = compileIndexes(var->varAttrs->type);
  else if (var->kind == OBJ_FUNCTION)
    varType = var->funcAttrs->returnType;
  else if (var->kind == OBJ_PARAMETER)
    varType = var->paramAttrs->type;

  return varType;
}

void compileAssignSt(void) {
  if(checkError==1)
    printf("compileAssignSt\n");
    
  // parse the assignment and check type consistency
  Type *lvalueType = NULL;
  Type *expType = NULL;

  lvalueType = compileLValue();
  eat(SB_ASSIGN);
  expType =  compileExpression();

  checkTypeEquality(lvalueType, expType);
}

void compileCallSt(void) {
  if(checkError==1)
    printf("compileCallSt\n");
    
  Object* proc;

  eat(KW_CALL);
  eat(TK_IDENT);

  proc = checkDeclaredProcedure(currentToken->string);

  compileArguments(proc->procAttrs->paramList);
}

void compileGroupSt(void) {
  if(checkError==1)
    printf("compileGroupSt\n");
    
  eat(KW_BEGIN);
  compileStatements();
  eat(KW_END);
}

void compileIfSt(void) {
  if(checkError==1)
    printf("compileIfSt\n");
    
  eat(KW_IF);
  compileCondition();
  eat(KW_THEN);
  compileStatement();
  if (lookAhead->tokenType == KW_ELSE) 
    compileElseSt();
}

void compileElseSt(void) {
  if(checkError==1)
    printf("compileElseSt\n");
    
  eat(KW_ELSE);
  compileStatement();
}

void compileWhileSt(void) {
  if(checkError==1)
    printf("compileWhileSt\n");
    
  eat(KW_WHILE);
  compileCondition();
  eat(KW_DO);
  compileStatement();
}

void compileForSt(void) {
  if(checkError==1)
    printf("compileForSt\n");
    
  // Check type consistency of FOR's variable
  eat(KW_FOR);
  eat(TK_IDENT);

  // check if the identifier is a variable
  Object *var = checkDeclaredVariable(currentToken->string);
  checkBasicType(var->varAttrs->type);

  eat(SB_ASSIGN);
  Type *exp1Type = compileExpression();
  checkBasicType(exp1Type);

  eat(KW_TO);
  Type *exp2Type = compileExpression();
  checkBasicType(exp2Type);

  // Compare 3 types
  checkTypeEquality(var->varAttrs->type, exp1Type);
  checkTypeEquality(exp1Type, exp2Type);

  eat(KW_DO);
  compileStatement();
}

void compileArgument(Object* param) {
  if(checkError==1)
    printf("compileArgument\n");
    
  // parse an argument, and check type consistency
  //       If the corresponding parameter is a reference, the argument must be a lvalue

  if (param->paramAttrs->kind == PARAM_REFERENCE){

    if (lookAhead->tokenType == TK_IDENT) {
      checkDeclaredLValueIdent(lookAhead->string);
    } else {
      error(ERR_TYPE_INCONSISTENCY, lookAhead->lineNo, lookAhead->colNo);
    }
  }

  Type *argType = compileExpression();
  checkTypeEquality(argType, param->paramAttrs->type);

}

void compileArguments(ObjectNode* paramList) {
  if(checkError==1)
    printf("compileArguments\n");
    
  // parse a list of arguments, check the consistency of the arguments and the given parameters
    
  switch (lookAhead->tokenType) {
  case SB_LPAR:
    eat(SB_LPAR);
    compileArgument(paramList->object);

    while (lookAhead->tokenType == SB_COMMA) {
      eat(SB_COMMA);
      paramList = paramList->next;
      if (paramList != NULL)
	compileArgument(paramList->object);
      else
	error(ERR_PARAMETERS_ARGUMENTS_INCONSISTENCY, currentToken->lineNo, currentToken->colNo);
    }
    
    // param list still has next one when we've done parsing arguments
    // means number of arguments doesn't match number of params
    if (paramList->next != NULL)
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




    /*??????????????????????????????????????????::::::::::::::::::::::::::::::::::
    // Param list exists but we don't see left parenthesis
    if (paramList->object != NULL)
    error(ERR_PARAMETERS_ARGUMENTS_INCONSISTENCY, currentToken->lineNo, currentToken->colNo);
    ??????????????????????????????????????????????????*/
    
    break;
  default:
    error(ERR_INVALID_ARGUMENTS, lookAhead->lineNo, lookAhead->colNo);
  }
}

void compileCondition(void) {
  if(checkError==1)
    printf("compileCondition\n");
    
  // check the type consistency of LHS and RSH, check the basic type
  Type *exp1 = compileExpression();
  checkBasicType(exp1);

  switch (lookAhead->tokenType) {
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

  Type *exp2 = compileExpression();
  checkBasicType(exp2);

  // Compare 2 sides
  checkTypeEquality(exp1, exp2);
}

//EXERCISE 4 -----------------------------------------------------------------------------------------------------
Type* compileExpression(void) {
  if(checkError==1)
    printf("compileExpression\n");
    
  Type* type;
  
  switch (lookAhead->tokenType) {
    //with SB_PLUS and SB_MINUS, the following Token must have of Number
    //So with add two string, we have not to check error that expression start with SB_PLUS or SB_MINUS
  case SB_PLUS:
    eat(SB_PLUS);
    type = compileExpression2();
    checkNumType(type);
    break;
  case SB_MINUS:
    eat(SB_MINUS);
    type = compileExpression2();
    checkNumType(type);
    break;
  default:
    type = compileExpression2();
  }
  return type;
}

//EXERCISE 4 -----------------------------------------------------------------------------------------------------------
Type* compileExpression2(void) {
  if(checkError==1)
    printf("compileExpression2\n");
    
  Type* type;

  type = compileTerm();
  if (type->typeClass == TP_STRING)//Neu bat dau cua Expression la mot String:
    compileExpression4();//Dung cho phep cong 2 xau
    
  compileExpression3();//Expression3 Dung cho cong so

  return type;
}

//EXERCISE 4 ----------------------------------------------------------------------------------------------------------------
void compileExpression3(void) {
  if(checkError==1)
    printf("compileExpression3\n");
    
  Type* type;

  switch (lookAhead->tokenType) {
  case SB_PLUS:
    eat(SB_PLUS);
    type = compileTerm();
    checkNumType(type);
    compileExpression3();
    break;
  case SB_MINUS:
    eat(SB_MINUS);
    type = compileTerm();
    checkNumType(type);
    compileExpression3();
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
    break;
  default:
    error(ERR_INVALID_EXPRESSION, lookAhead->lineNo, lookAhead->colNo);
  }
}

//EXERCISE 4 ----------------------------------------------------------------------------------------------------
void compileExpression4(void) {//phep cong 2 xau
  if(checkError==1)
    printf("compileExpression4\n");
    
  Type* type;

  switch (lookAhead->tokenType) {
  case SB_PLUS:
    eat(SB_PLUS);
    type = compileTerm();
    checkStringType(type);
    compileExpression4();
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
    break;
  default:
    error(ERR_INVALID_EXPRESSION, lookAhead->lineNo, lookAhead->colNo);
  }
}

//EXERCISE 4 -------------------------------------
Type* compileTerm(void) {
  if(checkError==1)
    printf("compileTerm\n");
    
  Type* type;

  type = compileFactor();
  if (type->typeClass == TP_STRING)
    return type;//phep cong hai xau
  //vi compilieTerm2 dung cho phep * va / ma minh khong can xet den vi o day chi xet phpe + xau
  
  compileTerm2();

  return type;
}

//EXERCISE 4 -------------------------------------
void compileTerm2(void) {
  if(checkError==1)
    printf("compileTerm2\n");
    
  Type* type;

  switch (lookAhead->tokenType) {
  case SB_TIMES:
    eat(SB_TIMES);
    type = compileFactor();
    checkNumType(type);
    compileTerm2();
    break;
  case SB_SLASH:
    eat(SB_SLASH);
    type = compileFactor();
    checkNumType(type);
    compileTerm2();
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
    break;
  default:
    error(ERR_INVALID_TERM, lookAhead->lineNo, lookAhead->colNo);
  }
}

//EXERCISE 4 -------------------------------------
Type* compileFactor(void) {
  if(checkError==1)
    printf("compileFactor\n");
    
  // parse a factor and return the factor's type

  Object* obj = NULL;
  Type* type = NULL;

  switch (lookAhead->tokenType) {
  case TK_INT:
    eat(TK_INT);
    type = makeIntType();
    break;
    
  case TK_DOUBLE:
    eat(TK_DOUBLE);
    type = makeDoubleType();
    break;
    
  case TK_CHAR:
    eat(TK_CHAR);
    type = makeCharType();
    break;
    
  case TK_STRING:

    eat(TK_STRING);
    type = makeStringType();
    break;
    
    
  case TK_IDENT:
    eat(TK_IDENT);  
    // check if the identifier is declared
    obj = checkDeclaredIdent(currentToken->string);

    switch (obj->kind) {
    case OBJ_CONSTANT:
      // use as an empty type
      type = makeIntType();

      // assign the type of the constant
      type->typeClass = obj->constAttrs->value->type;
      break;
    case OBJ_VARIABLE:
      if (obj->varAttrs->type->typeClass != TP_ARRAY)
	type = obj->varAttrs->type;
      else
	type = compileIndexes(obj->varAttrs->type);
      break;
    case OBJ_PARAMETER:
      type = obj->paramAttrs->type;
      break;
    case OBJ_FUNCTION:
      type = obj->funcAttrs->returnType;
      compileArguments(obj->funcAttrs->paramList);
      break;
    default: 
      error(ERR_INVALID_FACTOR,currentToken->lineNo, currentToken->colNo);
      break;
    }
    break;
  default:
    error(ERR_INVALID_FACTOR, lookAhead->lineNo, lookAhead->colNo);
  }

  return type;
}

Type* compileIndexes(Type* arrayType) {
  if(checkError==1)
    printf("compileIndexes\n");
    
  // parse a sequence of indexes, check the consistency to the arrayType, and return the element type
  Type *idxType = NULL;
  Type *elmType = NULL;

  while (lookAhead->tokenType == SB_LSEL) {
    eat(SB_LSEL);

    // if current element is not of array type,
    // then the access to the next dimension is invalid
    checkArrayType(arrayType);

    idxType = compileExpression();
    checkIntType(idxType);

    eat(SB_RSEL);

    // Down 1 level of dimension
    arrayType = arrayType->elementType;
  }

  // arrayType becomes elmType when we traverse to the last dimension
  elmType = arrayType;

  return elmType;
}

int compile(char *fileName) {
  if(checkError==1)
    printf("compile\n");
    
  if (openInputStream(fileName) == IO_ERROR)
    return IO_ERROR;

  currentToken = NULL;
  lookAhead = getValidToken();

  initSymTab();

  compileProgram();

  printObject(symtab->program,0);

  cleanSymTab();

  free(currentToken);
  free(lookAhead);
  closeInputStream();
  return IO_SUCCESS;

}
