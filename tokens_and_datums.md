---
layout: index
---

[« Back to the index](index)

<hr>

# Creating and handling Tokens and Datums

Class `Token` is a wrapper class around `Datum` pointers and non-datum
objects. In fact, since datum objects have a memory manager, we should
avoid creating Datum objects on the stack as local variables. Class
`Token` takes ownership of the `Datum` pointers and will properly
delete it when it is no longer needed. Thus, use one of the following
idioms:

## Construction

    Token t(new IntergerDatum(5));
    Token t=5;
    Token t= new IntegerDatum(5);

The object constructor `Token(Datum&)` is historic and should not be
used anymore.

## Assignment

    Token t1=t;
    t1.move(t); // move datum from t to t1

`TokenArrays`, `TokenStack`, and `Dictionary` are token
containers. Their assignment interface takes

1. datum pointers
2. token references

Thus, the recommended assignments are

    array.push_back(new IntegerDatum(5));

It directly passes the datum pointer to the location in the
array. Some convenient ways to write assignments are actually
inefficient

## Examples

1. `a.push_back(5);`

   This is convenient notation, but is much more expensive, because it is
   equivalent to the following code:

       IntegerDatum tmp1(5); 
       Token tmp2(new IntegerDatum(mp1));
       Token tmp3(tmp2);  // one more datum copy
       a.push_back_move(tmp3);

   Some of this, the compiler can optimize away, but benchmarks showed a
   big residual overhead compared to directly assigning the datum
   pointer.

2. `a.push_back(IntegerDatum(5));`

   This looks efficient, but in fact it is not, because it is equivalent
   to:

       Token tmp1( new IntegerDatum(IntegerDatum(5)); 
       a.push_back_move(tmp1);

3. `a.push_back(t);`

   Involves one datum copy

4. `a.push_back_move(t);`

   Moves the pointer and leaves a void token behind.