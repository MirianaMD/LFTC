#include <stdio.h>
#include <stdlib.h>

#include "lexer.h"
#include "utils.h"
#include "parser.h"

int main()
{
  //char *inbuf=loadFile("tests/testlex.c");
  char* inbuf=loadFile("tests/testparser.c");
  //puts(inbuf); //de aici afiseaza testparser.c
  Token* parselist = tokenize(inbuf);
  showTokens(parselist);
  parse(parselist);
  return 0;
}
