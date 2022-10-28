#include "nlab.h"

int main(int argc, char* argv[])
{
    if(argc != 2){
        fprintf(stderr, "Usage: %s <file>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    FILE* fp = nfopen(argv[1], "rt");

    Program* prog = (Program*) ncalloc(1, sizeof(Program));
    Read_File(fp, prog);
    fclose(fp);

#ifdef INTERP
    Var* Vtb = (Var*) ncalloc(NUMVARIABLE, sizeof(Var));
    prog->vtb = Vtb;
    stack* s = stack_init();
    prog->stk = s;
    stack* loop_stk = stack_init();
    prog->loop_stk = loop_stk; 
#endif

    PROG(prog);
    free(prog);

#ifdef INTERP
    assert(free_Vtb(Vtb));
    assert(stack_free(s));
    assert(stack_free(loop_stk));
#endif

    return 0;
}
