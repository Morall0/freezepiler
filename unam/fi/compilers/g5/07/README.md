# Freezepiler

C Compiler frontend and backend.

## How to compile

First, make sure you are in the project's root directory `/unam/fi/compilers/g5/07/`.
Once there, run the following command to build the project:

~~~ bash
$ make 
~~~

This will create the executable inside the `bin/` directory.

## How to run

After compiling, you can run the executable from the same root directory. The program accepts once argument: either a path to a source file or the `-s`flag followed by a string. It also accepts the `-v` (verbose) option that displays additional information about the compilation process.

### Examples of execution:

~~~ bash
# Example 1: path to a file
$ ./bin/main path/to/program.c

# Example 2: string and displaying extra info
$ ./bin/main -s 'int main(void) {printf("hello world!"); return 0;} -v'
~~~
