#Modified from Neil's Makefile!!!
#in my codes, I had the interpreter code mixed-in with the parser code.
#This additional code is protected by #ifdef INTERP / #endif clauses.
#This code is only "seen" by the compiler if you have #defined INTERP
#This #define would be annoying to change inside nlab.c/nlab.h so inside it is
#set in the gcc/clang statement using -DINTERP
#in this way ./parse & ./interp can both be build form the same source file.

CFLAGS := -Wall -Wextra -Wpedantic -Wfloat-equal -Wvla -std=c99
SANI := -fsanitize=address -fsanitize=undefined
NLBS := $(wildcard *.nlb)
RESULTS := $(NLBS:.nlb=.result)

all : parse parse_s parse_v interp interp_s interp_v alltest allextension

allparse: parse parse_s parse_v
allinterp: interp interp_s interp_v
alltest: test test_s test_v
allextension: extension extension_s extension_v

#Used for my own test
test: func.c nlab.h test.c
		gcc test.c func.c $(CFLAGS) -O2 -DINTERP -o test -lm
test_s: func.c nlab.h test.c
		gcc test.c func.c $(CFLAGS) $(SANI) -g3 -DINTERP -o test_s -lm
test_v: func.c nlab.h test.c
		gcc test.c func.c $(CFLAGS) -g3 -DINTERP -o test_v -lm


interp: nlab.c nlab.h func.c
		gcc nlab.c func.c $(CFLAGS) -O2 -DINTERP -o interp -lm

interp_s: nlab.c nlab.h func.c
		gcc nlab.c func.c $(CFLAGS) $(SANI) -g3 -DINTERP -o interp_s -lm

interp_v: nlab.c nlab.h func.c
		gcc nlab.c func.c $(CFLAGS) -g3 -DINTERP -o interp_v -lm


parse: nlab.c nlab.h func.c
		gcc nlab.c func.c $(CFLAGS) -O2 -o parse -lm

parse_s: nlab.c nlab.h func.c
		gcc nlab.c func.c $(CFLAGS) $(SANI) -g3 -o parse_s -lm

parse_v: nlab.c nlab.h func.c
		gcc nlab.c func.c $(CFLAGS) -g3 -o parse_v -lm


extension: extension.h extension.c
		gcc extension.c $(CFLAGS) -O2 -o extension -lm

extension_s: extension.h extension.c
		gcc extension.c $(CFLAGS) $(SANI) -g3 -o extension_s -lm

extension_v: extension.h extension.c
		gcc extension.c $(CFLAGS) -g3 -o extension_v -lm


# For all .nlb files, run them and output result to a .result file
runall : $(RESULTS)

%.result:
		./interp_s $*.nlb > $*.result


# 'make clean' removes all executables
clean:
		rm -f parse parse_s parse_v interp interp_s interp_v test test_s test_v extension extension_s extension_v $(RESULTS)
