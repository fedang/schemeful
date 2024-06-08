// any_sexp
//
// A single-file library that provides a simple and somewhat opinionated
// interface for parsing and manipulating s-expressions.
//
// Note that the library does not offer the means to evaluate the
// s-expressions. That can be easily implemented separately using the
// provided any_sexp_t type and helper functions.
//
// To use this library you should choose a suitable file to put the
// implementation and define ANY_SEXP_IMPLEMENT. For example
//
//    #define ANY_SEXP_IMPLEMENT
//    #include "any_sexp.h"
//
// Additionally, you can customize the library behavior by defining certain
// macros in the file where you put the implementation. You can see which are
// supported by reading the code guarded by ANY_SEXP_IMPLEMENT.
//
// This library is licensed under the terms of the MIT license.
// A copy of the license is included at the end of this file.
//

#ifndef ANY_SEXP_INCLUDE
#define ANY_SEXP_INCLUDE

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdio.h>

typedef enum {
    ANY_SEXP_TAG_ERROR  = 0xf,
    ANY_SEXP_TAG_NIL    = 0,
    ANY_SEXP_TAG_CONS   = 1 << 0,
    ANY_SEXP_TAG_SYMBOL = 1 << 1,
    ANY_SEXP_TAG_STRING = 1 << 2,
    ANY_SEXP_TAG_NUMBER = 1 << 3,
} any_sexp_tag_t;

#ifdef ANY_SEXP_NO_BOXING

typedef struct any_sexp {
    any_sexp_tag_t tag;
    union {
        struct any_sexp_cons *cons;
        char *symbol;
        intptr_t number;
    };
} any_sexp_t;

#define ANY_SEXP_ERROR (any_sexp_error())
#define ANY_SEXP_NIL   (any_sexp_nil())

#define ANY_SEXP_GET_TAG(sexp)    ((sexp).tag)
#define ANY_SEXP_GET_CONS(sexp)   ((sexp).cons)
#define ANY_SEXP_GET_SYMBOL(sexp) ((sexp).symbol)
#define ANY_SEXP_GET_STRING(sexp) ((sexp).symbol)
#define ANY_SEXP_GET_NUMBER(sexp) ((sexp).number)

#else

typedef void *any_sexp_t;

#define ANY_SEXP_ERROR (any_sexp_t)UINTPTR_MAX
#define ANY_SEXP_NIL   (any_sexp_t)NULL

#define ANY_SEXP_BIT_SHIFT  (sizeof(uintptr_t) * 8 - 4)
#define ANY_SEXP_BIT_SIGN   ((uintptr_t)1 << (ANY_SEXP_BIT_SHIFT - 1))
#define ANY_SEXP_UNTAG(sexp) (any_sexp_t)((uintptr_t)(sexp) & ~((uintptr_t)0xf << ANY_SEXP_BIT_SHIFT))

#define ANY_SEXP_TAG(sexp, tag)   (any_sexp_t)((uintptr_t)(sexp) | ((uintptr_t)tag << ANY_SEXP_BIT_SHIFT))
#define ANY_SEXP_GET_TAG(sexp)    (((uintptr_t)(sexp) >> ANY_SEXP_BIT_SHIFT) & 0xf)
#define ANY_SEXP_GET_CONS(sexp)   ((any_sexp_cons_t *)ANY_SEXP_UNTAG(sexp))
#define ANY_SEXP_GET_SYMBOL(sexp) (((char *)ANY_SEXP_UNTAG(sexp)))
#define ANY_SEXP_GET_STRING(sexp) (((char *)ANY_SEXP_UNTAG(sexp)))
#define ANY_SEXP_GET_NUMBER(sexp) (any_sexp_number_untag(sexp))

static inline intptr_t any_sexp_number_untag(any_sexp_t sexp)
{
    uintptr_t ptr = (uintptr_t)ANY_SEXP_UNTAG(sexp);
    uintptr_t clear = ptr & ~((uintptr_t)0x1f << (ANY_SEXP_BIT_SHIFT - 1));

    // NOTE: Pad with 1's to adjust the two's complement when the sign is negative
    if (ptr & ANY_SEXP_BIT_SIGN)
        clear |= (uintptr_t)0x1f << (ANY_SEXP_BIT_SHIFT - 1);
    return *(intptr_t *)&clear;
}

#endif

#define ANY_SEXP_IS_TAG(sexp, tag) (ANY_SEXP_GET_TAG(sexp) == (tag))
#define ANY_SEXP_IS_ERROR(sexp)    (ANY_SEXP_IS_TAG(sexp, ANY_SEXP_TAG_ERROR))
#define ANY_SEXP_IS_NIL(sexp)      (ANY_SEXP_IS_TAG(sexp, ANY_SEXP_TAG_NIL))
#define ANY_SEXP_IS_CONS(sexp)     (ANY_SEXP_IS_TAG(sexp, ANY_SEXP_TAG_CONS))
#define ANY_SEXP_IS_SYMBOL(sexp)   (ANY_SEXP_IS_TAG(sexp, ANY_SEXP_TAG_SYMBOL))
#define ANY_SEXP_IS_STRING(sexp)   (ANY_SEXP_IS_TAG(sexp, ANY_SEXP_TAG_STRING))
#define ANY_SEXP_IS_NUMBER(sexp)   (ANY_SEXP_IS_TAG(sexp, ANY_SEXP_TAG_NUMBER))

#define ANY_SEXP_GET_CAR(sexp) (ANY_SEXP_GET_CONS(sexp)->car)
#define ANY_SEXP_GET_CDR(sexp) (ANY_SEXP_GET_CONS(sexp)->cdr)

typedef struct any_sexp_cons {
    any_sexp_t car;
    any_sexp_t cdr;
} any_sexp_cons_t;

typedef int (*any_sexp_getchar_t)(void *stream);

typedef int (*any_sexp_putchar_t)(int c, FILE *stream);

#ifndef ANY_SEXP_NO_READER

#ifndef ANY_SEXP_READER_BUFFER_LENGTH
#define ANY_SEXP_READER_BUFFER_LENGTH 512
#endif

typedef struct {
    any_sexp_getchar_t getc;
    void *stream;
    int c;
} any_sexp_reader_t;

typedef struct {
    const char *source;
    size_t length;
    size_t cursor;
} any_sexp_reader_string_t;

void any_sexp_reader_init(any_sexp_reader_t *reader, any_sexp_getchar_t getc, void *stream);

void any_sexp_reader_string_init(any_sexp_reader_t *reader, any_sexp_reader_string_t *string, const char *source, size_t length);

void any_sexp_reader_file_init(any_sexp_reader_t *reader, FILE *file);

bool any_sexp_reader_end(any_sexp_reader_t *reader);

any_sexp_t any_sexp_read(any_sexp_reader_t *reader);

#endif

#ifndef ANY_SEXP_NO_WRITER

typedef struct {
    any_sexp_putchar_t putc;
    void *stream;
} any_sexp_writer_t;

void any_sexp_writer_init(any_sexp_writer_t *writer, any_sexp_putchar_t putc, void *stream);

int any_sexp_write(any_sexp_writer_t *writer, any_sexp_t sexp);

int any_sexp_fprint(FILE *file, any_sexp_t sexp);

int any_sexp_print(any_sexp_t sexp);

#endif

any_sexp_t any_sexp_error(void);

any_sexp_t any_sexp_nil(void);

any_sexp_t any_sexp_symbol(const char *symbol, size_t length);

any_sexp_t any_sexp_string(const char *string, size_t length);

any_sexp_t any_sexp_number(intptr_t value);

any_sexp_t any_sexp_quote(any_sexp_t sexp);

any_sexp_t any_sexp_cons(any_sexp_t car, any_sexp_t cdr);

any_sexp_t any_sexp_car(any_sexp_t sexp);

any_sexp_t any_sexp_cdr(any_sexp_t sexp);

any_sexp_t any_sexp_reverse(any_sexp_t sexp);

any_sexp_t any_sexp_append(any_sexp_t a, any_sexp_t b);

any_sexp_t any_sexp_copy(any_sexp_t sexp);

any_sexp_t any_sexp_copy_list(any_sexp_t sexp);

void any_sexp_free(any_sexp_t sexp);

void any_sexp_free_list(any_sexp_t sexp);

#endif

#ifdef ANY_SEXP_IMPLEMENT

#include <ctype.h>

#ifndef ANY_SEXP_MALLOC
#include <stdlib.h>
#define ANY_SEXP_MALLOC malloc
#define ANY_SEXP_FREE free
#endif

#ifndef ANY_SEXP_CHAR_COMMENT
#define ANY_SEXP_CHAR_COMMENT ';'
#endif

#ifndef ANY_SEXP_CHAR_OPEN
#define ANY_SEXP_CHAR_OPEN '('
#endif

#ifndef ANY_SEXP_CHAR_CLOSE
#define ANY_SEXP_CHAR_CLOSE ')'
#endif

#ifndef ANY_SEXP_CHAR_STRING
#define ANY_SEXP_CHAR_STRING '"'
#endif

#ifndef ANY_SEXP_CHAR_ESCAPE
#define ANY_SEXP_CHAR_ESCAPE '\\'
#endif

#ifndef ANY_SEXP_CHAR_QUOTE
#define ANY_SEXP_CHAR_QUOTE '\''
#endif

#ifndef ANY_SEXP_QUOTE_SYMBOL
#define ANY_SEXP_QUOTE_SYMBOL "quote"
#endif

#ifndef ANY_SEXP_NO_READER

// For the scheme specification these are the extended characters
// to be accepted in addition to the alphanumeric ones in a symbol.
//     ! $ % & * + - . / : < = > ? @ ^ _ ~
//
// However here we allow any character except the ones used by
// other syntactic constructs.
//
static inline bool any_sexp_issym(char c)
{
#ifndef ANY_SEXP_NO_COMMENT
    if (c == ANY_SEXP_CHAR_COMMENT)
        return false;
#endif

    if (c == ANY_SEXP_CHAR_STRING)
        return false;

#ifndef ANY_SEXP_NO_QUOTE
    if (c == ANY_SEXP_CHAR_QUOTE)
        return false;
#endif

    if (c == ANY_SEXP_CHAR_QUOTE)
        return false;

    return c != EOF
        && c != ANY_SEXP_CHAR_OPEN
        && c != ANY_SEXP_CHAR_CLOSE
        && !isspace(c);
}

static inline char any_sexp_reader_advance(any_sexp_reader_t *reader)
{
    reader->c = reader->getc(reader->stream);
    return reader->c;
}

static void any_sexp_reader_skip(any_sexp_reader_t *reader)
{
    while (!any_sexp_reader_end(reader)) {
#ifndef ANY_SEXP_NO_COMMENT
        if (reader->c == ANY_SEXP_CHAR_COMMENT) {
            while (!any_sexp_reader_end(reader) && reader->c != '\n')
                any_sexp_reader_advance(reader);
        }
#endif
        if (!isspace(reader->c))
            return;

        any_sexp_reader_advance(reader);
    }
}

static char any_sexp_reader_string_getc(any_sexp_reader_string_t *string)
{
    return string->cursor < string->length
         ? string->source[string->cursor++]
         : EOF;
}

void any_sexp_reader_init(any_sexp_reader_t *reader, any_sexp_getchar_t getc, void *stream)
{
    reader->getc = getc;
    reader->stream = stream;
    any_sexp_reader_advance(reader);
}

void any_sexp_reader_string_init(any_sexp_reader_t *reader, any_sexp_reader_string_t *string, const char *source, size_t length)
{
    string->source = source;
    string->length = length;
    string->cursor = 0;
    any_sexp_reader_init(reader, (any_sexp_getchar_t)any_sexp_reader_string_getc, string);
}

void any_sexp_reader_file_init(any_sexp_reader_t *reader, FILE *file)
{
    any_sexp_reader_init(reader, (any_sexp_getchar_t)fgetc, file);
}

bool any_sexp_reader_end(any_sexp_reader_t *reader)
{
    return reader->c == EOF;
}

any_sexp_t any_sexp_read(any_sexp_reader_t *reader)
{
    char buffer[ANY_SEXP_READER_BUFFER_LENGTH + 1];

    any_sexp_reader_skip(reader);

    // Symbol | Number
    if (any_sexp_issym(reader->c)) {
        size_t length = 0;
        bool number = true;

        do {
            if (length < ANY_SEXP_READER_BUFFER_LENGTH) {
                number = number && (length == 0 && reader->c == '-' || isdigit(reader->c));
                buffer[length++] = reader->c;
            }

            any_sexp_reader_advance(reader);
        } while (any_sexp_issym(reader->c));

        buffer[length] = '\0';

        if (number) {
            intptr_t value = strtol(buffer, NULL, 10);
            return any_sexp_number(value);
        }

        return any_sexp_symbol(buffer, length);
    }

    // String
    if (reader->c == ANY_SEXP_CHAR_STRING) {
        any_sexp_reader_advance(reader);

        size_t length = 0;
        char prev = '\0';

        while (!any_sexp_reader_end(reader)) {
            if (reader->c == ANY_SEXP_CHAR_STRING && prev != ANY_SEXP_CHAR_ESCAPE)
                break;

            if (length < ANY_SEXP_READER_BUFFER_LENGTH)
                buffer[length++] = reader->c;

            prev = reader->c;
            any_sexp_reader_advance(reader);
        }

        any_sexp_reader_advance(reader);
        return any_sexp_string(buffer, length);
    }

#ifndef ANY_SEXP_NO_QUOTE
    // Quote
    if (reader->c == ANY_SEXP_CHAR_QUOTE) {
        any_sexp_reader_advance(reader);
        return any_sexp_quote(any_sexp_read(reader));
    }
#endif

    // List
    if (reader->c == ANY_SEXP_CHAR_OPEN) {
        any_sexp_reader_advance(reader);

        any_sexp_t sexp = ANY_SEXP_NIL;

        while (!any_sexp_reader_end(reader)) {
            any_sexp_reader_skip(reader);

            if (reader->c == ANY_SEXP_CHAR_CLOSE)
                break;

            any_sexp_t sub = any_sexp_read(reader);
            sexp = any_sexp_cons(sub, sexp); // reversed

            if (ANY_SEXP_IS_ERROR(sub) || ANY_SEXP_IS_ERROR(sexp)) {
                any_sexp_free(sexp);
                return ANY_SEXP_ERROR;
            }
        }

        any_sexp_reader_advance(reader);
        return any_sexp_reverse(sexp);
    }

    return ANY_SEXP_ERROR;
}

#endif

#ifndef ANY_SEXP_NO_WRITER

static int any_sexp_writer_puts(any_sexp_writer_t *writer, const char *string)
{
    int i;
    for (i = 0; string[i] != '\0'; i++) {
        if (writer->putc(string[i], writer->stream) == EOF)
            return EOF;
    }
    return i;
}

static int any_sexp_writer_putnum(any_sexp_writer_t *writer, intptr_t value)
{
    int c = 0;
    if (value > 9) {
        c = any_sexp_writer_putnum(writer, value / 10);
        value %= 10;
    }

    if (writer->putc('0' + value, writer->stream) == EOF)
        return EOF;

    return c + 1;
}

void any_sexp_writer_init(any_sexp_writer_t *writer, any_sexp_putchar_t putc, void *stream)
{
    writer->putc = putc;
    writer->stream = stream;
}

int any_sexp_write(any_sexp_writer_t *writer, any_sexp_t sexp)
{
    switch (ANY_SEXP_GET_TAG(sexp)) {
        case ANY_SEXP_TAG_ERROR:
            return any_sexp_writer_puts(writer, "<error>");

        case ANY_SEXP_TAG_NIL: {
            const char nil[3] = { ANY_SEXP_CHAR_OPEN, ANY_SEXP_CHAR_CLOSE, '\0' };
            return any_sexp_writer_puts(writer, nil);
        }

        case ANY_SEXP_TAG_CONS: {
            any_sexp_t car = any_sexp_car(sexp), cdr = any_sexp_cdr(sexp);

#ifndef ANY_SEXP_NO_QUOTE
            if (ANY_SEXP_IS_SYMBOL(car) && ANY_SEXP_IS_NIL(any_sexp_cdr(cdr)) &&
                !strcmp(ANY_SEXP_GET_SYMBOL(car), ANY_SEXP_QUOTE_SYMBOL)) {

                if (writer->putc(ANY_SEXP_CHAR_QUOTE, writer->stream) == EOF)
                    return EOF;

                return any_sexp_write(writer, any_sexp_car(cdr));
            }
#endif

            if (writer->putc(ANY_SEXP_CHAR_OPEN, writer->stream) == EOF)
                return EOF;

            int c = 2, tmp;
            while (true) {
                tmp = any_sexp_write(writer, car);
                if (tmp == EOF)
                    return EOF;
                c += tmp;

                if (ANY_SEXP_IS_CONS(cdr)) {
                    if (writer->putc(' ', writer->stream) == EOF)
                        return EOF;

                    car = any_sexp_car(cdr);
                    cdr = any_sexp_cdr(cdr);
                    c++;
                } else if (!ANY_SEXP_IS_NIL(cdr)) {
                    if (any_sexp_writer_puts(writer, " . ") == EOF)
                        return EOF;

                    car = cdr;
                    cdr = ANY_SEXP_NIL;
                    c += 3;
                } else
                    break;
            }

            if (writer->putc(ANY_SEXP_CHAR_CLOSE, writer->stream) == EOF)
                return EOF;

            return c;
        }

        case ANY_SEXP_TAG_SYMBOL:
            return any_sexp_writer_puts(writer, ANY_SEXP_GET_SYMBOL(sexp));

        case ANY_SEXP_TAG_STRING: {
            if (writer->putc(ANY_SEXP_CHAR_STRING, writer->stream) == EOF)
                return EOF;

            int c = any_sexp_writer_puts(writer, ANY_SEXP_GET_STRING(sexp));
            if (c == EOF)
                return EOF;

            if (writer->putc(ANY_SEXP_CHAR_STRING, writer->stream) == EOF)
                return EOF;

            return c + 2;
        }

        case ANY_SEXP_TAG_NUMBER: {
            intptr_t value = ANY_SEXP_GET_NUMBER(sexp);
            bool sign = value < 0;
            if (sign) {
                value = -value;
                if (writer->putc('-', writer->stream) == EOF)
                    return EOF;
            }
            return sign + any_sexp_writer_putnum(writer, value);
        }
    }
}

int any_sexp_fprint(FILE *file, any_sexp_t sexp)
{
    any_sexp_writer_t writer;
    any_sexp_writer_init(&writer, (any_sexp_putchar_t)fputc, file);
    return any_sexp_write(&writer, sexp);
}

int any_sexp_print(any_sexp_t sexp)
{
    return any_sexp_fprint(stdout, sexp);
}

#endif

any_sexp_t any_sexp_error(void)
{
#ifndef ANY_SEXP_NO_BOXING
    return ANY_SEXP_ERROR;
#else
    any_sexp_t sexp = {
        .tag = ANY_SEXP_TAG_ERROR,
    };
    return sexp;
#endif
}

any_sexp_t any_sexp_nil(void)
{
#ifndef ANY_SEXP_NO_BOXING
    return ANY_SEXP_NIL;
#else
    any_sexp_t sexp = {
        .tag = ANY_SEXP_TAG_NIL,
    };
    return sexp;
#endif
}

any_sexp_t any_sexp_symbol(const char *symbol, size_t length)
{
    if (symbol == NULL)
        return ANY_SEXP_ERROR;

    char *copy = ANY_SEXP_MALLOC(length + 1);
    if (copy == NULL)
        return ANY_SEXP_ERROR;

    memcpy(copy, symbol, length);
    copy[length] = '\0';

#ifndef ANY_SEXP_NO_BOXING
    return ANY_SEXP_TAG(copy, ANY_SEXP_TAG_SYMBOL);
#else
    any_sexp_t sexp = {
        .tag = ANY_SEXP_TAG_SYMBOL,
        .symbol = copy,
    };
    return sexp;
#endif
}

any_sexp_t any_sexp_string(const char *string, size_t length)
{

    any_sexp_t sexp = any_sexp_symbol(string, length);
    if (ANY_SEXP_IS_ERROR(sexp))
        return ANY_SEXP_ERROR;

    // Override the tag
#ifndef ANY_SEXP_NO_BOXING
    return ANY_SEXP_TAG(ANY_SEXP_UNTAG(sexp), ANY_SEXP_TAG_STRING);
#else
    sexp.tag = ANY_SEXP_TAG_STRING;
    return sexp;
#endif
}

any_sexp_t any_sexp_number(intptr_t value)
{
#ifndef ANY_SEXP_NO_BOXING
    // Handle the sign bit!

    uintptr_t sexp = *(uintptr_t *)&value;
    sexp &= ~((uintptr_t)0x1f << (ANY_SEXP_BIT_SHIFT - 1));

    if (value < 0)
        sexp |= ANY_SEXP_BIT_SIGN;

    return ANY_SEXP_TAG(sexp, ANY_SEXP_TAG_NUMBER);
#else
    any_sexp_t sexp = {
        .tag = ANY_SEXP_TAG_NUMBER,
        .number = value,
    };
    return sexp;
#endif
}

any_sexp_t any_sexp_quote(any_sexp_t sexp)
{
    any_sexp_t quote = any_sexp_symbol(ANY_SEXP_QUOTE_SYMBOL, strlen(ANY_SEXP_QUOTE_SYMBOL));
    return any_sexp_cons(quote, any_sexp_cons(sexp, ANY_SEXP_NIL));
}

any_sexp_t any_sexp_cons(any_sexp_t car, any_sexp_t cdr)
{
    any_sexp_cons_t *cons = ANY_SEXP_MALLOC(sizeof(any_sexp_cons_t));
    if (cons == NULL)
        return ANY_SEXP_ERROR;

    cons->car = car;
    cons->cdr = cdr;

#ifndef ANY_SEXP_NO_BOXING
    return ANY_SEXP_TAG(cons, ANY_SEXP_TAG_CONS);
#else
    any_sexp_t sexp = {
        .tag = ANY_SEXP_TAG_CONS,
        .cons = cons,
    };
    return sexp;
#endif
}

any_sexp_t any_sexp_car(any_sexp_t sexp)
{
    if (!ANY_SEXP_IS_CONS(sexp))
        return ANY_SEXP_ERROR;
    return ANY_SEXP_GET_CAR(sexp);
}

any_sexp_t any_sexp_cdr(any_sexp_t sexp)
{
    if (!ANY_SEXP_IS_CONS(sexp))
        return ANY_SEXP_ERROR;
    return ANY_SEXP_GET_CDR(sexp);
}

any_sexp_t any_sexp_reverse(any_sexp_t sexp)
{
    if (ANY_SEXP_IS_NIL(sexp))
        return sexp;

    if (!ANY_SEXP_IS_CONS(sexp))
        return ANY_SEXP_ERROR;

    any_sexp_t cons = sexp,
               prev = ANY_SEXP_NIL,
               next = ANY_SEXP_NIL;

    while (ANY_SEXP_GET_CONS(cons) != NULL) {
        if (!ANY_SEXP_IS_NIL(cons) && !ANY_SEXP_IS_CONS(cons))
            return ANY_SEXP_ERROR;

        memcpy(&next, &ANY_SEXP_GET_CDR(cons), sizeof(any_sexp_t));
        memcpy(&ANY_SEXP_GET_CDR(cons), &prev, sizeof(any_sexp_t));
        memcpy(&prev, &cons, sizeof(any_sexp_t));
        memcpy(&cons, &next, sizeof(any_sexp_t));
    }

    return prev;
}

any_sexp_t any_sexp_append(any_sexp_t a, any_sexp_t b)
{
    if (ANY_SEXP_IS_NIL(a))
        return b;

    if (!ANY_SEXP_IS_CONS(a))
        return ANY_SEXP_ERROR;

    any_sexp_t tail = any_sexp_append(any_sexp_cdr(a), b);
    return ANY_SEXP_IS_ERROR(tail)
         ? ANY_SEXP_ERROR
         : any_sexp_cons(any_sexp_car(a), tail);
}

any_sexp_t any_sexp_copy(any_sexp_t sexp)
{
    switch (ANY_SEXP_GET_TAG(sexp)) {
        case ANY_SEXP_TAG_ERROR:
        case ANY_SEXP_TAG_NIL:
        case ANY_SEXP_TAG_NUMBER:
            return sexp;

        case ANY_SEXP_TAG_CONS:
            return any_sexp_cons(any_sexp_car(sexp), any_sexp_cdr(sexp));

        case ANY_SEXP_TAG_SYMBOL: {
            char *symbol = ANY_SEXP_GET_SYMBOL(sexp);
            return any_sexp_symbol(symbol, strlen(symbol));
        }

        case ANY_SEXP_TAG_STRING: {
            char *string = ANY_SEXP_GET_STRING(sexp);
            return any_sexp_symbol(string, strlen(string));
        }
    }
}

any_sexp_t any_sexp_copy_list(any_sexp_t sexp)
{
    if (ANY_SEXP_IS_CONS(sexp)) {
        any_sexp_t car = any_sexp_copy_list(any_sexp_car(sexp));
        any_sexp_t cdr = any_sexp_copy_list(any_sexp_cdr(sexp));
        return any_sexp_cons(car, cdr);
    }

    return any_sexp_copy(sexp);
}

void any_sexp_free(any_sexp_t sexp)
{
    switch (ANY_SEXP_GET_TAG(sexp)) {
        case ANY_SEXP_TAG_ERROR:
        case ANY_SEXP_TAG_NIL:
        case ANY_SEXP_TAG_NUMBER:
            break;

        case ANY_SEXP_TAG_CONS:
            ANY_SEXP_FREE(ANY_SEXP_GET_CONS(sexp));
            break;

        case ANY_SEXP_TAG_SYMBOL:
        case ANY_SEXP_TAG_STRING:
            ANY_SEXP_FREE(ANY_SEXP_GET_SYMBOL(sexp));
            break;
    }
}

void any_sexp_free_list(any_sexp_t sexp)
{
    if (ANY_SEXP_IS_CONS(sexp)) {
            any_sexp_free_list(any_sexp_car(sexp));
            any_sexp_free_list(any_sexp_cdr(sexp));
    }

    any_sexp_free(sexp);
}

#endif

// MIT License
//
// Copyright (c) 2024 Federico Angelilli
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
