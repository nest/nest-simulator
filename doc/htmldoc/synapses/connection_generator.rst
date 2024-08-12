.. _connection_generator:

Connection generator interface
==============================

.. admonition:: Availability

   This connection rule is only available if NEST was compiled with
   :ref:`support for libneurosim <compile_with_libneurosim>`.

To allow the generation of connectivity by means of an external
library, NEST supports the connection generator interface [2]_. For
more details on this interface, see the git repository of `libneurosim
<https://github.com/INCF/libneurosim>`_.

In contrast to the other rules for creating connections, this rule
relies on a Connection Generator object to describe the connectivity
pattern in a library-specific way. The connection generator is handed
to :py:func:`.Connect` under the key ``cg`` of the connection specification
dictionary and evaluated internally. If the connection generator
provides values for connection weights and delays, their respective
indices can be specified under the key ``params_map``. Alternatively,
all synapse parameters can be specified using the synapse
specification argument to ``Connect()``.

The following listing shows an example for using CSA (`Connection Set Algebra <https://github.com/INCF/csa>`_ [1]_) in NEST via the connection generator interface and randomly connects 10% of the neurons from
``A`` to the neurons in ``B``, each connection having a weight of
10000.0 pA and a delay of 1.0 ms:

.. code-block:: python

   import csa

   A = nest.Create('iaf_psc_alpha', 100)
   B = nest.Create('iaf_psc_alpha', 100)

   # Create the Connection Generator object
   cg = csa.cset(csa.random(0.1), 10000.0, 1.0)

   # Map weight and delay indices to values from cg
   params_map = {'weight': 0, 'delay': 1}

   conn_spec = {'rule': 'conngen', 'cg': cg, 'params_map': params_map}
   nest.Connect(A, B, conn_spec)


References
----------
.. [1] Djurfeldt M. The Connection-set Algebra—A Novel Formalism for the Representation of Connectivity Structure in Neuronal Network Models. Neuroinformatics. 2012; 10: 287–304. https://doi.org/10.1007/s12021-012-9146-1
.. [2] Djurfeldt M, Davison AP and Eppler JM (2014). Efficient generation of
       connectivity in neuronal networks from simulator-independent
       descriptions. Front. Neuroinform.
       https://doi.org/10.3389/fninf.2014.00043
