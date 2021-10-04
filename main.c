const char* cyclic_dependency() {
	return "Cyclic module dependency detected. Aborting build script!";
}

const char* invalid_module_syntax() {
	return "Invalid module syntax. Failed to generate build script!";
}


int main()
{
	// TODO: This might be implemented through I/O-redirection,
	// though does that work on other platforms than Linux?
  return 0;
}
