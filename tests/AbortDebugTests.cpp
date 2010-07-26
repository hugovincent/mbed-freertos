#include <stdio.h>

volatile int global_int;

/* Func5 generates an undefined instruction trap */
void func5(void)
{
	printf("About to undefined instruction trap... bye bye\n");

	int opcode = 0x07f000f0; // this is the minimal guaranteed undefined (as opposed to unpredictable) instruction
	void (*test)() = (void (*)())&opcode;
	test();
}

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

	/* Random stuff in register (to check the register dump is correct): */
	volatile register int test1 = 0xDEADBEEF;

	printf("About to have a data abort [0x%x]... bye bye\n", test1);

	if (global_int == 2) /* Trick the optimizer */
		test1 <<= 4;

	*bad = 0x12345678;
}

/* Func1 and Func2 are just to add some stuff to the backtrace */
void func2(void)
{
	volatile int *good = &global_int;
	*good = 2;

	printf("In func%d\n", global_int);
	func3();
}

void func1(void)
{
	volatile int *good = &global_int;
	*good = 1;

	printf("In func%d\n", global_int);
	func2();
}
