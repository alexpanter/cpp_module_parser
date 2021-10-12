#ifndef STRING_HELPER_H
#define STRING_HELPER_H

int strhlp_is_whitespace(char c);
int strhlp_is_newline(char c);
int strhlp_is_quotation(char c);
int strhlp_is_anglebracket(char c);

int strhlp_is_whitespace_or_quotation(char c);
int strhlp_is_whitespace_or_anglebracket(char c);

int strhlp_ends_word(char c);
int strhlp_get_symbol_length(char* str);

int strhlp_ends_keyword_export(char c);
int strhlp_ends_keyword_import(char c);
int strhlp_ends_keyword_module(char c);

char* strhlp_read_module_name(char* str, int* str_len);

void strhlp_trim_front_back(char* str, int(*trim_func)(char));

#endif // STRING_HELPER_H
