# `lib` folder

Definition of high-level API of PyNEST.

This file defines the user-level functions of NEST's Python interface
by mapping NEST commands to Python. Please try to follow these
rules:

1. Nodes are identified by their global IDs (node ID) by default.

2. node IDs are always represented by a NodeCollection, e.g.
   NodeCollection(metadata=None, model=iaf_psc_alpha, size=10, first=1, last=10)

3. Commands that return a node ID must return it as NodeCollection.

4. If you have a *very* good reason, you may deviate from these guidelines.
