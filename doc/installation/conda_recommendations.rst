Conda environment recommendations
=================================

This page provides a series of recommendations for installing NEST with
conda.

Create a dedicated environment for NEST
---------------------------------------

We recommend that you **create a dedicated environment for NEST**, which
should ensure there are no conflicts with previously installed packages.

Install all programs
--------------------

We strongly recommend that you **install all programs** you'll need, 
(such as ``ipython`` or ``jupyter-lab``) in the environment 
(``<envname>``) **at the same time**, by **appending them to the command below**.

Installing packages later may override previously installed dependencies 
and potentially break packages! See `managing environments in the Conda 
documentation <https://docs.conda.io/projects/conda/en/latest/user-guide/tasks/manage-environments.html#creating-an-environment-with-commands>`_
for more information.

Keep your base environment empty
--------------------------------

Your base environment should be as empty as possible in order to avoid
conflicts with other environments. Always install packages only in the new
environments (don't worry about duplicates, conda will link the packages
if they are used in multiple environments, and not produce disk eating copies).

Get a good overview
-------------------

Obtain a good overview of which packages are installed where. You can use
``conda env export -n base`` and ``conda env export -n yournestenv``
(replacing the ``yournestenv`` name with whatever you chose). Make
sure each environment contains all dependencies. One way to make
this obvious would be to reduce conda stack to ``0`` (conda documentation
`here <https://docs.conda.io/projects/conda/en/latest/user-guide/tasks/manage-environments.html#nested-activation>`_),
and/or to a certain degree by not auto-activating the base environment (conda documentation
`here <https://docs.conda.io/projects/conda/en/latest/user-guide/tasks/manage-environments.html#conda-init>`_).
Then packages from base do not 'leak' into your new environments. Note however, that packages from your system
will usually also be available in your conda environment and may cause similar conflicts.