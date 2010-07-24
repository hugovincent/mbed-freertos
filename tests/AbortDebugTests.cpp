#include <stdio.h>

int global_int;

/* Func4 generates a prefetch abort */
void func4(void)
{
	void (*bad)(int) = (void (*)(int))0x09000000;
	printf("About to prefetch abort... bye bye\n");

	bad(global_int);
}

/* Func3 generates a data abort */
void func3(void)
{
	int *bad = (int *)0x09000000;

	printf("About to have a data abort... bye bye\n");
	*bad = 0x12345678;
}

/* Func1 and Func2 are just to add some stuff to the backtrace */
void func2(void)
{
	int *good = &global_int;
	*good = 2;

	printf("In func%d\n", global_int);
	func3();
}

void func1(void)
{
	int *good = &global_int;
	*good = 1;

	printf("In func%d\n", global_int);
	func2();
}
