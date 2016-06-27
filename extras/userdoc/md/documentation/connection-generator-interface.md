# The Connection Generator Interface

NEST supports the Connection Generator Interface since 2014 ([Djurfeldt et al., 2014](http://dx.doi.org/10.3389/fninf.2014.00043)). It allows to couple connection generating libraries to NEST without having to modify NEST itself.

In contrast to the [built-in connection functions](connection-management.md) and the Topology module, the Connection Generator Interface has a different way for specifying the connectivity:

1. a connection generator object is created
2. neurons are created
3. the connection generator is applied to a list of source and target neurons using the `CGConnect functions`.

## Example using the Connection-Set Algebra


