# Standart Compilation Mode
First, make sure to clone the stable or pre-stable version of program. Then the program should compiles without warnings and errors. To compile in stantard deploy mode use:

	g++ -o ep2 ep2.cpp

With either `-O1`, `-O2` or `-O3`, if some otimizations are desidered. And it also can be compiled in standart testing mode using:

g++ -g -Wall -fstack-protector-all -fsanitize=undefined,leak,address -o ep2 ep2.cpp

Which should be the same effect as the previuos compilation, except that will be slower.
# Supported operating systems
The following operating systems are tested with pre-stable and stable version:
- Debian 5.10.149-2 x86_64 GNU/Linux (Linux 5.10.0-19-amd64) 
# Pre-stable requirements
The first requiment to mark the program as pre-stable is that it compiles in standart testing mode without warnings. And it also shouldn't generate major warnings when compile with `-Wextra`. 

Then the program shall pass all sanity test without generating errors at the end, which are enabled during compilation with `-D__SANITY_TESTS__` (or verbosely with `-D__VERBOSE_TESTS__`). It is recomended that also test it in interactive mode, enabled in compilation with `-D__INTERACTIVE_TESTS__`

Finally it can be marked as pre-stable. 

The pre-stable version doen't recieve any feature directly. Instead the unstable version should be used. Bugfixes also should meet the same above requimentent to be merged into pre-stable.
# Stable requirements

First, consider varing some of [Intrumentation Options (GCC)]{https://gcc.gnu.org/onlinedocs/gcc-6.3.0/gcc/Instrumentation-Options.html} avaliabre in gcc/g++, which may need to use either `-llsan` or `-fsanitize-undefined-trap-on-error`.

Then the program shall pass against the Standart Given Test.

# Testing against Standart Given Test
There is two test that was initally given, they are stored at `given_tests`. So, copy it to working directory to use them:

	cp given_tests/* .

After that, the program compiled in either standart deploy mode or standart testing mode can be called using the following arguments:

	./ep2 2 test1.txt test2.txt

Which should create and yields the result at `resultado.out`. Which can be compared against the expected result with:

	diff -s expected.out resultado.out

Which will report that both file are identical on sucess. And finally, if all previuos was sucessfully done without errors and warning, then the program was passed it this test.
