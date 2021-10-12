#include <string.h>
#include <stdio.h>          // TODO: Remove this when testing done!
#include "string_helper.h"


int strhlp_is_whitespace(char c)
{
	switch (c)
	{
	case ' ':
	case '\t':
	case '\r':
	case '\n':
		return 1;
	default:
		return 0;
	}
}

int strhlp_is_newline(char c)
{
	switch (c)
	{
	case '\n':
	case '\r':
		return 1;
	default:
		return 0;
	}
}

int strhlp_is_quotation(char c)
{
	switch (c)
	{
	case '\"':
	case '\'':
		return 1;
	default:
		return 0;
	}
}

int strhlp_is_anglebracket(char c)
{
	switch (c)
	{
	case '<':
	case '>':
		return 1;
	default:
		return 0;
	}
}


int strhlp_is_whitespace_or_quotation(char c)
{
	return strhlp_is_whitespace(c) || strhlp_is_quotation(c);
}

int strhlp_is_whitespace_or_anglebracket(char c)
{
	return strhlp_is_whitespace(c) || strhlp_is_anglebracket(c);
}


int strhlp_ends_word(char c)
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

int strhlp_get_symbol_length(char* str)
{
	char* ptr = str;
	int cnt = 0;

	while (!strhlp_ends_word(*ptr))
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
	case '>':
	case '\"':
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
		// is alpha-numerical, dot, or underscore, respectively
		// TODO: Could this be done more efficiently?
		if ((c >= 65 && c <= 90) || (c >= 97 && c <= 122) || (c >= 48 && c < 59) ||
			c == 46 || c == 95) {
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


void strhlp_trim_front_back(char* str, int(*trim_func)(char))
{
	// search for beginning of string
	int start_pos = 0;
	while (trim_func(str[start_pos]))
	{
		start_pos++;
	}

	// now move the entire string backwards
	int i1, i2;
	int moves = 0;
	for (i1 = 0, i2 = start_pos; str[i2] != '\0'; i1++, i2++, moves++)
	{
		str[i1] = str[i2];
	}

	// lastly, trim from the end
	i2 -= 1;
	while (trim_func(str[i2]) || i2 >= i1)
	{
		str[i2] = '\0';
		i2 -= 1;
	}
}
