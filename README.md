# ep2
programatic exercise of alprog2
# Compiling and testing memory leaks
The following command line will compile the program. But will generate a error message when program exit if there's a memory leak. The flag `-g` add source mapping, so this error message becomes more human friendly.

g++ -g -fsanitize=leak -o ep2 ep2.cpp

# Warnings
Don't use in any circunstance the makefile in `devmakefile/makefile`, because it isn't directory indepent and may overwrite/delete/leak sensitive information, including the source file.

