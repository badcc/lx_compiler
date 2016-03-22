# lx_lang
Depends on my `ab` library which isn't currently public.
Currently interprets, will eventually compile. Probably to x86 ASM, but maybe C to utilize the optimizer.

## Story
On my plane ride I began writing a compiler in C for a language I made up just to learn more about compilers. Currently just an interpreter and only uses numbers (doubles), but code is designed in such a way that it's easy to add strings and more data types. Creates Abstract Syntax Tree. Expressions are in Postfix (RPN) notation, but can easily change to use of Infix using Shunting-yard algorithm that I've already implemented in C. See code in lang2.c

## Note
This langauge is not meant for real use. It's a learning experience.


    # Whitespace doesn't matter
    hello:=4 
    world:=3
    hello=2
    res:=hello world ^
    print(res)
