---
layout: index
---

[« Back to the index](index)

<hr>

# SLI Coding Guidelines

## Overview

1. Names of SLI operators shall conform to the following rules.

        functionname     - standard name for PS conforming operator
        FunctionName     - names of Mathematica-like operator, or
                           (other-language)-like operator
        :functionname    - name of an undocumented, internal operator which 
                           may disappear from the language anytime. Should
                           only be used in libraries, but not in user
                           programs.
        ::functionname   - name of undocumented, internal operator, which 
                           operates on the execution stack.

1. Names may have type-specifier (TS) appended

        functionnameTS
 The type-specifier is interpreted from left to right. It indicates which parameters are expected on the operand stack. The rightmost type is expected topmost on the stack.

1. A type-specifier is defined as follows

        non-terminals := {TS, TA}
        terminals     := {tn, _, epsilon}
        TA            :=    tn   |  TA a
        TS            := _ TA TS | epsilon
 `tn` may be expanded to one of the following:

        integer     i
        double      d
        number      num  /* integer or double */
        bool        b
        name        n
        literal     l
        procedure   p
        lprocedure  lp
        array       a
        dictionary  di
        string      s
        trie        t
        istream     is
        xistream    xs
        ostream     os
        regex       r
        any         ""  /* the empty string */
 Homogeneous arrays are indicated by appending `a` to the above mentioned type specifiers. This may even by used in a recursive fashion, to indicate nested homogeneous arrays. For example, the following homogeneous SLI arrays have those type specifiers:

        array of integers            ia
        array of doubles             da
        array of numbers             numa
        array of dictionaries        dia
        array of arrays of istreams  isaa

## Examples

    pop_    - takes one argument of arbitrary type         
    exch__  - takes two arguments of arbitrary type
    foo_i_  - takes one integer and one anytype.
    foo__i  - takes one anytype and one integer.
    foo_d_i - takes one double and one integer.
    foo_di  - takes one dictionary

1. A name with type-specifier is not required to perform typechecking.
1. Names with type-specifier are called `variants`.
1. The object which combines all `variants` is called `root`. (The name `root` is motivated by the fact that this is the part of the operator name which is common to all variants).
1. The `root` variant is required to perform type-checking.
1. The general properties of the operator shall be documented with the `root`.
1. All information which is specific to a `variant`, shall be documented with the `variant`.
1. From the documentation of a `variant` there shall be a reference to its `root`.
1. In the documentation of the `root`, a reference to the `variants` is not mandatory, but recommended.
