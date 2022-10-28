#pragma once

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>
#include <ctype.h>

#define strsame(A, B) (strcmp(A, B) == 0)
#define ERROR(PHRASE) {fprintf(stderr, "%s", PHRASE);\
                        exit(EXIT_FAILURE);}
#define MAXTOKENSIZE 50
#define MAXNUMTOKENS 500
#define BIGSTR 500
#define NUMVARIABLE 26
#define ONES 1
#define SCALAR 1
#define VARNAMELEN 2
#define VARANDVAL 2
#define VARANDTWOVAL 3
#define LOOPSTART 3
#ifdef INTERP
#define STACKTYPE "Linked"
#define NORESULT -1
#endif

#ifdef INTERP
struct variable {
   int** arr;
   int row;
   int col;
};
typedef struct variable Var;

typedef Var stacktype;

struct dataframe {
   stacktype i;
   struct dataframe* next;
};
typedef struct dataframe dataframe;

struct stack {
   dataframe* start;
   int size;
};
typedef struct stack stack;
#endif

struct prog {
   char wds[MAXNUMTOKENS][MAXTOKENSIZE];
   int cw; // Current Word
   int size;
#ifdef INTERP
   Var* vtb;
   stack* stk;
   stack* loop_stk;
#endif
};
typedef struct prog Program;


void* nfopen(char* fname, char* mode);
void* ncalloc(int n, size_t size);
void Read_File(FILE* fp, Program* p);
// Used for test
void Print_Prog(Program* p);

void PROG(Program* p);
void INSTRCLIST(Program *p);
void INSTRC(Program* p);

void PRINT(Program* p);
void SET(Program* p);
void LOOP(Program* p);
void CREATE(Program* p);
void POLISHLIST(Program* p);
void POLISH(Program* p);

bool is_VARNAME(Program* p);
bool is_STRING(Program* p);
bool is_INTEGER(Program* p);
bool is_PUSHDOWN(Program* p);
bool is_UNARYOP(Program* p);
bool is_BINARYOP(Program* p);

#ifdef INTERP
void** n2dcalloc(int h, int w, size_t szelem);
void n2dfree(void**p, int h);
Var* Var_init(int row, int col, int n);
void Var_read(char* fname, Var* vtb, int idx);
void Var_assign(Var* v_idx, Var* v_val, Var* vtb);
// Used for test
void Print_Var(const Var v);

bool free_Var(Var* v);
bool free_Vtb(Var* v);
int Ueightcount(Var v, int i, int j);

void B_binaryop(stack* s, char* func);
void B_scalar(Var v_sca, Var v_mat, stack* s, char* func);
void B_scalar_2(Var v_sca, Var v_mat, stack* s, char* func);
void B_matrix(Var val1, Var val2, stack* s, char* func);
int B_operating(int v1, int v2, char* func);
#endif

#ifdef INTERP
// Modified from Neil's ADT-Stack!!!
/* Create an empty stack */
stack* stack_init(void);
/* Add element to top */
void stack_push(stack* s, stacktype i);
/* Take element from top */
bool stack_pop(stack* s, stacktype* d);
/* Copy top element into d (but don't pop it) */
bool stack_peek(stack*s,  stacktype* d);
/* Clears all space used */
bool stack_free(stack* s);
#endif
