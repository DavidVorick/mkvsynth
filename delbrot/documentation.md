# delbrot documentation #
delbrot has grown appreciably in complexity since adopting a flex+bison implementation. This document aims to explain the inner workings of the delbrot.l, delbrot.y, and delbrot.c.

## basic workings ##
flex and bison are programs that take definition files and turn them into C functions. flex creates a *lexing function*, which reads in a segment of the input file, matches it to a rule, and returns the corresponding token. bison creates a *parsing function*, which reads in tokens, matches them to a rule, and executes the corresponding C code. The whole process looks something like this:

    /********** flex ***********\ /** bison **\
    scanner -> tokenizer -> lexer -> parser -> interpreter

If your language's grammar is very simple, bison can serve as your interpreter, and the two definition files are all you need. But for any sort of high-level language, bison alone isn't enough. Instead of being used to execute code, bison is used to generate an *abstract syntax tree*, or AST, which contains all the parsed values, function calls, etc. in a hierarchical structure. For example, the AST for a simple `if` block could be represented as:

           if
          /  \
         /    \
        /      \
      cond    stmt
     / | \      |
    x  0  >    ...

And the equivalent C code would be:

    if (x > 0) {
        ...
    }

Once a section of the AST is fully built, a separate function is used to walk down the AST and execute the code. Specifically, delbrot uses a *recursive descent* model, meaning it starts at the top of the AST and recursively evaluates each child argument before combining them to produce a result.

## delbrot.l ##
This file contains the lexing rules for the language. flex matches text using regexs and always chooses the longest match. The corresponding C code is then executed. In most cases, this amounts to little more than returning the matched value and/or appropriate token type. The special cases here are for identifiers and numbers. For our purposes, an identifier is a character sequence that does not begin with a number, and a number is a sequence of digits with an optional decimal portion. Since all text is scanned in as chars, we must first convert digits to numbers with the `atof()` function. Identifiers are a bit more complicated, but essentially we just try to match the identifier against a list of keywords, functions, and variables. If it's a keyword, we return the appropriate token. If it's a function name, we return the appropriate function pointer. If it's an existing variable, we return the appropriate variable pointer. If it's none of those, we assume it's a new variable and allocate space for it in our symbol table.

## delbrot.y ##
Here we define the grammar of the language. Now that we have a sequence of tokens, we read them in one at a time until they match one of the defined rules, at which point we execute the corresponding C code. First we define precedences for each operator, so the parser knows how to resolve expressions like `x + y / x`. Next comes the main grammar section. For each rule, we define what arrangements of tokens qualify as an instance of that rule. For example, a statement can be an expression, a function definition, an if-else block, etc. For each instance, we call `mkOpNode()` with the appropriate arguments, which connects "operation nodes" (like `+` or `=`) to their child nodes. Below the grammar section, we fill out the definitions for the ASTnode and variable functions.

## delbrot.c ##
This file contains the `ex()` function, which is called each time we read in a full statement. The purpose of `ex()` is to reduce a section of the AST. In the example above, that would mean reducing the "cond" node (and all its children) down to a single node containing either "true" or "false." As such, this function must be highly recursive, since a node's children may need to be reduced before they can be meaningfully combined. This is also where we define the built-in functions of the language.

## memory concerns ##
delbrot's memory management system has gone through endless revisions, all of them deficient in some way. Presently it is pretty leaky, but hopefully a more sophisticated garbage collector will change that. Memory management for the parser isn't a huge concern because most scripts won't contain any looping or recursion. However, it's still an important issue to address.
