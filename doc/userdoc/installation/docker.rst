.. _docker:

|macos| |linux| |windows| Docker
--------------------------------

Docker provides an isolated container to run applications. The
NEST Docker container includes a complete install of NEST and
is set up so you can create, modify, and run Juptyer Notebooks
and save them on your host machine.  (See the Note below for
alternative ways to use the Docker container.)

1. If you do not have Docker installed, follow the Docker
   installation instructions for your system here:
   https://docs.docker.com/install/.

   If you are using **Linux**, we **strongly recommend** you
   also create a Docker group to manage Docker as a non-root
   user. See instructions on the Docker website:
   https://docs.docker.com/install/linux/linux-postinstall/


2. Create a directory or change into a directory that you want
   to use for your Jupyter Notebooks.

.. code-block:: bash

    mkdir my_nest_scripts
    cd my_nest_scripts

3. Run the Docker container. Replace the ``<version>`` with one
   of the latest NEST versions (e.g., ``2.20.0``) or use
   ``latest`` for the most recent build from the source code.

.. code-block:: bash

    docker run --rm -e LOCAL_USER_ID=`id -u $USER` -v $(pwd):/opt/data -p 8080:8080 nestsim/nest:<version> notebook


4. Once completed, a link to a Jupyter Notebook will be
   generated, as shown below. You can then copy and paste the
   link into your browser.

    .. image:: ../static/img/docker_link.png
       :align: center
       :width: 1000px


5. You can now use the Jupyter Notebook as you normally
   would. Anything saved in the Notebook will be placed in the
   directory you started the Notebook from.

6. You can shutdown the Notebook in the terminal by typing
   :kbd:`Ctrl-c` twice.  Once the Notebook is shutdown the
   container running NEST is removed.


.. note::

    You can check for updates to the Docker build by typing:

    .. code-block:: bash

        docker pull nestsim/nest:<version>

.. note::

    You can also create an instance of a terminal within the container itself and, for example, run Python scripts.

    .. code-block::

        docker run --rm -it -e LOCAL_USER_ID=`id -u $USER` -v $(pwd):/opt/data -p 8080:8080 nestsim/nest:<version> /bin/bash

    See the `README <https://github.com/nest/nest-docker>`_ to find out more, but note some functionality, such as DISPLAY, will not be available.



