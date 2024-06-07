#ifndef EVAL_H
#define EVAL_H

#include "any_sexp.h"

any_sexp_t eval_symbol(const char *symbol, any_sexp_t env);

any_sexp_t eval_cons(any_sexp_cons_t *cons, any_sexp_t env);

any_sexp_t eval(any_sexp_t sexp, any_sexp_t env);

#endif
