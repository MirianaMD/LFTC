#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include "parser.h"
#include "lexer.h"
#include "utils.h"

Token *iTk;        // the iterator in the tokens list
Token *consumedTk; // the last consumed token

void tkerr(const char *fmt, ...);
bool unit();
bool structDef();
bool varDef();
bool typeBase();
bool arrayDecl();
bool fnDef();
bool fnParam();
bool stm();
bool stmCompound();
bool expr();
bool exprAssign();
bool exprOr();
bool exprAnd();
bool exprEq();
bool exprRel();
bool exprAdd();
bool exprMul();
bool exprCast();
bool exprUnary();
bool exprPostfix();
bool exprPrimary();

//Mesajele de eroare nu ar trebui sa contina componente din gramatica (ex: RACC, SEMICOLO etc) si ele trebuie sa fie cat mai specifice pentru problema aparuta

void tkerr(const char *fmt, ...) {
  fprintf(stderr, "error in line %d: ", iTk->line);
  va_list va;
  va_start(va, fmt);
  vfprintf(stderr, fmt, va);
  va_end(va);
  fprintf(stderr, "\n");
  exit(EXIT_FAILURE);
}

bool consume(int code) {
  if (iTk->code == code) {
    consumedTk = iTk;
    iTk = iTk->next;
    return true;
  }
  return false;
}

// unit: ( structDef | fnDef | varDef )* END
bool unit() {
    for (;;) {
        if (structDef()) {
        } else if (fnDef()) {
        } else if (varDef()) {
        } else {
            break;
        }
    }
    if (consume(END)) {
        return true;
    } else tkerr("Syntax error for program from unit to end!");
    return false;
}

// structDef: STRUCT ID LACC varDef* RACC SEMICOLON
bool structDef() {
    Token *start = iTk;
    if (consume(STRUCT)) {
        if (consume(ID)) {
            if (consume(LACC)) {
                for (;;) {
                    if (varDef()) {
                    } else {
                        break;
                    }
                }
                if (consume(RACC)) {
                    if (consume(SEMICOLON)) {
                        return true;
                    } else tkerr("Missing ; after struct definition!");
                } else tkerr("Missing '}' in struct definition!");
            }
        } else tkerr("Struct has not an identifier name!");
    }
    iTk = start;
    return false;
}

//varDef: typeBase ID arrayDecl? SEMICOLON
bool varDef() {
    Token *start = iTk;
    if(typeBase()){
        if(consume(ID)){
            if(arrayDecl()){}
            if(consume(SEMICOLON)){
                return true;
            } else tkerr("Expected ; after member definition!");
        } else tkerr("Expected identifier after variable type!");
    }
    iTk = start;
    return false;
}

// typeBase: TYPE_INT | TYPE_DOUBLE | TYPE_CHAR | STRUCT ID
bool typeBase() {
    Token *start = iTk;
  if (consume(TYPE_INT)) {
      return true;
  }
  if (consume(TYPE_DOUBLE)) {
      return true;
  }
  if (consume(TYPE_CHAR)) {
      return true;
  }
  if (consume(STRUCT)) {
    if (consume(ID)) {
        return true;
    } else tkerr("Struct has not an identifier!");
  }
  iTk = start;
  return false;
}

//arrayDecl: LBRACKET INT? RBRACKET
bool arrayDecl() {
    Token *start = iTk;
    if (consume(LBRACKET)) {
        if (consume(INT)) {}
        if (consume(RBRACKET)) {
            return true;
        } else tkerr("Missing ] after array declaration!");
    }
    iTk = start;
    return false;
}

//fnDef: ( typeBase | VOID ) ID
//LPAR ( fnParam ( COMMA fnParam )* )? RPAR
//stmCompound //corpul functiei
bool fnDef(){
    Token *start = iTk;
    if(typeBase()){
        if(consume(ID)){
            if(consume(LPAR)){
                if(fnParam()) {
                    while (consume(COMMA)) {
                        if (fnParam()) {}
                        else tkerr("Missing type base after , in function's header!");
                    }
                }
                if(consume(RPAR)) {
                    if(stmCompound()){
                        return true;
                    } else tkerr("Expected body for function declared!");
                } else tkerr("Missing ) in function header!");
            }
        } else tkerr("Expected type base for function identifier!");
    }

    if(consume(VOID)) {
        if (consume(ID)) {
            if (consume(LPAR)) {
                if (fnParam()) {
                    while (consume(COMMA)) {
                        if (fnParam()) {}
                        else tkerr("Missing type base after , in function's header!");
                    }
                }
                if (consume(RPAR)) {
                    if (stmCompound()) {
                        return true;
                    } else tkerr("Expected body for function declared!");
                } else tkerr("Missing ) in function header!");
            }
        } else tkerr("Expected type base for function identifier!");
    }
    iTk = start;
    return false;
}

//fnParam: typeBase ID arrayDecl?
bool fnParam(){
    Token *start = iTk;
    if(typeBase()){
        if(consume(ID)){
            if(arrayDecl()){}
            return true;
        } else tkerr("Missing parameter name!");
    }
    iTk = start;
    return false;
}

//stm: stmCompound
//| IF LPAR expr RPAR stm ( ELSE stm )?
//| WHILE LPAR expr RPAR stm
//| RETURN expr? SEMICOLON
//| expr? SEMICOLON
bool stm(){
    Token *start = iTk;
    if(stmCompound())
    {   return true;
    }
    if(consume(IF)){
        if(consume(LPAR)){
            if(expr()){
                if(consume(RPAR)){
                    if(stm()){
                        if(consume(ELSE)) {
                            if (stm()) {}
                            else tkerr("Expected statement after ELSE!");
                        }
                        return true;
                    } else tkerr("Expected statement after IF!");
                } else tkerr("Missing ) after IF!");
            } else tkerr("Expected expression in IF!");
        } else tkerr("Expected ( after IF!");
    }
    if(consume(WHILE)){
        if(consume(LPAR)){
            if(expr()){
                if(consume(RPAR)){
                    if(stm()){
                        return true;
                    } else tkerr("Expected statement after WHILE!");
                } else tkerr("Missing ) after WHILE!");
            }else tkerr("Expected expression in WHILE!");
        } else tkerr("Expected expression after WHILE! / Expected ( after WHILE!");
    }
    if(consume(RETURN)){
        if(expr()){}
        if(consume(SEMICOLON)){
            return true;
        } else tkerr("Missing ; after RETURN!");
    }
    if(expr()) {
        if (consume(SEMICOLON)) {
            return true;
        } else tkerr("Expected ; after expression!");
    }
    if (consume(SEMICOLON))
            return true;

    iTk = start;
    return false;
}

//stmCompound: LACC ( varDef | stm )* RACC
bool stmCompound(){
    Token *start = iTk;
    if(consume(LACC)){
        for(;;){
            if(varDef()){
            } else if (stm()){
            } else {
                break;
            }
        }
        if(consume(RACC)){
            return true;
        } else tkerr("Missing } after function block!");
    }
    iTk = start;
    return false;
}

//expr: exprAssign
bool expr(){
    Token *start = iTk;
    if(exprAssign()){
        return true;
    }
    iTk = start;
    return false;
}

//exprAssign: exprUnary ASSIGN exprAssign | exprOr
// a;
bool exprAssign(){
    Token *start = iTk;
    if(exprUnary()){
        if(consume(ASSIGN)){
            if(exprAssign()){
                return true;
            } else tkerr("Expected expression after = !");
        }
    }
    iTk = start;
    if(exprOr()){
        return true;
    }
    iTk = start;
    return false;
}

// exprOr: exprOr OR exprAnd | exprAnd
bool exprOrPrim() {
    Token *start = iTk;
    if (consume(OR)) {
        if (exprAnd()) {
            if (exprOrPrim()) {
                return true;
            }
        } else tkerr("Invalid expression after || !");
    }
    iTk = start;
    return true;
}

bool exprOr() {
    Token *start = iTk;
    if (exprAnd()) {
        if (exprOrPrim()) {
            return true;
        }
    }
    iTk = start;
    return false;
}

//exprAndPrim: AND exprEq exprAndPrim | epsilon <=> (AND exprEq exprAndPrim)?  //final
bool exprAndPrim(){
    Token *start = iTk;
    if(consume(AND)){
        if(exprEq()){
            if(exprAndPrim()){
                return true;
            }
        }
        else tkerr("Invalid expression after && !");
    }
    iTk = start;
    return true;
}

//exprAnd: exprAnd AND exprEq | exprEq
//exprAnd: exprEq exprAndPrim  //final
bool exprAnd(){
    Token *start = iTk;
    if(exprEq()){
        if(exprAndPrim()){
            return true;
        }
    }
    iTk = start;
    return false;
}

//exprEqPrim: (( EQUAL | NOTEQ ) exprRel exprEqPrim)?  //final
bool exprEqPrim(){
    Token *start = iTk;
    if(consume(EQUAL)){
        if(exprRel()){
            if(exprEqPrim()){
                return true;
            }
        } else tkerr("Invalid expression after == !");
    }
    if(consume(NOTEQ)){
        if(exprRel()){
            if(exprEqPrim()){
                return true;
            }
        } else tkerr("Invalid expression after != !");
    }
    iTk = start;
    return true;
}

//exprEq: exprEq ( EQUAL | NOTEQ ) exprRel | exprRel
//exprEq: exprRel exprEqPrim //final
bool exprEq(){
    Token *start = iTk;
    if(exprRel()){
        if(exprEqPrim()){
            return true;
        }
    }
    iTk = start;
    return false;
}

//exprRelPrim: (( LESS | LESSEQ | GREATER | GREATEREQ ) exprAdd exprRelPrim)?
bool exprRelPrim(){
    Token *start = iTk;
    if(consume(LESS)){
        if(exprAdd()){
            if(exprRelPrim()){
                return true;
            }
        } else tkerr("Invalid expression after < !");
    }
    if(consume(LESSEQ)){
        if(exprAdd()){
            if(exprRelPrim()){
                return true;
            }
        } else tkerr("Invalid expression after <= !");
    }
    if(consume(GREATER)){
        if(exprAdd()){
            if(exprRelPrim()){
                return true;
            }
        } else tkerr("Invalid expression after > !");
    }
    if(consume(GREATEREQ)){
        if(exprAdd()){
            if(exprRelPrim()){
                return true;
            }
        } else tkerr("Invalid expression after >= !");
    }
    iTk = start;
    return true;

}
//exprRel: exprRel ( LESS | LESSEQ | GREATER | GREATEREQ ) exprAdd | exprAdd
//exprRel: exprAdd exprRelPrim
bool exprRel(){
    Token *start = iTk;
    if(exprAdd()){
        if(exprRelPrim()){
            return true;
        }
    }
    iTk = start;
    return false;
}

//exprAddPrim: (( ADD | SUB ) exprMul exprAddPrim)?
bool exprAddPrim(){
    Token *start = iTk;
    if(consume(ADD)){
        if (exprMul()) {
            if (exprAddPrim()) {
                return true;
            }
        } else tkerr("Invalid expression after + !");
    }
    if(consume(SUB)) {
        if (exprMul()) {
            if (exprAddPrim()) {
                return true;
            }
        } else tkerr("Invalid expression after - !");
    }

    iTk = start;
    return true;
}
//exprAdd: exprAdd ( ADD | SUB ) exprMul | exprMul
//exprAdd: exprMul exprAddPrim
bool exprAdd(){
    Token *start = iTk;
    if(exprMul()){
        if(exprAddPrim()){
            return true;
        }
    }
    iTk = start;
    return false;
}

//exprMulPrim: ((MUL|DIV) exprCast exprMulPrim)?
bool exprMulPrim(){
    Token *start = iTk;
    if(consume(MUL)){
        if(exprCast()){
            if(exprMulPrim()){
                return true;
            }
        } else tkerr("Invalid expression after * !");
    }

    if(consume(DIV)){
        if(exprCast()){
            if(exprMulPrim()){
                return true;
            }
        } else tkerr("Invalid expression after / !");
    }
    iTk = start;
    return true;
}
//exprMul: exprMul ( MUL | DIV ) exprCast | exprCast
//exprMul: exprCast exprMulPrim
bool exprMul(){
    Token *start = iTk;
    if(exprCast()){
        if(exprMulPrim()){
            return true;
        }
    }
    iTk = start;
    return false;
}

//exprCast: LPAR typeBase arrayDecl? RPAR exprCast | exprUnary
bool exprCast(){
    Token *start = iTk;
    if(consume(LPAR)){
        if(typeBase()){
            if(arrayDecl()){}
            if(consume(RPAR)){
                if(exprCast()){
                    return true;
                }
            } else tkerr("Missing ) for cast!");
        }
    }
    iTk = start;
    if(exprUnary()){
        return true;
    }
    iTk = start;
    return false;
}

//exprUnary: ( SUB | NOT ) exprUnary | exprPostfix
bool exprUnary(){
    Token *start = iTk;
    if(consume(SUB)){
        if(exprUnary()){
            return true;
        }
    }
    if(consume(NOT)){
        if(exprUnary()){
            return true;
        }
    }
    iTk = start;
    if(exprPostfix()){
        return true;
    }
    iTk = start;
    return false;
}

// (LBRACKET expr RBRACKET exprPostfixPrim | DOT ID exprPostfixPrim)?
bool exprPostfixPrim(){
    Token *start = iTk;
    if(consume(LBRACKET)){
        if(expr()){
            if(consume(RBRACKET)){
                if(exprPostfixPrim()){
                    return true;
                }
            } else tkerr("Missing ] after expression!");
        } else tkerr("Expected expression after [ !");
    }
    if(consume(DOT)){
        if(consume(ID)){
            if(exprPostfixPrim()){
                return true;
            }
        } else tkerr("Expected identifier after . !");
    }
    iTk = start;
    return true;
}

//exprPostfix: exprPostfix LBRACKET expr RBRACKET
//| exprPostfix DOT ID
//| exprPrimary
bool exprPostfix(){
    Token *start = iTk;
    if(exprPrimary()){
        if(exprPostfixPrim()){
            return true;
        }
    }
    iTk = start;
    return false;
}

//exprPrimary: ID ( LPAR ( expr ( COMMA expr )* )? RPAR )?
//| INT | DOUBLE | CHAR | STRING | LPAR expr RPAR
bool exprPrimary(){
    Token *start = iTk;
    if(consume(ID)){
        if(consume(LPAR)){
            if(expr()){
                for(;;){
                    if(consume(COMMA)){
                        if(expr()){}
                        else tkerr("Expected expression after , in function call!");
                    } else {
                        break;
                    }
                }
            }
           if(consume(RPAR)){
               return true;
           } else tkerr("Missing ) in function call!");
        }
        return true;
    }
    if(consume(INT)){
        return true;
    }
    if(consume(DOUBLE)){
        return true;
    }
    if(consume(CHAR)){
        return true;
    }
    if(consume(STRING)){
        return true;
    }
    if(consume(LPAR)){
        if(expr()){
            if(consume(RPAR)){
                return true;
            } else tkerr("Missing ) after expression!");
        }
    }
    iTk = start;
    return false;
}

void parse(Token *tokens) {
  iTk = tokens;
  if (!unit()) {
    tkerr("Syntax error!");
  }
}

