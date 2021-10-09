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
