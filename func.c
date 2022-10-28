#include "nlab.h"

void* nfopen(char* fname, char* mode)
{
    FILE* fp;
    fp = fopen(fname, mode);
    if(!fp){
        fprintf(stderr, "Cannot read %s ?\n", fname);
        exit(EXIT_FAILURE);
    }
    return fp;
}

void* ncalloc(int n, size_t size)
{
   void* v = calloc(n, size);
   if(v==NULL){
      ERROR("Cannot calloc() space");
   }
   return v;
}

void Read_File(FILE* fp, Program* p)
{
    char str_tmp[BIGSTR];
    int i = 0;
    while(fscanf(fp, "%s", str_tmp) == 1 && i < MAXNUMTOKENS){
        if(strsame(str_tmp, "#")){
            assert(fgets(str_tmp, BIGSTR, fp));
        }
        else{
            strcpy(p->wds[i], str_tmp);
            i += 1;
        }
    }
    assert(i <= MAXNUMTOKENS);
    p->size = i;
}

// Used for test
void Print_Prog(Program* p)
{
    for(int i = 0; i < p->size; i++){
        printf("%s  ", p->wds[i]);
        if(strsame(p->wds[i], ";")){
            printf("\n");
        }
    }
    printf("\n");
}



void PROG(Program* p)
{
    if(! strsame(p->wds[p->cw], "BEGIN")){
        ERROR("No BEGIN statement ?\n");
    }
    p->cw += 1;
    if(! strsame(p->wds[p->cw], "{")){
        ERROR("No { after BEGIN ?\n");
    }

    p->cw += 1;
    INSTRCLIST(p);
}

void INSTRCLIST(Program *p)
{
    if(strsame(p->wds[p->cw], "}")){
        return;
    }
    INSTRC(p);

#ifdef INTERP
    // Check loop ?
    if(p->loop_stk->size != 0 && strsame(p->wds[p->cw+1], "}")){
        Var v_loop, v_idx, v_lp_start;
        assert(stack_pop(p->loop_stk, &v_loop));
        assert(stack_pop(p->loop_stk, &v_idx));
        assert(stack_peek(p->loop_stk, &v_lp_start));
        int loop = v_loop.arr[0][0];
        int idx = v_idx.arr[0][0];
        int lp_start = v_lp_start.arr[0][0];
        
        // Check counter is still scalar ?
        if(p->vtb[idx].row != SCALAR || p->vtb[idx].col != SCALAR){
            ERROR("Changing the LOOP counter to vector ?\n");
        }
        // Counter + 1
        p->vtb[idx].arr[0][0] += 1;
        
        // Counter <= loop ?
        if(p->vtb[idx].arr[0][0] <= loop){
            p->cw = lp_start;
            stack_push(p->loop_stk, v_idx);
            stack_push(p->loop_stk, v_loop);
        }
        else{
            assert(stack_pop(p->loop_stk, &v_lp_start));
            free_Var(&v_loop);
            free_Var(&v_idx);
            free_Var(&v_lp_start);
        }
    }
#endif

    p->cw += 1;
    INSTRCLIST(p);
}

void INSTRC(Program* p)
{
    if(strsame(p->wds[p->cw], "PRINT")){
        PRINT(p);
    }
    else if(strsame(p->wds[p->cw], "SET")){
        SET(p);
    }
    else if(strsame(p->wds[p->cw], "LOOP")){
        LOOP(p);
    }
    else if(strsame(p->wds[p->cw], "ONES") || strsame(p->wds[p->cw], "READ")){
        CREATE(p);
    }
    else{
        ERROR("Expecting an instruction ?\n");
    }
}


void PRINT(Program* p)
{
    assert(strsame(p->wds[p->cw], "PRINT"));
    p->cw += 1;
    if(is_VARNAME(p)){
#ifdef INTERP
        int idx = p->wds[p->cw][1] - 'A';
        if(p->vtb[idx].row == 0){
            ERROR("Using a variable before definition ?\n");
        }
        Print_Var(p->vtb[idx]);
#endif
        return;
    }

    else if(is_STRING(p)){
#ifdef INTERP
        char tmp[MAXTOKENSIZE];
        int l = strlen(p->wds[p->cw]);
        // Discard the " "
        memcpy(tmp, p->wds[p->cw]+(1*sizeof(char)), (l-2)*sizeof(char));
        tmp[l-2] = '\0';
        printf("%s\n", tmp);
#endif
        return;
    }

    ERROR("Expecting a VARNAME or a STRING after PRINT ?\n");   
}

void SET(Program* p)
{
    assert(strsame(p->wds[p->cw], "SET"));
    p->cw += 1;
    if(! is_VARNAME(p)){
        ERROR("Invalid parameters for SET ?\n");
    }
#ifdef INTERP
    int idx = p->wds[p->cw][1] - 'A';
    Var* v = Var_init(SCALAR, SCALAR, idx);
    stack_push(p->stk, *v);
    free(v);
#endif
    p->cw += 1;
    if(! strsame(p->wds[p->cw], ":=")){
        ERROR("Invalid parameters for SET ?\n");
    }
    p->cw += 1;
    POLISHLIST(p);
}

void LOOP(Program* p)
{
    assert(strsame(p->wds[p->cw], "LOOP"));
#ifdef INTERP
    int lp_start = p->cw + LOOPSTART;
    Var* v_tmp1 = Var_init(SCALAR, SCALAR, lp_start);
    stack_push(p->loop_stk, *v_tmp1);
    free(v_tmp1);
#endif
    p->cw += 1;
    if(! is_VARNAME(p)){
        ERROR("Invalid LOOP statement ?\n");
    }
#ifdef INTERP
    int idx = p->wds[p->cw][1] - 'A';
    Var* v_idx1 = Var_init(SCALAR, SCALAR, idx);
    Var* v_val1 = Var_init(SCALAR, SCALAR, ONES);
    Var_assign(v_idx1, v_val1, p->vtb);
    stack_push(p->loop_stk, *v_idx1);
    free(v_idx1);
    free(v_val1);
#endif
    p->cw += 1;
    if(! is_INTEGER(p)){
        ERROR("Invalid LOOP statement ?\n");
    }
#ifdef INTERP
    int loop;
    assert(sscanf(p->wds[p->cw], "%d", &loop));
    Var* v_tmp2 = Var_init(SCALAR, SCALAR, loop);
    stack_push(p->loop_stk, *v_tmp2);
    free(v_tmp2);
#endif
    p->cw += 1;
    if(! strsame(p->wds[p->cw], "{")){
        ERROR("Invalid LOOP statement ?\n");
    }
    p->cw += 1;
    INSTRCLIST(p);
}

void CREATE(Program* p)
{
    if(strsame(p->wds[p->cw], "ONES")){
        p->cw += 1;
        if(! is_INTEGER(p)){
            ERROR("Invalid parameters for ONES ?\n");
        }
#ifdef INTERP
        int row, col, idx;
        assert(sscanf(p->wds[p->cw], "%d", &row));
#endif
        p->cw += 1;
        if(! is_INTEGER(p)){
            ERROR("Invalid parameters for ONES ?\n");
        }
#ifdef INTERP
        assert(sscanf(p->wds[p->cw], "%d", &col));
#endif
        p->cw += 1;
        if(! is_VARNAME(p)){
            ERROR("Invalid parameters for ONES ?\n");
        }
#ifdef INTERP
        idx = p->wds[p->cw][1] - 'A';
        // An existing variable ?
        if(p->vtb[idx].row != 0){
            // Avoid memory leak by multiple assignments
            free_Var(&p->vtb[idx]);
        }
        Var* tmp = Var_init(row, col, ONES);
        memcpy(&p->vtb[idx], tmp, sizeof(*tmp));
        free(tmp);
#endif
    }

    else if(strsame(p->wds[p->cw], "READ")){
        p->cw += 1;
        if(! is_STRING(p)){
            ERROR("Invalid parameters for READ ?\n");
        }
#ifdef INTERP
        char fname[MAXTOKENSIZE];
        int l = strlen(p->wds[p->cw]);
        // Discard the " "
        memcpy(fname, p->wds[p->cw]+(1*sizeof(char)), (l-2)*sizeof(char));
        fname[l-2] = '\0';
#endif
        p->cw += 1;
        if(! is_VARNAME(p)){
            ERROR("Invalid parameters for READ ?\n");
        }
#ifdef INTERP
        int idx = p->wds[p->cw][1] - 'A';
        // An existing variable ?
        if(p->vtb[idx].row != 0){
            // Avoid memory leak by multiple assignments
            free_Var(&p->vtb[idx]);
        }
        Var_read(fname, p->vtb, idx);
#endif
    }
}

void POLISHLIST(Program* p)
{
    if(strsame(p->wds[p->cw], ";")){
#ifdef INTERP
        // Only one or no variable in the stack
        if(p->stk->size < VARANDVAL){
            ERROR("No enough variable for SET statement ?\n");
        }
        // One variable and one value in the stack
        else if(p->stk->size == VARANDVAL){
            Var v_val; 
            Var v_idx;
            assert(stack_pop(p->stk, &v_val));
            assert(stack_pop(p->stk, &v_idx));
            Var_assign(&v_idx, &v_val, p->vtb);
            free_Var(&v_idx);
        }
        else{
            ERROR("Too many variables for SET statement ?\n");
        }
#endif
        return;
    }
    POLISH(p);
    p->cw += 1;
    POLISHLIST(p);
}

void POLISH(Program* p)
{
    if(is_PUSHDOWN(p)){
        return;
    }
    else if(is_UNARYOP(p)){
        return;
    }
    else if(is_BINARYOP(p)){
        return;
    }
    ERROR("Invalid parameters for SET ?\n");
}


bool is_VARNAME(Program* p)
{
    if ( strlen(p->wds[p->cw]) != VARNAMELEN || 
            p->wds[p->cw][0] != '$' || 
            ! isupper(p->wds[p->cw][VARNAMELEN-1]) ){
        return false;
    }
    return true;
}

bool is_STRING(Program* p)
{
    int l = strlen(p->wds[p->cw]);
    if( p->wds[p->cw][0] != '"' || p->wds[p->cw][l-1] != '"' ){
        return false;
    }
    return true;
}

bool is_INTEGER(Program* p)
{
    int d;
    if(sscanf(p->wds[p->cw], "%d", &d) != 1){
        return false;
    }
    if(d < 0){
        return false;
    }
    return true;
}

bool is_PUSHDOWN(Program* p)
{
    if(is_VARNAME(p)){
#ifdef INTERP
        int idx, row, col;
        idx = p->wds[p->cw][1] - 'A';
        if(p->vtb[idx].row == 0){
            ERROR("Using a variable before definition ?\n");
        }
        row = p->vtb[idx].row;
        col = p->vtb[idx].col;
        Var* v = Var_init(row, col, ONES);
        for(int i = 0; i < row; i++){
            for(int j = 0; j < col; j++){
                v->arr[i][j] = p->vtb[idx].arr[i][j];
            }
        }
        stack_push(p->stk, *v);
        free(v);
#endif
        return true;
    }

    else if(is_INTEGER(p)){
#ifdef INTERP
        int num;
        assert(sscanf(p->wds[p->cw], "%d", &num));
        Var* v = Var_init(SCALAR, SCALAR, num);
        stack_push(p->stk, *v);
        free(v);
#endif
        return true;
    }
    return false;
}

bool is_UNARYOP(Program* p)
{
    if(strsame(p->wds[p->cw], "U-NOT")){
#ifdef INTERP
        if(p->stk->size < VARANDVAL){
            ERROR("No parameter for U-NOT ?\n");
        }
        Var v_val;
        int row, col;
        assert(stack_pop(p->stk, &v_val));
        row = v_val.row;
        col = v_val.col;
        for(int i = 0; i < row; i++){
            for(int j = 0; j < col; j++){
                if(v_val.arr[i][j] == 0){
                    v_val.arr[i][j] = 1;
                }
                else if(v_val.arr[i][j] == 1){
                    v_val.arr[i][j] = 0;
                }
            }
        }
        stack_push(p->stk, v_val);
#endif
        return true;
    }
    
    else if(strsame(p->wds[p->cw], "U-EIGHTCOUNT")){
#ifdef INTERP
        if(p->stk->size < VARANDVAL){
            ERROR("No parameter for U-EIGHTCOUNT ?\n");
        }
        Var v_val;
        int row, col;
        assert(stack_pop(p->stk, &v_val));
        row = v_val.row;
        col = v_val.col;
        Var* tmp = Var_init(row, col, ONES);
        for(int i = 0; i < row; i++){
            for(int j = 0; j < col; j++){
                tmp->arr[i][j] = Ueightcount(v_val, i, j);
            }
        }
        stack_push(p->stk, *tmp);
        free_Var(&v_val);
        free(tmp);
#endif
        return true;
    }
    return false;
}

bool is_BINARYOP(Program* p)
{
    if(strsame(p->wds[p->cw], "B-AND")){
#ifdef INTERP
        if(p->stk->size < VARANDTWOVAL){
            ERROR("No enough variable for B-AND ?\n");
        }
        B_binaryop(p->stk, "AND");
#endif
        return true;
    }
    else if(strsame(p->wds[p->cw], "B-OR")){
#ifdef INTERP
        if(p->stk->size < VARANDTWOVAL){
            ERROR("No enough variable for B-OR ?\n");
        }
        B_binaryop(p->stk, "OR");
#endif
        return true;
    }
    else if(strsame(p->wds[p->cw], "B-GREATER")){
#ifdef INTERP
        if(p->stk->size < VARANDTWOVAL){
            ERROR("No enough variable for B-GREATER ?\n");
        }
        B_binaryop(p->stk, "GREATER");
#endif
        return true;
    }
    else if(strsame(p->wds[p->cw], "B-LESS")){
#ifdef INTERP
        if(p->stk->size < VARANDTWOVAL){
            ERROR("No enough variable for B-LESS ?\n");
        }
        B_binaryop(p->stk, "LESS");
#endif
        return true;
    }
    else if(strsame(p->wds[p->cw], "B-ADD")){
#ifdef INTERP
        if(p->stk->size < VARANDTWOVAL){
            ERROR("No enough variable for B-ADD ?\n");
        }
        B_binaryop(p->stk, "ADD");
#endif
        return true;
    }
    else if(strsame(p->wds[p->cw], "B-TIMES")){
#ifdef INTERP
        if(p->stk->size < VARANDTWOVAL){
            ERROR("No enough variable for B-TIMES ?\n");
        }
        B_binaryop(p->stk, "TIMES");
#endif
        return true;
    }
    else if(strsame(p->wds[p->cw], "B-EQUALS")){
#ifdef INTERP
        if(p->stk->size < VARANDTWOVAL){
            ERROR("No enough variable for B-EQUALS ?\n");
        }
        B_binaryop(p->stk, "EQUALS");
#endif
        return true;
    }
    return false;
}



#ifdef INTERP
void** n2dcalloc(int h, int w, size_t szelem)
{
    int i;
    void** p;
    p = calloc(h, sizeof(void*));
    if(p == NULL){
        ERROR("Cannot calloc() space");
    }
    for(i = 0; i < h; i++){
        p[i] = calloc(w, szelem);
        if(p[i] == NULL){
            ERROR("Cannot calloc() space");
        }
    }
    return p;
}

void n2dfree(void**p, int h)
{
    if(p){
        int i;
        for(i = 0; i < h; i++){
            free(p[i]);
        }
        free(p);
    }
}

Var* Var_init(int row, int col, int n)
{
    Var* v = (Var*) ncalloc(1, sizeof(Var));
    v->row = row;
    v->col = col;
    v->arr = (int**) n2dcalloc(row, col, sizeof(int));
    for(int i = 0; i < v->row; i++){
        for(int j = 0; j < v->col; j++){
            v->arr[i][j] = n;
        }
    }
    return v;
}

void Var_read(char* fname, Var* vtb, int idx)
{
    FILE* fp = nfopen(fname, "rt");
    int row, col, d;
    if(fscanf(fp, "%d %d", &row, &col) != 2 || row < 0 || col < 0){
        ERROR("Invalid .arr file ?\n");
    }
    Var* v = Var_init(row, col, ONES);
    for(int i = 0; i < row; i++){
        for(int j = 0; j < col; j++){
            if(fscanf(fp, "%d", &v->arr[i][j]) != 1 ||
                                    v->arr[i][j] < 0){
                ERROR("Invalid .arr file ?\n");
            }
        }
    }
    if(fscanf(fp, "%d", &d) == 1){
        ERROR("Invalid .arr file ?\n");
    }
    fclose(fp);
    memcpy(&vtb[idx], v, sizeof(*v));
    free(v);
}

void Var_assign(Var* v_idx, Var* v_val, Var* vtb)
{
    int idx = v_idx->arr[0][0];
    // An existing variable ?
    if(vtb[idx].row != 0){
        // Avoid memory leak by multiple assignments
        free_Var(&vtb[idx]);
    }
    vtb[idx] = *v_val;
}

// Used for test
void Print_Var(const Var v)
{
    for(int i = 0; i < v.row; i++){
        for(int j = 0; j < v.col; j++){
            printf("%d ", v.arr[i][j]);
        }
        printf("\n");
    }
}


bool free_Var(Var* v)
{
    if(v){
        n2dfree((void**)v->arr, v->row);
    }
    return true;
}

bool free_Vtb(Var* vtb)
{
    if(vtb){
        int i;
        for(i = 0; i < NUMVARIABLE; i++){
            if(vtb[i].row !=  0){
                n2dfree((void**) vtb[i].arr, vtb[i].row);
            }
        }
        free(vtb);
    }
    return true;
}

int Ueightcount(Var v, int i, int j)
{
    int cnt = 0;
    for(int y = i-1; y <= i+1; y++){
        for(int x = j-1; x <= j+1; x++){
            if(y >= 0 && y < v.row && x >= 0 && x < v.col){
                if(v.arr[y][x] > 0){
                    cnt += 1;
                }
            }
        }
    }
    if(v.arr[i][j] > 0){
        cnt -= 1;
    }
    return cnt;
}


void B_binaryop(stack* s, char* func)
{
    Var val1, val2;
    assert(stack_pop(s, &val2));
    assert(stack_pop(s, &val1));
    if(val2.row == 1 && val2.col == 1){
        B_scalar(val2, val1, s, func);
    }
    else if(val1.row == 1 && val1.col ==1){
        B_scalar_2(val1, val2, s, func);
    }
    else if(val1.row == val2.row && val1.col == val2.col){
        B_matrix(val1, val2, s, func);
    }
    else{
        ERROR("Operating two matrix of different size ?\n");
    }
}

void B_scalar(Var v_sca, Var v_mat, stack* s, char* func)
{
    int sca = v_sca.arr[0][0];
    for(int i = 0; i < v_mat.row; i++){
        for(int j = 0; j < v_mat.col; j++){
            v_mat.arr[i][j] = B_operating(v_mat.arr[i][j], sca, func);
        }
    }
    stack_push(s, v_mat);
    free_Var(&v_sca);
}

void B_scalar_2(Var v_sca, Var v_mat, stack* s, char* func)
{
    int sca = v_sca.arr[0][0];
    for(int i = 0; i < v_mat.row; i++){
        for(int j = 0; j < v_mat.col; j++){
            v_mat.arr[i][j] = B_operating(sca, v_mat.arr[i][j], func);
        }
    }
    stack_push(s, v_mat);
    free_Var(&v_sca);
}

void B_matrix(Var val1, Var val2, stack* s, char* func)
{
    for(int i = 0; i < val2.row; i++){
        for(int j = 0; j < val2.col; j++){
            val2.arr[i][j] = B_operating(val1.arr[i][j], val2.arr[i][j], func);
        }
    }
    stack_push(s, val2);
    free_Var(&val1);
}

int B_operating(int v1, int v2, char* func)
{
    int result = NORESULT;
    if(strsame(func, "AND")){
        bool b = v1 && v2;
        result = (int) b;
    }
    else if(strsame(func, "OR")){
        bool b = v1 || v2;
        result = (int) b;
    }
    else if(strsame(func, "GREATER")){
        bool b = v1 > v2;
        result = (int) b;
    }
    else if(strsame(func, "LESS")){
        bool b = v1 < v2;
        result = (int) b;
    }
    else if(strsame(func, "ADD")){
        result = v1 + v2;
    }
    else if(strsame(func, "TIMES")){
        result = v1 * v2;
    }
    else if(strsame(func, "EQUALS")){
        bool b = v1 == v2;
        result = (int) b;
    }
    return result;
}
#endif


#ifdef INTERP
// Modified from Neil's ADT-Stack!!!
/* Create an empty stack */
stack* stack_init(void)
{
   stack* s = (stack*) ncalloc(1, sizeof(stack));
   return s;
}

/* Add element to top */
void stack_push(stack* s, stacktype d)
{
   if(s){
      dataframe* f = ncalloc(1, sizeof(dataframe));
      f->i = d;
      f->next = s->start;
      s->start = f;
      s->size = s->size + 1;
   }
}

/* Take element from top */
bool stack_pop(stack* s, stacktype* d)
{
   if((s == NULL) || (s->start == NULL)){
      return false;
   }

   dataframe* f = s->start->next;
   *d = s->start->i;
   free(s->start);
   s->start = f;
   s->size = s->size - 1;
   return true;
}

/* Copy top element into d (but don't pop it) */
bool stack_peek(stack*s,  stacktype* d)
{
   if((s == NULL) || (s->start == NULL)){
      return false;
   }
   *d = s->start->i;
   return true;
}

/* Clears all space used */
bool stack_free(stack* s)
{
   if(s){
      dataframe* p = s->start;
      while(p != NULL){
         dataframe* tmp = p->next;
         free(p);
         p = tmp;
      }
      free(s);
   }
   return true;
}
#endif
