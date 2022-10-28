#include "nlab.h"

/* TEST[0] is for is_VARNAME() & is_STRING() & is_INTEGER()
                & free_Var() & free_Vtb() & B_operating() */
/* TEST[1] is for is_PUSHDOWN() & is_UNARYOP() & is_BINARYOP()
                & Var_init() & Var_read() & Ueightcount() 
                & Var_assign() & B_binaryop() */
/* TEST[2] is for POLISH() */
/* TEST[3] is for B_scalar() & B_scalar_2() & B_matrix() */

int main(void)
{
    bool TEST[4] = {true, true, true, true};
    Program* p = (Program*) ncalloc(1, sizeof(Program));
#ifdef INTERP
    TEST[2] = false;
    Var* Vtb = (Var*) ncalloc(NUMVARIABLE, sizeof(Var));
    p->vtb = Vtb;
    stack* s = stack_init();
    p->stk = s;
    stack* l_stk = stack_init();
    p->loop_stk = l_stk;   
#endif

    if(TEST[0]){
        strcpy(p->wds[100],"$SN");
        p->cw = 100;
        assert(! is_VARNAME(p));
        assert(! is_STRING(p));
        assert(! is_INTEGER(p));
        strcpy(p->wds[100],"10");
        assert(! is_VARNAME(p));
        assert(! is_STRING(p));
        assert(is_INTEGER(p));
        strcpy(p->wds[100],"$a");
        assert(! is_VARNAME(p));
        assert(! is_STRING(p));
        strcpy(p->wds[100],"$S");
        assert(is_VARNAME(p));
        assert(! is_STRING(p));
        assert(! is_INTEGER(p));  
        strcpy(p->wds[100],"\"$S\"");
        assert(! is_VARNAME(p));
        assert(is_STRING(p));
        strcpy(p->wds[100],"-1");
        assert(! is_VARNAME(p));
        assert(! is_INTEGER(p));
#ifdef INTERP
        assert(free_Var(NULL));
        assert(free_Vtb(NULL));
        assert(B_operating(2, 3, "AND") == 1);
        assert(B_operating(2, 3, "EQUALS") == 0);
        assert(B_operating(2, 3, "TIMES") == 6);
        assert(B_operating(2, 3, "AD") == NORESULT);
#endif
        printf("TEST0 passed\n");
    }
    if(TEST[1]){
#ifdef INTERP
        Var* tmp1 = Var_init(SCALAR, SCALAR, 10);
        Vtb[0] = *tmp1;
        free(tmp1);
        assert(Vtb['A'-'A'].row == 1);
        assert(Vtb['A'-'A'].col == 1);
        assert(Vtb['A'-'A'].arr[0][0] == 10);
        Var_read("$B.arr", Vtb, 1);
        /* Use this line for check
        printf("$B:\n"); Print_Var(Vtb[1]); */
        assert(Vtb['B'-'A'].row == 3);
        assert(Vtb['B'-'A'].col == 3);
        assert(Vtb['B'-'A'].arr[0][0] == 0);
        assert(Vtb['B'-'A'].arr[2][1] == 1);
        assert(Ueightcount(Vtb[1], 1, 1) == 2);
        assert(Ueightcount(Vtb[1], 0, 0) == 0);
#endif
        strcpy(p->wds[100],"$A");
        p->cw = 100;
        assert(is_PUSHDOWN(p));
        assert(! is_UNARYOP(p));
        assert(! is_BINARYOP(p));
#ifdef INTERP
        Var tmp_pop;
        assert(stack_pop(s, &tmp_pop));
        assert(tmp_pop.arr[0][0] == 10);
        free_Var(&tmp_pop);
#endif

        strcpy(p->wds[100],"240");
        assert(is_PUSHDOWN(p));
        assert(! is_UNARYOP(p));
        assert(! is_BINARYOP(p));
#ifdef INTERP
        assert(stack_pop(s, &tmp_pop));
        assert(tmp_pop.arr[0][0] == 240);
        free_Var(&tmp_pop);
#endif

#ifdef INTERP    
        Var* tmp2 = Var_init(SCALAR, SCALAR, 'C'-'A');
        stack_push(s, *tmp2);
        Var* tmp3 = Var_init(Vtb[1].row, Vtb[1].col, ONES);
        for(int i = 0; i < Vtb[1].row; i++){
            for(int j = 0; j < Vtb[1].col; j++){
                tmp3->arr[i][j] = Vtb[1].arr[i][j];
            }
        }
        stack_push(s, *tmp3);
        free(tmp2);
        free(tmp3);
#endif      
        strcpy(p->wds[100],"U-EIGHTCOUNT");
        assert(! is_PUSHDOWN(p));
        assert(is_UNARYOP(p));
        assert(! is_BINARYOP(p));
#ifdef INTERP    
        assert(stack_peek(s, &tmp_pop));
        /* Use this line for check
        Print_Var(tmp_pop); */
        assert(tmp_pop.arr[0][1] == 0);
        assert(tmp_pop.arr[2][0] == 1);
#endif

#ifdef INTERP         
        Var* tmp4 = Var_init(3, 3, 2);
        stack_push(s, *tmp4);
        free(tmp4);
#endif
        strcpy(p->wds[100],"B-TIMES");
        assert(! is_PUSHDOWN(p));
        assert(! is_UNARYOP(p));
        assert(is_BINARYOP(p));
#ifdef INTERP    
        assert(stack_peek(s, &tmp_pop));
        /* Use this line for check
        Print_Var(tmp_pop); */
        assert(tmp_pop.arr[0][1] == 0);
        assert(tmp_pop.arr[1][1] == 4);
        assert(tmp_pop.arr[2][0] == 2);
#endif

        
#ifdef INTERP         
        Var* tmp5 = Var_init(SCALAR, SCALAR, 3);
        stack_push(s, *tmp5);
        free(tmp5);
#endif
        strcpy(p->wds[100],"B-ADD");
        assert(! is_PUSHDOWN(p));
        assert(! is_UNARYOP(p));
        assert(is_BINARYOP(p));
#ifdef INTERP    
        assert(stack_peek(s, &tmp_pop));
        /* Use this line for check
        Print_Var(tmp_pop); */
        assert(tmp_pop.arr[0][1] == 3);
        assert(tmp_pop.arr[1][1] == 7);
        assert(tmp_pop.arr[2][0] == 5);
#endif

        strcpy(p->wds[100],"\"B-OR\"");
        assert(! is_PUSHDOWN(p));
        assert(! is_UNARYOP(p));
        assert(! is_BINARYOP(p));
#ifdef INTERP
        assert(stack_pop(s, &tmp_pop));
        Var tmp_pop2;
        assert(stack_pop(s, &tmp_pop2));
        Var_assign(&tmp_pop2, &tmp_pop, Vtb);
        assert(Vtb['C'-'A'].arr[0][1] == 3);
        assert(Vtb['C'-'A'].arr[1][1] == 7);
        assert(Vtb['C'-'A'].arr[2][0] == 5);
        free_Var(&tmp_pop2);
#endif
        printf("TEST1 passed\n");
    }
    if(TEST[2]){
        strcpy(p->wds[100],"66");
        p->cw = 100;
        POLISH(p);
        strcpy(p->wds[100],"U-NOT");
        POLISH(p);
        strcpy(p->wds[100],"B-GREATER");
        POLISH(p);
        printf("TEST2 passed\n");
    }

#ifdef INTERP
    if(TEST[3]){
        /* Use these lines for check
        printf("$A:\n"); Print_Var(Vtb[0]);
        printf("$B:\n"); Print_Var(Vtb[1]);
        printf("$C:\n"); Print_Var(Vtb[2]); */
        Var tmp_t3_pop, tmp_t3_pop2;
        Var* tmp_t3_1 = Var_init(SCALAR, SCALAR, 5);
        Var* tmp_t3_2 = Var_init(Vtb[2].row, Vtb[2].col, ONES);
        for(int i = 0; i < Vtb[2].row; i++){
            for(int j = 0; j < Vtb[2].col; j++){
                tmp_t3_2->arr[i][j] = Vtb[2].arr[i][j];
            }
        }
        B_scalar(*tmp_t3_1, *tmp_t3_2, s, "GREATER");
        free(tmp_t3_1); free(tmp_t3_2);
        assert(stack_peek(s, &tmp_t3_pop));
        assert(tmp_t3_pop.arr[0][1] == 0);
        assert(tmp_t3_pop.arr[1][1] == 1);
        assert(tmp_t3_pop.arr[2][0] == 0);

        Var* tmp_t3_3 = Var_init(SCALAR, SCALAR, 5);
        Var* tmp_t3_4 = Var_init(Vtb[2].row, Vtb[2].col, ONES);
        for(int i = 0; i < Vtb[2].row; i++){
            for(int j = 0; j < Vtb[2].col; j++){
                tmp_t3_4->arr[i][j] = Vtb[2].arr[i][j];
            }
        }
        B_scalar_2(*tmp_t3_3, *tmp_t3_4, s, "GREATER");
        free(tmp_t3_3); free(tmp_t3_4);
        assert(stack_peek(s, &tmp_t3_pop));
        assert(tmp_t3_pop.arr[0][1] == 1);
        assert(tmp_t3_pop.arr[1][1] == 0);
        assert(tmp_t3_pop.arr[2][0] == 0);

        assert(stack_pop(s, &tmp_t3_pop2));
        assert(stack_pop(s, &tmp_t3_pop));
        /* Use this line for check
        Print_Var(tmp_t3_pop); Print_Var(tmp_t3_pop2); */
        B_matrix(tmp_t3_pop, tmp_t3_pop2, s, "ADD");
        assert(stack_pop(s, &tmp_t3_pop));
        assert(tmp_t3_pop.arr[0][1] == 1);
        assert(tmp_t3_pop.arr[1][1] == 1);
        assert(tmp_t3_pop.arr[2][0] == 0);
        free_Var(&tmp_t3_pop);
        printf("TEST3 passed\n");
    } 
#endif

#ifdef INTERP
    assert(free_Vtb(Vtb));
    assert(stack_free(s));
    assert(stack_free(l_stk));
#endif
    free(p);


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
/*                      Second Part                               */
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
    
    Program* p2 = (Program*) ncalloc(1, sizeof(Program));
    FILE* fp = nfopen("test.nlb", "rt");
    Read_File(fp, p2);
    fclose(fp);
    /* Use this line for check
    Print_Prog(p2); */
#ifdef INTERP
    Var* Vtb2 = (Var*) ncalloc(NUMVARIABLE, sizeof(Var));
    p2->vtb = Vtb2;
    stack* s2 = stack_init();
    p2->stk = s2;
    stack* l_stk2 = stack_init();
    p2->loop_stk = l_stk2;
#endif
    PROG(p2);
    free(p2);
#ifdef INTERP
    assert(free_Vtb(Vtb2));
    assert(stack_free(s2));
    assert(stack_free(l_stk2));
#endif
    return 0;
}
