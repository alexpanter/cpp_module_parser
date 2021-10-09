#ifndef STRING_HELPER_H
#define STRING_HELPER_H

int string_helper_is_whitespace(char c);
int string_helper_ends_word(char c);
int get_symbol_length(char* str);

int strhlp_ends_keyword_export(char c);
int strhlp_ends_keyword_import(char c);
int strhlp_ends_keyword_module(char c);

char* strhlp_read_module_name(char* str, int* str_len);

#endif // STRING_HELPER_H
