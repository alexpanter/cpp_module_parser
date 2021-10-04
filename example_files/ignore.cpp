export module ignore;
import nonimportant;
import dontcare;

int ignore_this_function(char c)
{
	// do something that we don't care about when building
	// module dependency graph.
	return -55;
}

#ifdef UNKNOWN_MAGIC_CONSTANT
static_assert(false);
#endif
