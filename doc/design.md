# Rune Language Design Document

Rune is a toy programming language that is being developed largely for the sake of fun and learning.  But hopefully it will also turn into something interesting, with at least one or two worthwhile ideas in it.

This document is an attempt to document the current language design.  It may not always be 100% up-to-date, but it should be reasonably representative.



Brackets, braces, and parenthesis
---------------------------------

The use of braces etc. in Rune are significantly different than in C-style languages.  The goal is to have each type of brace--as much as possible--represent a set of _related_ concepts.

The general philosophy is as follows:

`()` are used for grouping executable code together.  This means that they are used for grouping expressions just like in C-style languages, but they also define code blocks and lexical scope.

`{}` are used for grouping data types together.  In particular this means that they are used for defining and representing data structures such as structs, tuples, unions, etc.  They are also used when writing literals of such structures.

`[]` are for function call parameters.

This non-standard approach means that code may be a little tricky to read at first if you are used to C-style languages.  But the benefit is that there is a very clear visual distinction between e.g. function call parameters and a tuple literal, among other things.



Comments
--------

    # This is a comment.  All comments are
    # single-line comments.  They start
    # with "#" and continue until the end of
    # the line:

    val a = 2  # Comments can start anywhere in a line

    #: Doc comments start with "#:" and go to the end
    #: of the line.  Doc comments without intervening
    #: non-whitespace characters are treated as a
    #: single contiguous doc comment.
    #: Doc comments, unlike regular comments, are only
    #: legal at specific places in the code.  They must
    #: always immediately precede something that is
    #: "documentable", such as a function or type
    #: definition.



Declarations
------------

There are four kinds of declarations in Rune:

- Types
- Compile-time constants
- Immutable variables
- Mutable variables

They are written (in their respective minimal forms) like this:

    # Type declaration
    type Foo: ...
    
    # Compile-time constant declaration
    const bar = ...
    
    # Immutable variable declaration
    val a = ...
    
    # Mutable variable declaration
    var b

Where "..." is a stand-in for actual code.

`const` and `val` declarations _require_ a value initializer.  `var` can optionally have a value initializer, but does not require one.  `type` declarations don't have a value initializer.  Value initializers are an `=` followed by an expression:

    var b = 42

`type` declarations _require_ a type specification.  `const`, `val`, and `var` can optionally include a type specification.  Types are specified with a `:` after the name, followed by a type:

    # 8-bit unsigned integer
    var b: u8
    
Note that type specification (if present) always precedes value initialization:

    var b: u8 = 42

Just because type specification is optional for `const`, `val`, and `var` declarations does not mean that they are typeless.  Rather, the compiler can often infer their type from how they are used.  If the compiler cannot infer their type, then you must provide a type specification.


Built-in Types
--------------

There are several built-in types in Rune.

The signed and unsigned integers:

    # Signed
    i8
    i16
    i32
    i64
    
    # Unsigned
    u8
    u16
    u32
    u64

The floating point numbers:

    f16
    f32
    f64
    
And a few miscellaneous types:

    byte       # Raw byte of data, without any particular interpretation
    codepoint  # Unicode code point (32 bits)
    @T         # Pointer to memory with type T
    []T        # Non-owning slice with elements of type T (basically a fat
               # pointer that specifies an array length)
    [n]T       # An array of n elements of type T.  Both n and T are part
               # of its type.

All other data types in Rune are built by putting these types together in interesting ways via compound types (covered later).



Expressions
-----------

Expressions are pieces of executable code that (typically) evaluate to some kind of data.  Nearly anything that is not a declaration, type definition, or compiler directive is an expression.  An example of a simple expression:

    1 + 2

This expression takes two integers and adds them together, resulting in a value of 3.

Many expressions have side-effects beyond the value they return.  Assignment is a good example:

    a = 1 + 2

Because expressions can have side-effects, it's important to be able to put them in an ordered sequence to be executed one-after-the-other.  In Rune you do this by putting expressions on separate lines:

    a = 1 + 2  # Executed first
    a = a - 5  # Executed second
    b = 42     # Executed third
    
You can group sub-expressions using parenthesis to force evaluation precidence:

    3 * 1 + 4    # Evaluates to 7
    3 * (1 + 4)  # Evaluates to 15



Lexical scope
-------------

Parenthesis also determine lexical scope:

    (
        val foo = 5
    )
    val bar = foo + 3  # ILLEGAL: foo went out of scope



Functions
---------

Functions are defined with function literals.  A function literal looks like this:

    fn [a: i32, b: i32] -> i32 (
        val c = a + b
        return c
    )

To call a function you typically need to give it a name first.  You do this the same way you give any other value a name: with a `const`, `val`, or `var` declaration:

    const foo = fn [a: i32, b: i32] -> i32 (
        val c = a + b
        return c
    )

Now you can call the function like so:

    foo[1, 3]

There is syntactic sugar for declaring a named const function:

    fn foo [a: i32, b: i32] -> i32 (
        val c = a + b
        return c
    )

This is exactly identical in meaning to the `const foo` declaration above.

There is, however, one thing that the syntactic sugar version can do that the `const` declaration version cannot.  The sugar version can declare functions with operator names:

    fn + [a: i32, b: i32] -> i32 (
        val c = a + b
        return c
    )



Function Call Sugar
-------------------

In addition to the standard function call syntax shown above, there are three other function call syntaxes:

    # Unary prefix call:
    # Only works for functions with precisely one parameter.
    foo 5
    
    # Binary infix call:
    # Only works for functions with precisely two parameters.
    1 foo 3
    
    # Method call:
    # Works for functions with one or more parameters, and uses the value
    # to the left of the dot as the first parameter.
    1.foo[3]

These calling syntaxes are just syntactic sugar for the standard syntax, and (importantly) only work for compile-time-constant functions.



Compound Types
--------------

Rune supports several ways of composing types together to create other types.  But to understand how to utilize them, you need to understand a bit of how Rune thinks about types and names.


### Type Declarations ###

Much like the distinction between a variable and a literal, Rune makes a distinction between named types and type "literals".  To create a new named type, you use the `type` keyword:

    type Meters: f32

This declares a new type `Meters` that is structurally identical to a 32-bit float.  However, note that `Meters` is considered a distinct type from `f32`.  `type` doesn't create type aliases, it creates completely new types.  This is important because, for example, if we also defined a `Liters` type from `f32` we wouldn't want `Meters` and `Liters` to be implicitly interchangeable.

If you _do_ want to convert between types, you can do so explicitly by using the `as` keyword:
    
    val m: Meters = 5.0
    val l: Liters = m as Liters


### Tuples ###

The simplest compound type in Rune is the tuple.  A tuple type is written like this:

    {i32, i32, f64}

As with any other type, you can either use tuple types directly or create new named types out of them with the `type` keyword:

    type Foo: {i32, i32, f64}
    
    var a: {i32, i32, f64}
    var b: Foo

A tuple literal looks like this:

    {42, 53, 6.4}

A tuple literal from a named tuple type looks like this:

    Foo{42, 53, 6.4}

Literals can be used to initialze or assign variables or constants with tuple types:

    var a: {i32, i32, f64}
    var b: Foo
    
    a = {42, 53, 6.4}
    b = Foo{42, 53, 6.4}


### Structs ###

Structs are almost identical to tuples, except that their fields are named:

    struct {
        x: i32,
        y: i32,
        baz: f32
    }

    type Bar: struct {
        x: i32,
        y: i32,
        baz: f32
    }

You can access a struct's fields using dot notation:

    var a: Bar
    
    a.x = 42
    a.y = a.x + 5
    a.baz = 3.5

Struct literals look like this:

    # Un-named
    struct{x=42, y=53, baz=6.4}
    
    # Named
    Bar{x=42, y=53, baz=6.4}