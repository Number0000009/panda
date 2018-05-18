
int global_initialized_var = 0x11223344;
int global_uninitialized_var;


void foo()
{
	int scoped_initialized_var = 0x12345678;
	int scoped_uninitialized_var;
	(void) scoped_initialized_var;
	(void) scoped_uninitialized_var;
}

int main()
{
	volatile int i = 666;
	(void) i;
	return 0;
}
