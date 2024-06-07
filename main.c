#include <stdio.h>
#include <string.h>

#include "eval.h"

#define ANY_LOG_IMPLEMENT
#include "any_log.h"

void print_env(any_sexp_t env)
{
    log_info("Environment:");
    any_sexp_print(env);
    printf("\n");
}

void repl()
{
    printf("My own little lisp :)\n");
    any_sexp_t env = ANY_SEXP_NIL;

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
        any_sexp_print(eval_define(sexp, &env));
        //any_sexp_free_list(sexp);
    }

    print_env(env);
    any_sexp_free_list(env);
}

int main(int argc, char **argv)
{
    any_log_init(stdout, ANY_LOG_TRACE);

    if (argc == 1) {
        repl();
        return 0;
    }

    if (argc == 2) {
        FILE *file = fopen(argv[1], "rb");
        if (file == NULL) {
            log_error("Failed to open file %s", argv[1]);
            return 1;
        }

        any_sexp_t env = ANY_SEXP_NIL;
        eval_file(file, &env);
        print_env(env);
        any_sexp_free_list(env);
        return 0;
    }

    log_error("Unexpected arguments");
    return 1;
}
