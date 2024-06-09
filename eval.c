#include <string.h>
#include <stdio.h>

#include "eval.h"
#include "any_log.h"

#define ANY_SEXP_IMPLEMENT
#include "any_sexp.h"

static any_sexp_t primitives = ANY_SEXP_NIL;

// Environment
//
// ((symbol value) (symbol value) ...)

any_sexp_t eval_find_symbol(const char *symbol, any_sexp_t env)
{
    if (ANY_SEXP_IS_NIL(env))
        return ANY_SEXP_ERROR;

    any_sexp_t car = any_sexp_car(env);
    if (!strcmp(symbol, ANY_SEXP_GET_SYMBOL(any_sexp_car(car))))
        return any_sexp_cdr(car);

    return eval_find_symbol(symbol, any_sexp_cdr(env));
}

any_sexp_t eval_symbol(const char *symbol, any_sexp_t env)
{
    any_sexp_t value = eval_find_symbol(symbol, env);

    if (ANY_SEXP_IS_ERROR(value))
        log_error("Symbol %s not bound in scope", symbol);

    return value;
}

bool eval_is_symbol_list(any_sexp_t sexp)
{
    if (ANY_SEXP_IS_NIL(sexp))
        return true;

    if (!ANY_SEXP_IS_CONS(sexp))
        return false;

    return ANY_SEXP_IS_SYMBOL(any_sexp_car(sexp))
        && eval_is_symbol_list(any_sexp_cdr(sexp));
}

bool eval_is_lambda(any_sexp_t sexp)
{
    any_sexp_t car = any_sexp_car(sexp);
    any_sexp_t cadr = any_sexp_car(any_sexp_cdr(sexp));
    any_sexp_t cddr = any_sexp_cdr(any_sexp_cdr(sexp));
    any_sexp_t caddr = any_sexp_car(cddr);

    return ANY_SEXP_IS_CONS(sexp)
        && ANY_SEXP_IS_SYMBOL(car)
        && !strcmp(ANY_SEXP_GET_SYMBOL(car), "lambda")
        && ANY_SEXP_IS_CONS(cadr)
        && ANY_SEXP_IS_CONS(cddr)
        && ANY_SEXP_IS_NIL(any_sexp_cdr(cddr))
        && eval_is_symbol_list(cadr);
}

bool eval_is_let(any_sexp_t sexp)
{
    // (cons 'let (cons (cons id (cons value nil)) (cons body nil)))
    //
    any_sexp_t car = any_sexp_car(sexp);
    any_sexp_t cadr = any_sexp_car(any_sexp_cdr(sexp));
    any_sexp_t cddr = any_sexp_cdr(any_sexp_cdr(sexp));
    any_sexp_t caddr = any_sexp_car(cddr);

    return ANY_SEXP_IS_CONS(sexp)
        && ANY_SEXP_IS_SYMBOL(car)
        && !strcmp(ANY_SEXP_GET_SYMBOL(car), "let")
        && ANY_SEXP_IS_CONS(cddr)
        && ANY_SEXP_IS_NIL(any_sexp_cdr(cddr))
        && ANY_SEXP_IS_CONS(cadr)
        && ANY_SEXP_IS_SYMBOL(any_sexp_car(cadr))
        && ANY_SEXP_IS_NIL(any_sexp_cdr(any_sexp_cdr(cadr)));
}

bool eval_is_quote(any_sexp_t sexp)
{
    // (cons 'quote (cons x nil))
    //
    any_sexp_t car = any_sexp_car(sexp);
    any_sexp_t cdr = any_sexp_cdr(sexp);

    return ANY_SEXP_IS_CONS(sexp)
        && ANY_SEXP_IS_SYMBOL(car)
        && !strcmp(ANY_SEXP_GET_SYMBOL(car), "quote")
        && ANY_SEXP_IS_CONS(cdr)
        && ANY_SEXP_IS_NIL(any_sexp_cdr(cdr));
}

bool eval_find_fvs(const char *symbol, any_sexp_t fvs)
{
    if (ANY_SEXP_IS_NIL(fvs))
        return false;

    any_sexp_t car = any_sexp_car(fvs);

    if (!ANY_SEXP_IS_SYMBOL(car))
        log_panic("Invalid fvs passed");

    return !strcmp(symbol, ANY_SEXP_GET_SYMBOL(car))
        || eval_find_fvs(symbol, any_sexp_cdr(fvs));
}

any_sexp_t eval_merge_fvs(any_sexp_t a, any_sexp_t b)
{
    if (ANY_SEXP_IS_NIL(a))
        return b;

    if (!ANY_SEXP_IS_CONS(a))
        return ANY_SEXP_ERROR;

    if (eval_find_fvs(ANY_SEXP_GET_SYMBOL(any_sexp_car(a)), b))
        return eval_merge_fvs(any_sexp_cdr(a), b);

    return any_sexp_cons(any_sexp_car(a), eval_merge_fvs(any_sexp_cdr(a), b));
}

any_sexp_t eval_get_fvs(any_sexp_t sexp, any_sexp_t pars)
{
    if (ANY_SEXP_IS_SYMBOL(sexp)) {
        const char *symbol = ANY_SEXP_GET_SYMBOL(sexp);
        return eval_find_fvs(symbol, pars) || eval_find_fvs(symbol, primitives)
             ? ANY_SEXP_NIL
             : any_sexp_cons(sexp, ANY_SEXP_NIL);
    }

    if (ANY_SEXP_IS_CONS(sexp)) {

        // Update parameters
        //
        if (eval_is_lambda(sexp)) {
            any_sexp_t sub_pars = any_sexp_car(any_sexp_cdr(sexp));
            any_sexp_t body = any_sexp_car(any_sexp_cdr(any_sexp_cdr(sexp)));

            return eval_get_fvs(body, eval_merge_fvs(sub_pars, pars));
        }

        if (eval_is_let(sexp)) {
            any_sexp_t name = any_sexp_car(any_sexp_car(any_sexp_cdr(sexp)));
            any_sexp_t body = any_sexp_car(any_sexp_cdr(any_sexp_cdr(sexp)));

            return eval_get_fvs(body, eval_merge_fvs(any_sexp_cons(name, ANY_SEXP_NIL), pars));
        }

        if (eval_is_quote(sexp))
            return ANY_SEXP_NIL;

        any_sexp_t car = eval_get_fvs(any_sexp_car(sexp), pars);
        any_sexp_t cdr = eval_get_fvs(any_sexp_cdr(sexp), pars);

        return ANY_SEXP_IS_ERROR(car) || ANY_SEXP_IS_ERROR(cdr)
             ? ANY_SEXP_ERROR
             : eval_merge_fvs(car, cdr);
    }

    return ANY_SEXP_NIL;
}

any_sexp_t eval_copy_fvs(any_sexp_t fvs, any_sexp_t env)
{
    if (ANY_SEXP_IS_NIL(fvs))
        return ANY_SEXP_NIL;

    if (!ANY_SEXP_IS_CONS(fvs) || !ANY_SEXP_IS_SYMBOL(any_sexp_car(fvs)))
        return ANY_SEXP_ERROR;

    any_sexp_t sexp = eval_symbol(ANY_SEXP_GET_SYMBOL(any_sexp_car(fvs)), env);
    any_sexp_t rest = eval_copy_fvs(any_sexp_cdr(fvs), env);

    if (ANY_SEXP_IS_ERROR(sexp) || ANY_SEXP_IS_ERROR(rest))
        return ANY_SEXP_ERROR;

    return any_sexp_cons(any_sexp_cons(any_sexp_car(fvs), any_sexp_copy_list(sexp)), rest);
}

any_sexp_t eval_lambda(any_sexp_t lambda, any_sexp_t env)
{
    any_sexp_t pars = any_sexp_car(lambda);
    any_sexp_t body = any_sexp_car(any_sexp_cdr(lambda));

    any_sexp_t fvs = eval_get_fvs(body, pars);
    any_sexp_t copy = eval_copy_fvs(fvs, env);

    return ANY_SEXP_IS_ERROR(fvs) || ANY_SEXP_IS_ERROR(copy)
         ? ANY_SEXP_ERROR
         : any_sexp_cons(copy, lambda);
}

any_sexp_t eval_let(any_sexp_t let, any_sexp_t env)
{
    any_sexp_t name = any_sexp_car(any_sexp_car(let));
    any_sexp_t sexp = any_sexp_car(any_sexp_cdr(any_sexp_car(let)));
    any_sexp_t body = any_sexp_car(any_sexp_cdr(let));

    any_sexp_t value = eval(sexp, env);
    if (!ANY_SEXP_IS_SYMBOL(name) || ANY_SEXP_IS_ERROR(value) || ANY_SEXP_IS_ERROR(body))
        return ANY_SEXP_ERROR;

    return eval(body, any_sexp_cons(any_sexp_cons(name, value), env));
}

any_sexp_t eval_begin(any_sexp_t list, any_sexp_t env)
{
    if (ANY_SEXP_IS_NIL(list))
        return ANY_SEXP_NIL;

    if (ANY_SEXP_IS_NIL(any_sexp_cdr(list)))
        return eval(any_sexp_car(list), env);

    if (ANY_SEXP_IS_ERROR(eval(any_sexp_car(list), env)))
        return ANY_SEXP_ERROR;

    return eval_begin(any_sexp_cdr(list), env);
}

any_sexp_t eval_append_env(any_sexp_t pars, any_sexp_t args, any_sexp_t fvs)
{
    if (!ANY_SEXP_IS_NIL(pars) && ANY_SEXP_IS_NIL(args)) {
        log_error("Too few arguments for parameters");
        return ANY_SEXP_ERROR;
    }

    if (ANY_SEXP_IS_NIL(pars) && ANY_SEXP_IS_NIL(args))
        return fvs;

    if (!ANY_SEXP_IS_SYMBOL(any_sexp_car(pars))) {
        log_error("Lambda parameter should be a symbol");
        return ANY_SEXP_ERROR;
    }

    if (!strcmp(ANY_SEXP_GET_SYMBOL(any_sexp_car(pars)), "&rest")) {
        if (!ANY_SEXP_IS_NIL(any_sexp_cdr(pars))) {
            log_error("Rest parameter should be the last");
            return ANY_SEXP_ERROR;
        }

        return any_sexp_cons(any_sexp_cons(any_sexp_car(pars), args), fvs);
    }

    any_sexp_t env = eval_append_env(any_sexp_cdr(pars), any_sexp_cdr(args), fvs);
    if (ANY_SEXP_IS_ERROR(env))
        return ANY_SEXP_ERROR;

    return any_sexp_cons(any_sexp_cons(any_sexp_car(pars), any_sexp_car(args)), env);
}

any_sexp_t eval_lambda_call(any_sexp_t fvs, any_sexp_t pars, any_sexp_t args, any_sexp_t body, any_sexp_t env)
{
    log_value_trace("Lambda call",
                    "g:fvs",  ANY_LOG_FORMATTER(any_sexp_fprint), fvs,
                    "g:pars", ANY_LOG_FORMATTER(any_sexp_fprint), pars,
                    "g:args", ANY_LOG_FORMATTER(any_sexp_fprint), args,
                    "g:body", ANY_LOG_FORMATTER(any_sexp_fprint), body);

    // Update the environment with the arguments
    any_sexp_t body_env = eval_append_env(pars, args, fvs);
    if (ANY_SEXP_IS_ERROR(body_env))
        return ANY_SEXP_ERROR;

    // Eval lambda body with the new environment
    return eval(body, body_env);
}

any_sexp_t eval_add(any_sexp_t sexp, any_sexp_t env)
{
    if (ANY_SEXP_IS_NIL(sexp))
        return any_sexp_number(0);

    if (!ANY_SEXP_IS_CONS(sexp)) {
        log_error("Malformed call to +");
        return ANY_SEXP_ERROR;
    }

    any_sexp_t number = eval(any_sexp_car(sexp), env);
    if (ANY_SEXP_IS_ERROR(number) || !ANY_SEXP_IS_NUMBER(number)) {
        log_error("Invalid number passed to +");
        return ANY_SEXP_ERROR;
    }

    any_sexp_t rest = eval_add(any_sexp_cdr(sexp), env);
    if (ANY_SEXP_IS_ERROR(rest) || !ANY_SEXP_IS_NUMBER(number))
        return ANY_SEXP_ERROR;

    return any_sexp_number(ANY_SEXP_GET_NUMBER(number) + ANY_SEXP_GET_NUMBER(rest));
}

any_sexp_t eval_multiply(any_sexp_t sexp, any_sexp_t env)
{
    if (ANY_SEXP_IS_NIL(sexp))
        return any_sexp_number(1);

    if (!ANY_SEXP_IS_CONS(sexp)) {
        log_error("Malformed call to *");
        return ANY_SEXP_ERROR;
    }

    any_sexp_t number = eval(any_sexp_car(sexp), env);
    if (ANY_SEXP_IS_ERROR(number) || !ANY_SEXP_IS_NUMBER(number)) {
        log_error("Invalid number passed to *");
        return ANY_SEXP_ERROR;
    }

    any_sexp_t rest = eval_multiply(any_sexp_cdr(sexp), env);
    if (ANY_SEXP_IS_ERROR(rest) || !ANY_SEXP_IS_NUMBER(number))
        return ANY_SEXP_ERROR;

    return any_sexp_number(ANY_SEXP_GET_NUMBER(number) * ANY_SEXP_GET_NUMBER(rest));
}

any_sexp_t eval_equal(any_sexp_t sexp, any_sexp_t env)
{
    any_sexp_t cdr = any_sexp_cdr(sexp);
    any_sexp_t cddr = any_sexp_cdr(cdr);

    if (!ANY_SEXP_IS_CONS(sexp) || !ANY_SEXP_IS_CONS(cdr) || !ANY_SEXP_IS_NIL(cddr)) {
        log_error("Malformed call to =");
        return ANY_SEXP_ERROR;
    }

    any_sexp_t a = eval(any_sexp_car(sexp), env);
    any_sexp_t b = eval(any_sexp_car(cdr), env);

    if (ANY_SEXP_IS_ERROR(a) || ANY_SEXP_IS_ERROR(b))
        return ANY_SEXP_ERROR;

    if (ANY_SEXP_IS_STRING(a) && ANY_SEXP_IS_STRING(b))
        return !strcmp(ANY_SEXP_GET_STRING(a), ANY_SEXP_GET_STRING(b))
             ? any_sexp_number(1)
             : ANY_SEXP_NIL;

    if (ANY_SEXP_IS_SYMBOL(a) && ANY_SEXP_IS_SYMBOL(b))
        return !strcmp(ANY_SEXP_GET_SYMBOL(a), ANY_SEXP_GET_SYMBOL(b))
             ? any_sexp_number(1)
             : ANY_SEXP_NIL;

    if (ANY_SEXP_IS_NUMBER(a) && ANY_SEXP_IS_NUMBER(b))
        return ANY_SEXP_GET_NUMBER(a) == ANY_SEXP_GET_NUMBER(b)
             ? any_sexp_number(1)
             : ANY_SEXP_NIL;

    if (ANY_SEXP_IS_NIL(a) || ANY_SEXP_IS_NIL(b))
        return any_sexp_number(1);

    log_error("Incompatible arguments to =");
    return ANY_SEXP_ERROR;
}

any_sexp_t eval_print(any_sexp_t sexp, any_sexp_t env)
{
    if (ANY_SEXP_IS_NIL(sexp))
        return ANY_SEXP_NIL;

    if (!ANY_SEXP_IS_CONS(sexp)) {
        log_error("Malformed call to print");
        return ANY_SEXP_ERROR;
    }

    any_sexp_t value = eval(any_sexp_car(sexp), env);
    if (ANY_SEXP_IS_ERROR(value))
        return ANY_SEXP_ERROR;

    any_sexp_print(value);
    printf("\n");

    return eval_print(any_sexp_cdr(sexp), env);
}

any_sexp_t eval_list(any_sexp_t sexp, any_sexp_t env)
{
    if (ANY_SEXP_IS_NIL(sexp))
        return ANY_SEXP_NIL;

    if (!ANY_SEXP_IS_CONS(sexp)) {
        log_error("Malformed list");
        return ANY_SEXP_ERROR;
    }

    any_sexp_t value = eval(any_sexp_car(sexp), env);
    if (ANY_SEXP_IS_ERROR(value))
        return ANY_SEXP_ERROR;

    return any_sexp_cons(value, eval_list(any_sexp_cdr(sexp), env));
}

any_sexp_t eval_if(any_sexp_t sexp, any_sexp_t env)
{
    any_sexp_t car = any_sexp_car(sexp);
    any_sexp_t cdr = any_sexp_cdr(sexp);
    any_sexp_t cadr = any_sexp_car(cdr);

    any_sexp_t cddr = any_sexp_cdr(cdr);
    any_sexp_t caddr = any_sexp_car(cddr);

    if (!ANY_SEXP_IS_CONS(sexp) || !ANY_SEXP_IS_CONS(cdr) ||
        !ANY_SEXP_IS_CONS(cddr) || !ANY_SEXP_IS_NIL(any_sexp_cdr(cddr))) {
        log_error("Malformed if");
        return ANY_SEXP_ERROR;
    }

    any_sexp_t cond = eval(car, env);
    if (ANY_SEXP_IS_ERROR(cond))
        return ANY_SEXP_ERROR;

    if (!ANY_SEXP_IS_NIL(cond))
        return eval(cadr, env);

    return eval(caddr, env);
}

any_sexp_t eval_cons(any_sexp_t sexp, any_sexp_t env)
{
    any_sexp_cons_t *cons = ANY_SEXP_GET_CONS(sexp);

    // Handle builtin functions
    if (ANY_SEXP_IS_SYMBOL(cons->car)) {

        // (quote exp) <=> (cons 'quote (cons exp nil))
        //
        if (!strcmp(ANY_SEXP_GET_SYMBOL(cons->car), "quote")) {
            if (!ANY_SEXP_IS_NIL(any_sexp_cdr(cons->cdr))) {
                log_error("Malformed quote");
                return ANY_SEXP_ERROR;
            }

            log_trace("Quote");
            return any_sexp_car(cons->cdr);
        }

        // (eval exp)
        //
        if (!strcmp(ANY_SEXP_GET_SYMBOL(cons->car), "eval")) {
            if (!ANY_SEXP_IS_NIL(any_sexp_cdr(cons->cdr))) {
                log_error("Malformed eval");
                return ANY_SEXP_ERROR;
            }

            log_trace("Eval");
            any_sexp_t sexp = eval(any_sexp_car(cons->cdr), env);
            return eval(sexp, ANY_SEXP_NIL);
        }

        // (+ a b c ...)
        //
        if (!strcmp(ANY_SEXP_GET_SYMBOL(cons->car), "+")) {
            log_trace("Add");
            return eval_add(cons->cdr, env);
        }

        // (* a b c ...)
        //
        if (!strcmp(ANY_SEXP_GET_SYMBOL(cons->car), "*")) {
            log_trace("Multiply");
            return eval_multiply(cons->cdr, env);
        }

        // (= a b c ...)
        //
        if (!strcmp(ANY_SEXP_GET_SYMBOL(cons->car), "=")) {
            log_trace("Equal");
            return eval_equal(cons->cdr, env);
        }

        // (print ...)
        //
        if (!strcmp(ANY_SEXP_GET_SYMBOL(cons->car), "print")) {
            log_trace("Print");
            return eval_print(cons->cdr, env);
        }

        // (list ...)
        //
        if (!strcmp(ANY_SEXP_GET_SYMBOL(cons->car), "list")) {
            log_trace("List");
            return eval_list(cons->cdr, env);
        }

        // (tag? x)
        //
        if (!strcmp(ANY_SEXP_GET_SYMBOL(cons->car), "tag?")) {
            if (!ANY_SEXP_IS_NIL(any_sexp_cdr(cons->cdr))) {
                log_error("Malformed tag");
                return ANY_SEXP_ERROR;
            }

            log_trace("Tag");
            return any_sexp_number(ANY_SEXP_GET_TAG(eval(any_sexp_car(cons->cdr), env)));
        }

        // (car l)
        //
        if (!strcmp(ANY_SEXP_GET_SYMBOL(cons->car), "car")) {
            if (!ANY_SEXP_IS_NIL(any_sexp_cdr(cons->cdr))) {
                log_error("Malformed car");
                return ANY_SEXP_ERROR;
            }

            log_trace("Car");
            any_sexp_t list = eval(any_sexp_car(cons->cdr), env);
            if (!ANY_SEXP_IS_CONS(list)) {
                log_error("Expected cons");
                return ANY_SEXP_ERROR;
            }

            return any_sexp_car(list);
        }

        // (cdr l)
        //
        if (!strcmp(ANY_SEXP_GET_SYMBOL(cons->car), "cdr")) {
            if (!ANY_SEXP_IS_NIL(any_sexp_cdr(cons->cdr))) {
                log_error("Malformed cdr");
                return ANY_SEXP_ERROR;
            }

            log_trace("Cdr");
            any_sexp_t list = eval(any_sexp_car(cons->cdr), env);
            if (!ANY_SEXP_IS_CONS(list)) {
                log_error("Expected cons");
                return ANY_SEXP_ERROR;
            }

            return any_sexp_cdr(list);
        }

        // (cons a b)
        //
        if (!strcmp(ANY_SEXP_GET_SYMBOL(cons->car), "cons")) {
            if (!ANY_SEXP_IS_NIL(any_sexp_cdr(any_sexp_cdr(cons->cdr)))) {
                log_error("Malformed cons");
                return ANY_SEXP_ERROR;
            }

            any_sexp_t a = eval(any_sexp_car(cons->cdr), env);
            any_sexp_t b = eval(any_sexp_car(any_sexp_cdr(cons->cdr)), env);

            log_value_trace("Making a cons",
                            "g:a", ANY_LOG_FORMATTER(any_sexp_fprint), a,
                            "g:b", ANY_LOG_FORMATTER(any_sexp_fprint), b);

            return ANY_SEXP_IS_ERROR(a) || ANY_SEXP_IS_ERROR(b)
                 ? ANY_SEXP_ERROR
                 : any_sexp_cons(a, b);
        }

        // (if a b c)
        //
        if (!strcmp(ANY_SEXP_GET_SYMBOL(cons->car), "if")) {
            log_trace("If");
            return eval_if(cons->cdr, env);
        }

        // (lambda (a b c ...) exp)
        //
        if (!strcmp(ANY_SEXP_GET_SYMBOL(cons->car), "lambda")) {
            if (!eval_is_lambda(sexp)) {
                log_error("Malformed lambda");
                return ANY_SEXP_ERROR;
            }

            log_trace("Lambda");
            any_sexp_t lambda = eval_lambda(cons->cdr, env);

            return ANY_SEXP_IS_ERROR(lambda)
                 ? ANY_SEXP_ERROR
                 : any_sexp_cons(cons->car, lambda);
        }

        // (begin a b c ...)
        //
        if (!strcmp(ANY_SEXP_GET_SYMBOL(cons->car), "begin")) {
            log_trace("Begin");
            return eval_begin(cons->cdr, env);
        }

        // (let (name value) body)
        //
        if (!strcmp(ANY_SEXP_GET_SYMBOL(cons->car), "let")) {
            if (!eval_is_let(sexp)) {
                log_error("Malformed let");
                return ANY_SEXP_ERROR;
            }

            log_trace("Let");
            return eval_let(cons->cdr, env);
        }

        // (error a)
        //
        if (!strcmp(ANY_SEXP_GET_SYMBOL(cons->car), "error")) {
            log_trace("Error");
            log_value_error("Called error",
                            "g:sexp", ANY_LOG_FORMATTER(any_sexp_fprint), cons->cdr);
            return ANY_SEXP_ERROR;
        }

        // (apply f l)
        //
        if (!strcmp(ANY_SEXP_GET_SYMBOL(cons->car), "apply")) {

            if (!ANY_SEXP_IS_CONS(cons->cdr) || !ANY_SEXP_IS_CONS(any_sexp_cdr(cons->cdr)) ||
                !ANY_SEXP_IS_NIL(any_sexp_cdr(any_sexp_cdr(cons->cdr)))) {
                log_error("Malformed apply");
                return ANY_SEXP_ERROR;
            }

            log_trace("Apply");
            any_sexp_t lambda = eval(any_sexp_car(cons->cdr), env);
            any_sexp_t list   = eval(any_sexp_car(any_sexp_cdr(cons->cdr)), env);

            if (!ANY_SEXP_IS_CONS(lambda) || !ANY_SEXP_IS_SYMBOL(any_sexp_car(lambda)) ||
                strcmp(ANY_SEXP_GET_SYMBOL(any_sexp_car(lambda)), "lambda") || !ANY_SEXP_IS_CONS(list)) {
                log_error("Invalid arguments passed to apply");
                return ANY_SEXP_ERROR;
            }

            any_sexp_t fvs  = any_sexp_car(any_sexp_cdr(lambda));
            any_sexp_t pars = any_sexp_car(any_sexp_cdr(any_sexp_cdr(lambda)));
            any_sexp_t body = any_sexp_car(any_sexp_cdr(any_sexp_cdr(any_sexp_cdr(lambda))));
            return eval_lambda_call(fvs, pars, list, body, env);
        }

        // (defmacro name (pars ...) body)
        //
        if (!strcmp(ANY_SEXP_GET_SYMBOL(cons->car), "defmacro")) {
            log_error("Defmacro can be used only at the top level");
            return ANY_SEXP_ERROR;
        }

        // (define name value)
        //
        if (!strcmp(ANY_SEXP_GET_SYMBOL(cons->car), "define")) {
            log_error("Define can be used only at the top level");
            return ANY_SEXP_ERROR;
        }

        // (include file)
        //
        if (!strcmp(ANY_SEXP_GET_SYMBOL(cons->car), "include")) {
            log_error("Include can be used only at the top level");
            return ANY_SEXP_ERROR;
        }

        // (expand list)
        //
        if (!strcmp(ANY_SEXP_GET_SYMBOL(cons->car), "expand")) {
            log_error("Expand can be used only at the top level");
            return ANY_SEXP_ERROR;
        }
    }

    any_sexp_t callee = eval(cons->car, env);

    // NOTE: Any lambda here has already been checked
    //
    if (ANY_SEXP_IS_CONS(callee) && ANY_SEXP_IS_SYMBOL(any_sexp_car(callee)) &&
        !strcmp(ANY_SEXP_GET_SYMBOL(any_sexp_car(callee)), "lambda")) {

        any_sexp_t fvs  = any_sexp_car(any_sexp_cdr(callee));
        any_sexp_t pars = any_sexp_car(any_sexp_cdr(any_sexp_cdr(callee)));
        any_sexp_t body = any_sexp_car(any_sexp_cdr(any_sexp_cdr(any_sexp_cdr(callee))));
        any_sexp_t args = eval_list(cons->cdr, env);

        return ANY_SEXP_IS_ERROR(args)
             ? ANY_SEXP_ERROR
             : eval_lambda_call(fvs, pars, args, body, env);
    }

    if (!ANY_SEXP_IS_ERROR(callee))
        log_error("Expected a function as a callee");

    return ANY_SEXP_ERROR;
}

any_sexp_t eval(any_sexp_t sexp, any_sexp_t env)
{
    switch (ANY_SEXP_GET_TAG(sexp)) {
        case ANY_SEXP_TAG_ERROR:
            return ANY_SEXP_ERROR;

        case ANY_SEXP_TAG_NIL:
            return ANY_SEXP_NIL;

        case ANY_SEXP_TAG_CONS:
            return eval_cons(sexp, env);

        case ANY_SEXP_TAG_SYMBOL:
            return eval_symbol(ANY_SEXP_GET_SYMBOL(sexp), env);

        case ANY_SEXP_TAG_STRING:
            return sexp;

        case ANY_SEXP_TAG_NUMBER:
            return sexp;
    }
}

any_sexp_t eval_quote_list(any_sexp_t sexp)
{
    if (ANY_SEXP_IS_NIL(sexp))
        return ANY_SEXP_NIL;

    return any_sexp_cons(any_sexp_quote(any_sexp_car(sexp)),
                         eval_quote_list(any_sexp_cdr(sexp)));
}

any_sexp_t eval_macro_list(any_sexp_t sexp, any_sexp_t env, any_sexp_t menv)
{
    if (ANY_SEXP_IS_NIL(sexp))
        return ANY_SEXP_NIL;

    if (!ANY_SEXP_IS_CONS(sexp)) {
        log_error("Invalid s-expression");
        return ANY_SEXP_ERROR;
    }

    any_sexp_t car = any_sexp_car(sexp);
    any_sexp_t cdr = any_sexp_cdr(sexp);

    return any_sexp_cons(eval_macro(car, env, menv), eval_macro(cdr, env, menv));
}

any_sexp_t eval_macro(any_sexp_t sexp, any_sexp_t env, any_sexp_t menv)
{
    if (ANY_SEXP_IS_CONS(sexp)) {
        any_sexp_t car = any_sexp_car(sexp);
        any_sexp_t cdr = any_sexp_cdr(sexp);

        if (ANY_SEXP_IS_SYMBOL(car)) {
            any_sexp_t macro = eval_find_symbol(ANY_SEXP_GET_SYMBOL(car), menv);

            // Apply macro
            // ((macro-body) (quote a) (quote b) ...)
            //
            if (!ANY_SEXP_IS_ERROR(macro)) {
                log_value_trace("Applying macro",
                                "g:macro", ANY_LOG_FORMATTER(any_sexp_fprint), macro,
                                "g:menv",  ANY_LOG_FORMATTER(any_sexp_fprint), menv);

                any_sexp_t fvs  = any_sexp_car(macro);
                any_sexp_t pars = any_sexp_car(any_sexp_cdr(macro));
                any_sexp_t body = any_sexp_car(any_sexp_cdr(any_sexp_cdr(macro)));

                return eval_macro(eval_lambda_call(fvs, pars, cdr, body, env), env, menv);
            }
        }

        return any_sexp_cons(eval_macro(car, env, menv), eval_macro_list(cdr, env, menv));
    }

    return sexp;
}


any_sexp_t eval_define(any_sexp_t sexp, any_sexp_t *env, any_sexp_t *menv)
{
    if (ANY_SEXP_IS_CONS(sexp) && ANY_SEXP_IS_SYMBOL(any_sexp_car(sexp))) {
        any_sexp_t cdr = any_sexp_cdr(sexp);
        any_sexp_t cadr = any_sexp_car(cdr);
        any_sexp_t cddr = any_sexp_cdr(cdr);
        any_sexp_t caddr = any_sexp_car(cddr);
        any_sexp_t cdddr = any_sexp_cdr(cddr);

        // (define name value)
        //
        if (!strcmp(ANY_SEXP_GET_SYMBOL(any_sexp_car(sexp)), "define")) {

            if (!ANY_SEXP_IS_CONS(cdr) || !ANY_SEXP_IS_CONS(cddr) ||
                !ANY_SEXP_IS_NIL(any_sexp_cdr(cddr)) || !ANY_SEXP_IS_SYMBOL(cadr)) {
                log_error("Malformed define");
                return ANY_SEXP_ERROR;
            }

            any_sexp_t value = eval(eval_macro(caddr, *env, *menv), *env);
            if (ANY_SEXP_IS_ERROR(value))
                return ANY_SEXP_ERROR;

            log_trace("Define (%s)", ANY_SEXP_GET_SYMBOL(cadr));
            *env = any_sexp_cons(any_sexp_cons(cadr, value), *env);
            return ANY_SEXP_NIL;
        }

        // (defmacro name (pars ...) body)
        //
        if (!strcmp(ANY_SEXP_GET_SYMBOL(any_sexp_car(sexp)), "defmacro")) {

            if (!ANY_SEXP_IS_CONS(cdr) || !ANY_SEXP_IS_CONS(cddr) ||
                !ANY_SEXP_IS_CONS(cdddr) || !ANY_SEXP_IS_NIL(any_sexp_cdr(cdddr)) ||
                !ANY_SEXP_IS_SYMBOL(cadr) || !eval_is_symbol_list(caddr)) {
                log_error("Malformed defmacro");
                return ANY_SEXP_ERROR;
            }

            log_trace("Defmacro (%s)", ANY_SEXP_GET_SYMBOL(cadr));
            any_sexp_t lambda = eval_lambda(eval_macro(any_sexp_cons(caddr, cdddr), *env, *menv), *env);
            if (ANY_SEXP_IS_ERROR(lambda))
                return ANY_SEXP_ERROR;

            *menv = any_sexp_cons(any_sexp_cons(cadr, lambda), *menv);
            return ANY_SEXP_NIL;
        }

        // (include file)
        //
        if (!strcmp(ANY_SEXP_GET_SYMBOL(any_sexp_car(sexp)), "include")) {

            if (!ANY_SEXP_IS_STRING(cadr) || !ANY_SEXP_IS_NIL(cddr)) {
                log_error("Malformed include");
                return ANY_SEXP_ERROR;
            }

            const char *path = ANY_SEXP_GET_STRING(cadr);
            FILE *file = fopen(path, "rb");
            if (file == NULL) {
                log_error("Failed to open file %s", path);
                return ANY_SEXP_ERROR;
            }

            log_trace("Include (%s)", path);
            return ANY_SEXP_IS_ERROR(eval_file(file, env, menv))
                 ? ANY_SEXP_ERROR
                 : ANY_SEXP_NIL;
        }

        // (expand list)
        //
        if (!strcmp(ANY_SEXP_GET_SYMBOL(any_sexp_car(sexp)), "expand")) {
            if (!ANY_SEXP_IS_NIL(cddr)) {
                log_error("Malformed expand");
                return ANY_SEXP_ERROR;
            }

            log_trace("Expand");
            return eval_macro(cadr, env, *menv);
        }
    }

    return eval(eval_macro(sexp, *env, *menv), *env);
}

any_sexp_t eval_file(FILE *file, any_sexp_t *env, any_sexp_t *menv)
{
    any_sexp_reader_t reader;
    any_sexp_reader_file_init(&reader, file);

    any_sexp_t sexp;
    do {
        sexp = any_sexp_read(&reader);

        if (ANY_SEXP_IS_ERROR(sexp)) {
            if (any_sexp_reader_end(&reader))
                return ANY_SEXP_NIL;

            log_error("Failed to read s-expression");
            break;
        }

        eval_define(sexp, env, menv);
    } while (!ANY_SEXP_IS_ERROR(sexp));

    return ANY_SEXP_ERROR;
}

static any_sexp_t symbol_list(const char *symbols[], size_t n)
{
    return n == 0
         ? ANY_SEXP_NIL
         : any_sexp_cons(any_sexp_symbol(symbols[n - 1], strlen(symbols[n - 1])),
                         symbol_list(symbols, n - 1));
}

void eval_init()
{
    static const char *symbols[] = {
        "include", "begin", "list",
        "quote", "defmacro", "define",
        "print", "eval", "tag?",
        "if", "lambda", "let",
        "error", "expand", "apply",
        "car", "cdr", "cons",
        "+", "*", "=",
    };
    primitives = symbol_list(symbols, sizeof(symbols) / sizeof(*symbols));

    log_value_trace("Initialized evaluator",
                    "g:primitives", ANY_LOG_FORMATTER(any_sexp_fprint), primitives);
}
