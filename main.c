#include <stdio.h>
#include <string.h>

#include "eval.h"

#define ANY_LOG_IMPLEMENT
#include "any_log.h"

int main()
{
	any_log_init(stdout, ANY_LOG_TRACE);

	printf("My own little lisp :)\n");

	any_sexp_reader_t reader;
	any_sexp_reader_string_t string;

	char buffer[ANY_SEXP_READER_BUFFER_LENGTH];

	// ((lambda (x y) x) "a" "b") ==> "a"
	// (((lambda (x) (x x)) (lambda (x) x)) "it works!") ==> "it works!"

	while (true) {
		printf("\n> ");
		fflush(stdout);

		if (!fgets(buffer, ANY_SEXP_READER_BUFFER_LENGTH, stdin))
			break;

		any_sexp_reader_string_init(&reader, &string, buffer, strlen(buffer));

		any_sexp_t sexp = any_sexp_read(&reader);
		any_sexp_print(sexp);

		printf("\n==>\n");

		any_sexp_print(eval(sexp, ANY_SEXP_NIL));
		any_sexp_free_list(sexp);
	}

	return 0;
}
