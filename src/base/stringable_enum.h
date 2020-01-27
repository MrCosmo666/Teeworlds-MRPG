#pragma once

#ifndef STRINGABLE_ENUM_H
#define STRINGABLE_ENUM_H

#define STRINGABLE_ENUM_ADD_COMMA(E) E,

#define STRINGABLE_ENUM_ELEM_STR(E) case E: return #E;

#define STRINGABLE_ENUM_STR_DECL(ENUM_NAME) \
const char *str_##ENUM_NAME(const ENUM_NAME a);

#define STRINGABLE_ENUM_STR_IMPL(ENUM_NAME) \
const char *str_##ENUM_NAME(const ENUM_NAME a) { \
    switch (a) { \
        ENUM_NAME(STRINGABLE_ENUM_ELEM_STR) \
        default: return "undefined"; \
    } \
}

#define STRINGABLE_ENUM_DECL(NAME) \
enum NAME { NAME(STRINGABLE_ENUM_ADD_COMMA) NUM_##NAME };\
STRINGABLE_ENUM_STR_DECL(NAME);

#define STRINGABLE_ENUM_IMPL(NAME) \
    STRINGABLE_ENUM_STR_IMPL(NAME);

#endif /* STRINGABLE_ENUM_H */