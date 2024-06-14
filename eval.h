#ifndef EVAL_H
#define EVAL_H

#include "any_sexp.h"

#define CAR(l)   (any_sexp_car(l))
#define CDR(l)   (any_sexp_cdr(l))
#define CAAR(l)  (CAR(CAR(l)))
#define CADR(l)  (CAR(CDR(l)))
#define CDDR(l)  (CDR(CDR(l)))
#define CADDR(l) (CAR(CDR(CDR(l))))

#define T (any_sexp_number(1))

typedef any_sexp_t (*eval_primitive_t)(any_sexp_t a, any_sexp_t b);

any_sexp_t eval_primitive(any_sexp_t sexp, any_sexp_t env, eval_primitive_t prim);

any_sexp_t eval_get_fvs(any_sexp_t sexp, any_sexp_t pars);

any_sexp_t eval_symbol(const char *symbol, any_sexp_t env);

any_sexp_t eval_cons(any_sexp_t sexp, any_sexp_t env);

any_sexp_t eval_list(any_sexp_t sexp, any_sexp_t env);

any_sexp_t eval(any_sexp_t sexp, any_sexp_t env);

any_sexp_t eval_macro(any_sexp_t sexp, any_sexp_t env, any_sexp_t menv);

void eval_change_env(any_sexp_t symbol, any_sexp_t value, any_sexp_t *env);

any_sexp_t eval_define(any_sexp_t sexp, any_sexp_t *env, any_sexp_t *menv);

any_sexp_t eval_file(FILE *file, any_sexp_t *env, any_sexp_t *menv);

void eval_init();

#endif
