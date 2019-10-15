# `lib` folder

Definition of high-level API of PyNEST.

This file defines the user-level functions of NEST's Python interface
by mapping NEST/SLI commands to Python. Please try to follow these
rules:

1. SLI commands have the same name in Python. This means that most
   function names are written in camel case, although the Python
   guidelines suggest to use lower case for function names. However,
   this way, it is easier for users to migrate from SLI to Python.
   
2. Nodes are identified by their global IDs (GID) by default.

3. GIDs are always represented by a GIDCollection, e.g.
   GIDCollection(metadata=None, model=iaf_psc_alpha, size=10, first=1, last=10)

4. Commands that return a GID must return it as GIDCollection.

5. When possible, loops over nodes should be propagated down to the
   SLI level. This minimizes the number of Python<->SLI conversions
   and increases performance. Loops in SLI are also faster than in
   Python. 
        
6. If you have a *very* good reason, you may deviate from these guidelines.
