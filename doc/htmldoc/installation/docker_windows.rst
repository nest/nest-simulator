.. _windows_docker:

Install NEST on Windows with Docker
===================================


The NEST simulator does not officially support Windows, but there are several workarounds to make NEST work on this platform,
including using Linux subsystems or Docker, the latter shown here.

This is a tried and tested solution with minimal preparation;
the setup will take approximately 20 minutes and requires a system restart.

1.  Download `Docker Desktop <https://www.docker.com/products/docker-desktop/>`_ for your Windows system and follow the instructions to install
    it.

    You will have to create a user account and restart your system after installation.

2.  Open Docker Desktop and in the search bar at the top type "NEST Simulator". You will find the most recent NEST build.

    Select it and click "pull".
    (This will show up as nest/nest-simulator with a drop-down menu that should automatically show the most recent, stable build.)

3.  You will have to run NEST Simulator with an internal server so you can use it with Python.

    Open a Windows console (type "cmd" in the search bar and press enter), and copy and paste this command into the console:

    ``docker run -it -p 8888:8888 --name nest-jupyter nest/nest-simulator:3.8 /bin/bash``

    (This is the command for version 3.8; change the version ID if neccessary)

    Press enter and wait until the process is completed. You now have NEST running with an internal server.
    Keep the console open for the next step.


4.  NEST includes Jupyter for easy Python programming. To access this service, enter this command in the console:

    ``jupyter notebook --ip=0.0.0.0 --no-browser --allow-root"``  and press enter

    You will receive a longer response that includes a line similar to this:
    ::

       (...)Or copy and paste one of these URLs:
        http://127.0.0.1:8888/?token=abc123...456

    Copy the URL and paste it to your browser.

    This should open Jupyter with NEST Simulator enabled.

5.  After this initial setup, you can start NEST Simulator and jupyter-nest by

    - running docker,
    - navigating to "containers"
    - clicking run on both "nest/nest_simulator:3.8" and "nest-jupyter"

    .. note::

     If the above steps to run NEST does not work, you can access it through the console

     Enter: "docker start nest-jupyter" press enter
     Enter: "docker exec -it nest-jupyter bash" press enter, stay in this window and
     Enter: "jupyter notebook --ip=0.0.0.0 --no-browser --allow-root" press enter
