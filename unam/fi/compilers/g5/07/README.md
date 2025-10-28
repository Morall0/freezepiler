# Freezepiler

C Compiler frontend and backend.

## How to compile

First, make sure you are in the project's root directory `/unam/fi/compilers/g5/07/`.
Once there, run the following command to build the project:

~~~ bash
$ make 
~~~

This will create the executable inside the `build/` directory.

## How to run

After compiling, you can run the executable from the same root directory. The program accepts once argument: either a path to a source file or the `-s`flag followed by a string. 

### Examples of execution:

~~~ bash
# Example 1: path to a file
$ ./build/lexer path/to/program.c

# Example 2: string
$ ./build/lexer -s 'printf("hello world!");'
~~~
