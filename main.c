#include <stdio.h>
#include <string.h>

#include "eval.h"

#define ANY_LOG_IMPLEMENT
#include "any_log.h"

void repl()
{
    printf("My own little lisp :)\n");

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
        any_sexp_print(eval(sexp, ANY_SEXP_NIL));
        any_sexp_free_list(sexp);
    }
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

        any_sexp_reader_t reader;
        any_sexp_reader_file_init(&reader, file);

        any_sexp_t sexp;
        do {
            sexp = any_sexp_read(&reader);
            eval(sexp, ANY_SEXP_NIL);
            any_sexp_free_list(sexp);
        } while (!ANY_SEXP_IS_ERROR(sexp));

        return 0;
    }

    log_error("Unexpected arguments");
    return 1;
}
