#include <string.h>
#include "string_helper.h"


int string_helper_is_whitespace(char c)
{
	switch (c)
	{
	case ' ':
	case '\t':
		return 1;
	default:
		return 0;
	}
}

int string_helper_ends_word(char c)
{
	// TODO: Inverse check would be better!
	switch (c)
	{
	case ' ':
	case '\t':
	case '\r':
	case '\n':
	case '\0':
	case ';':
	case ':':
	case '-':
	case '?':
	case '+':
		return 1;
	default:
		return 0;
	}
}

int get_symbol_length(char* str)
{
	char* ptr = str;
	int cnt = 0;

	while (!string_helper_ends_word(*ptr))
	{
		ptr++;
	}
	return cnt;
}


int strhlp_ends_keyword_export(char c)
{
	switch (c)
	{
	case '\0':
	case '\n':
	case '\r':
	case ' ':
	case ';':
		return 1;
	default:
		return 0;
	}
}

int strhlp_ends_keyword_import(char c)
{
	switch (c)
	{
	case '\0':
	case '\n':
	case '\r':
	case ' ':
	case ';':
	case ':':
		return 1;
	default:
		return 0;
	}
}

int strhlp_ends_keyword_module(char c)
{
	switch (c)
	{
	case '\0':
	case '\n':
	case '\r':
	case ' ':
	case ';':
		return 1;
	default:
		return 0;
	}
}


char* strhlp_read_module_name(char* str, int* str_len)
{
	int len = 0;
	char* ptr = str;
	while (1)
	{
		char c = *ptr;
		// is alpha-numerical or dot, respectively
		if ((c >= 65 && c <= 90) || (c >= 97 && c <= 122) || (c >= 48 && c < 59) || c == 46) {
			len++;
			ptr++;
			continue;
		}
		else if (strhlp_ends_keyword_import(c) && len > 0) {
			*str_len = len;
			return strndup(str, len);
		}
		else {
			return NULL;
		}
	}
}
