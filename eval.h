#ifndef EVAL_H
#define EVAL_H

#include "any_sexp.h"

any_sexp_t eval_get_fvs(any_sexp_t sexp, any_sexp_t pars);

any_sexp_t eval_symbol(const char *symbol, any_sexp_t env);

any_sexp_t eval_cons(any_sexp_t sexp, any_sexp_t env);

any_sexp_t eval_list(any_sexp_t sexp, any_sexp_t env);

any_sexp_t eval(any_sexp_t sexp, any_sexp_t env);

any_sexp_t eval_macro(any_sexp_t sexp, any_sexp_t env, any_sexp_t menv);

void eval_change_env(any_sexp_t symbol, any_sexp_t value, any_sexp_t env, any_sexp_t *envptr);

any_sexp_t eval_define(any_sexp_t sexp, any_sexp_t *env, any_sexp_t *menv);

any_sexp_t eval_file(FILE *file, any_sexp_t *env, any_sexp_t *menv);

void eval_init();

#endif
