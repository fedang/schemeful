#include <stdio.h>
#include <string.h>

#include "eval.h"

#define ANY_LOG_IMPLEMENT
#include "any_log.h"

// TODO: Actually handle memory...

void repl()
{
    printf("My own little lisp :)\n");

    eval_init();
    any_sexp_t env = ANY_SEXP_NIL, menv = ANY_SEXP_NIL;

    any_sexp_reader_t reader;
    any_sexp_reader_string_t string;
    char buffer[ANY_SEXP_READER_BUFFER_LENGTH];

    while (true) {
        printf("\n> ");
        fflush(stdout);

        if (!fgets(buffer, ANY_SEXP_READER_BUFFER_LENGTH, stdin))
            break;

        any_sexp_reader_string_init(&reader, &string, buffer, strlen(buffer));

        any_sexp_t sexp = any_sexp_read(&reader);
        if (ANY_SEXP_IS_ERROR(sexp)) {
            log_error("Invalid input expression");
            continue;
        }

        any_sexp_print(sexp);
        printf("\n===>\n");
        any_sexp_t value = eval_define(sexp, &env, &menv);
        any_sexp_print(value);

        eval_change_env(any_sexp_symbol("?", 1), value, &env);
    }

    log_value_info("Eval state",
                   "g:env", ANY_LOG_FORMATTER(any_sexp_fprint), env,
                   "g:menv", ANY_LOG_FORMATTER(any_sexp_fprint), menv);
    //any_sexp_free_list(env);
}

void usage()
{
    printf("Usage: schemeful [--trace] [file]\n");
}

int main(int argc, char **argv)
{
    any_log_level_t level = ANY_LOG_INFO;
    int argb = 1;

    if (argc > 1 && !strcmp(argv[1], "--trace")) {
        argb++;
        level = ANY_LOG_TRACE;
    }

    any_log_init(stdout, level);

    if ((argc - argb) == 0) {
        repl();
        return 0;
    }

    if ((argc - argb) == 1) {
        FILE *file = fopen(argv[argb], "rb");
        if (file == NULL) {
            log_error("Failed to open file %s", argv[argb]);
            return 1;
        }

        eval_init();
        any_sexp_t env = ANY_SEXP_NIL, menv = ANY_SEXP_NIL;

        eval_file(file, &env, &menv);
        //any_sexp_free_list(env);
        return 0;
    }


    usage();
    return 1;
}
