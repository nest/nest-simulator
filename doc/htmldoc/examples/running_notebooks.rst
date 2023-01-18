.. _run_jupyter:

How to run Jupyter notebooks
============================

Using EBRAINS Jupyter service
-------------------------------

Prerequisites

  * an EBRAINS account 

    If you do not have an account yet, you can sign up here: https://ebrains.eu/register.

1. Click on the Try it on EBRAINS button in the desired :ref:`example notebook <pynest_examples>`.

  .. image:: https://nest-simulator.org/TryItOnEBRAINS.png

  You will be redirected to lab.ebrains.eu

2. If not signed into ebrains already, you will be asked to sign in.

  .. image:: ../static/img/signin-ebrains.png

3. Choose a lab excution site

  .. image:: ../static/img/lab-execution-site-de.png

4. Jupyter lab will then try to clone the repository and open the selected notebook.
   The other notebooks wil be available in Jupyter lab in the left column, as well.

Now you can run the notebook and change variables to test how it works.

  .. image:: ../static/img/running-jupyterlab.png

----

Troubleshooting
---------------

Jupyter lab tries to clone the repository but an error occurs
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

  .. image:: ../static/img/error-jupyter-ebrains.png

* Close the current tab and open https://lab.ebrains.eu.

* Select the lab execution site that you were trying to access.

* Once the lab opens, go to ``File > Hub Control Panel``.

  .. image:: ../static/img/hubcontrol-notebook.png

* Stop the server - this may take several minutes.

  .. warning::

    If you stop the server, you will lose any changes you made during your session that
    have not been stored elsewhere. Download or commit any files you wish to keep before restarting the server!

  .. image:: ../static/img/stopserver-ebrains.png

* Once the server stops, you can retry running the notebook again.

 Alternatively, You can also try another execution site.



Error occurs when trying to run notebook: ``nest not found``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

* Check that the kernel version is correct (EBRAINS-22.10 and later should be compatible with NEST)

  .. image:: ../static/img/kernel-version-jupyter.png

----

Still not working?
~~~~~~~~~~~~~~~~~~

`Create an issue on GitHub <https://github.com/nest/nest-simulator/issues/new/choose>`_. 

The developer team will look into issues as quickly as possible.
