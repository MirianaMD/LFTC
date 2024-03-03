#include <stdio.h>
#include <stdlib.h>

#include "lexer.h"
#include "utils.h"

int main()
{
    char *inbuf=loadFile("tests/testlex.c");
    //puts(inbuf);
    Token *tokens=tokenize(inbuf);
    free(inbuf);
    showTokens(tokens);
    return 0;
}