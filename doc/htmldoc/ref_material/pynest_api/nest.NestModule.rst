.. _sec_kernel_attributes:

Kernel attributes (nest.NestModule)
===================================


The NEST kernel can be controlled from the PyNEST interface by getting or
setting attributes on the ``nest`` module:

.. code-block:: python

  # set a single kernel attribute
  nest.resolution = 0.1

  # set multiple attributes at once
  nest.set(min_delay=0.1, max_delay=2.0)

  # if you have the attributes in a dictionary
  params = {'min_delay': 0.1, 'max_delay': 2.0}
  nest.set(**params)

Here is a list of attributes that can be get and/or set on the ``nest`` module:


.. autoclass:: nest.NestModule
   :members:
   :no-undoc-members:
