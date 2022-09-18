#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

extern FILE* logger;

#define ASSERT_EXIT(_var, _val) if ((_var) _val){perror(#_var); exit(-1);}
    
#endif
