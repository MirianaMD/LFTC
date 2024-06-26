#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>

#include "lexer.h"
#include "utils.h"

Token *tokens;	// single linked list of tokens
Token *lastTk;  // the last token in list

int line=1;		// the current line in the input file

char ids[38][15] = { "ID",
		     "TYPE_CHAR","TYPE_DOUBLE","ELSE", "IF","TYPE_INT",
                     "RETURN", "STRUCT", "VOID", "WHILE",
                     "INT", "DOUBLE", "CHAR", "STRING",
                     "COMMA", "END", "SEMICOLON", "LPAR", "RPAR", "LBRACKET",
                     "RBRACKET", "LACC", "RACC",
                     "ASSIGN","EQUAL","ADD","SUB","MUL","DIV","DOT",
                     "AND","OR","NOT","NOTEQ","LESS","LESSEQ","GREATER","GREATEREQ"
};

// adds a token to the end of the tokens list and returns it
// sets its code and line
Token *addTk(int code){
  Token *tk=safeAlloc(sizeof(Token));
  tk->code=code;
  tk->line=line;
  tk->next=NULL;
  if(lastTk){
    lastTk->next=tk;
  }else{
    tokens=tk;
  }
  lastTk=tk;
  return tk;
}

char *extract(const char *begin,const char *end){
  //err("extract needs to be implemented");
  char* rez = safeAlloc(end-begin+1);
  int i=0;
  while(begin!=end){
    rez[i++]=*begin;
    begin++;
  }
  rez[i]='\0';
  return rez;
}

Token *tokenize(const char *pch){
  line=1;
  const char *start;
  Token *tk;
  for(;;){
    switch(*pch){
    case ' ':case '\t':pch++;break;
    case '\r': //handles different kinds of newlines (Windows: \r\n, Linux: \n, MacOS, OS X: \r or \n)
      if(pch[1]=='\n')pch++;
      // fallthrough to \n
    case '\n':
      line++;
      pch++;
      break;
      //char si string
    case '\'':
      if(pch[1]!='\'') {
	if(pch[2]=='\'') {
	  tk= addTk(CHAR);
	  tk->c=pch[1];
	  pch=pch+3;
	}
	else{
	  err("invalid char: %c - %d. Linia: %d",*pch,*pch,line);
	}
      }
      else{
	err("invalid char: %c - %d Linia: %d",*pch,*pch,line);
      }
      break;
    case '"':
      pch++;
      start=pch;
      while(*pch!='"')
	{
	  if (*pch == '\0' || *pch == '\n') {
	    err("Error, unterminated string constant!");
	  }
	  pch++;
	}
      char *text=extract(start,pch);
      tk=addTk(STRING);
      tk->text=text;
      pch++;
      break;

      //delimitators
    case ',':addTk(COMMA);pch++;break;
    case '\0':addTk(END);return tokens;
    case ';':addTk(SEMICOLON);pch++;break;
    case '(':addTk(LPAR);pch++;break;
    case ')':addTk(RPAR);pch++;break;
    case '[':addTk(LBRACKET);pch++;break;
    case ']':addTk(RBRACKET);pch++;break;
    case '{':addTk(LACC);pch++;break;
    case '}':addTk(RACC);pch++;break;

      //operators
    case '+':addTk(ADD);pch++;break;
    case '-':addTk(SUB);pch++;break;
    case '*':addTk(MUL);pch++;break;
    case '/':
      if(pch[1]=='/')//daca avem comentariu
	{
          for(start=pch++;*pch!='\n';pch++){}
	}else{
	addTk(DIV);
	pch++;
      }
      break;
    case '=':
      if(pch[1]=='='){
	addTk(EQUAL);
	pch=pch+2;
      }else{
	addTk(ASSIGN);
	pch++;
      }
      break;
    case '<':
      if(pch[1]=='='){
	addTk(LESSEQ);
	pch=pch+2;
      }else{
	addTk(LESS);
	pch++;
      }
      break;
    case '>':
      if(pch[1]=='='){
	addTk(GREATEREQ);
	pch=pch+2;
      }else{
	addTk(GREATER);
	pch++;
      }
      break;
    case '.':addTk(DOT);pch++;break;
    case '!':
      if(pch[1]=='='){
	addTk(NOTEQ);
	pch=pch+2;
      }else{
	addTk(NOT);
	pch++;
      }
      break;
    case '|':
      if(pch[1]=='|'){
	addTk(OR);
	pch=pch+2;
      }else{
	err("invalid char: %c - %d",*pch,*pch);
      }
      break;
    case '&':
      if(pch[1]=='&'){
	addTk(AND);
	pch=pch+2;
      }else{
	err("invalid char: %c - %d",*pch,*pch);
      }
      break;

    default:
      start = pch;
      if(isalpha(*pch)||*pch=='_'){
	for(start=pch++;isalnum(*pch)||*pch=='_';pch++){}
	char *text=extract(start,pch);
	if(strcmp(text,"char")==0)addTk(TYPE_CHAR);
	else if(strcmp(text,"double")==0)addTk(TYPE_DOUBLE);
	else if(strcmp(text,"else")==0)addTk(ELSE);
	else if(strcmp(text,"if")==0)addTk(IF);
	else if(strcmp(text,"int")==0)addTk(TYPE_INT);
	else if(strcmp(text,"return")==0)addTk(RETURN);
	else if(strcmp(text,"struct")==0)addTk(STRUCT);
	else if(strcmp(text,"void")==0)addTk(VOID);
	else if(strcmp(text,"while")==0)addTk(WHILE);
	//ID
	else{
	  tk=addTk(ID);
	  tk->text=text;
	}
      }
      else if(isdigit(*pch)) {
	int isInt = 1; //daca vom avea int punem 1, daca nu, punem 0 pt double
	while(isdigit(*pch)) pch++;
	if(*pch=='.') //avem double
	  {
	    isInt = 0;
	    pch++;
	    int valid=0;// 1 daca e nr real corespunzator, fara spatii dupa punct sau alte caractere care nu ar trebui sa fie
	    while (isdigit(*pch))
	      {
		pch++;
		valid=1;
	      }
	    if(valid==0)
	      {
		err("Invalid character: double invalid! Linia: %d", line);
	      }
	  }
	if('e'==*pch || 'E'==*pch) {
	  isInt = 0;
	  pch++;
	  if('+'==*pch || '-'==*pch) pch++;//vom avea nevoie de digit dupa +/-
	  if(!isdigit(*pch)) 
	    {
	      err("Invalid double: %s, linia %d\n", extract(start,pch),line);
	    }
	  while (isdigit(*pch)) pch++;
	}
	char* number = extract(start,pch);
	if(isInt) {
	  tk = addTk(INT);
	  tk->i = atoi(number);
	} else {
	  tk = addTk(DOUBLE);
	  tk->d = atof(number);
	}
      }
      else err("invalid char: %c - %d",*pch,*pch);
    }
  }
}

void showTokens(const Token *tokens){
  for(const Token *tk=tokens;tk;tk=tk->next){
    printf("%d\t%s",tk->line,ids[tk->code]);
    if(tk->code == ID || tk->code == STRING) {
      printf(":%s",tk->text);
    }
    if(tk->code == CHAR) {
      printf(":%c", tk->c);
    }
    if(tk->code == DOUBLE) {
      printf(":%.2f",tk->d);
    }
    if(tk->code == INT) {
      printf(":%d",tk->i);
    }
    printf("\n");
  }
}
