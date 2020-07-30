The Connection Generator Interface
==================================

Since 2014 (`Djurfeldt et al.,
2014 <http://dx.doi.org/10.3389/fninf.2014.00043>`__) NEST supports the
Connection Generator Interface. This allows to couple connection
generating libraries to NEST without having to modify NEST itself.

In contrast to the :doc:`regular creation of connections <connection_management>`, the
Connection Generator Interface provides a different way of specifying connectivity:

1. pre- and postsynaptic neurons are created
2. a Connection Generator object is created which specifies the
   connectivity pattern
3. the Connection Generator is applied to the sets of source and target
   neurons using the ``CGConnect`` function.

The API is part of the neurosim library hosted at INCF:

  http://software.incf.org/software/libneurosim


High-level Python API
---------------------

.. code-block:: python

   CGConnect (SOURCES, TARGETS, CG, [PARAMETER_MAP, [MODEL]])

Connect neurons from ``SOURCES`` to neurons from ``TARGETS`` using
connectivity specified by the connection generator ``CG``. ``SOURCES`` and
``TARGETS`` are both NodeCollections representing the node IDs. ``PARAMETER_MAP``
is a dictionary mapping names of values such as weight and delay to
value set positions. ``MODEL`` is the synapse model.


Example in PyNEST using the Connection-Set Algebra
--------------------------------------------------

In Python, the ``csa`` module can be used directly to specify the
connectivity.

.. code:: python

   import nest
   import csa

   # Create the neurons
   sources = nest.Create("iaf_psc_alpha", 2)
   targets = nest.Create("iaf_psc_alpha", 2)

   # Create the Connection Generator for one-to-one connectivity
   cg = csa.cset(csa.oneToOne)

   # Connect the neurons
   nest.CGConnect(sources, targets, cg)

   # Verify the connectivity
   conn = nest.GetConnections(sources)
   print((c['source'], c['target']) for c in nest.GetStatus(conn))


Example in SLI using the Connection-Set Algebra
-----------------------------------------------

In SLI, the CSA library is not available directly and the Connection
Generator has to be created by deserializing an XML string (or file). To
do so, we first have to select a library to perform the parsing of the
serialized string using ``CGSelectImplementation``. It takes the root
tag of the serialization and the name of the C++ library as arguments.
We then create the Connection Generator from the serialization using
``CGParse``. As in PyNEST, these ingredients can be passed on to
``CGConnect``.

.. code:: postscript

   % Select the connection generating library
   (CSA) (libcsa.so) CGSelectImplementation

   % Parse the serialized connection generator
   (<?xml version='1.0' encoding='UTF-8'?><CSA xmlns="http://software.incf.org/software/csa/1.0"><oneToOne/></CSA>) CGParse /cg Set

   % Create the neurons
   /iaf_psc_alpha 4 Create ;
   /sources [ 1  2] def
   /targets [3 4] def

   % Connect the neurons
   cg  sources targets CGConnect

   % Verify the connectivity
   << /source [ 1  2] >> GetConnections { GetStatus /target get } Map ==


References
----------

The interface was presented as a poster at the 4th INCF Congress of
Neuroinformatics:

.. [1] Jochen Eppler, Håkon Enger, Thomas Heiberg, Birgit Kriener, Hans
       Plesser, Markus Diesmann and Mikael Djurfeldt (2011) "Evaluating the
       Connection-Set Algebra for the neural simulator NEST", Conference
       Abstract: 4th INCF Congress of Neuroinformatics, Front. Neuroinform.,
       doi:10.3389/conf.fninf.2011.08.00085
