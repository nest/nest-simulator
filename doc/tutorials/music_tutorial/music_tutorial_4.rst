The pymusic interface
---------------------

MUSIC has recently aqcuired a `plain Python interface <https://github.com/INCF/MUSIC/tree/master/pymusic>`_
to go along with the C++ API. If you just want to connect with a simulation
rather than adding MUSIC capability to a simulator, this Python interface can
be a lot more convenient than C++. You have Numpy, Scipy and other high-level
libraries available, and you don’t need to compile anything.

The interface is closely modelled on the C++ API; indeed, the steps to
use it is almost exactly the same. You can mostly refer to the :doc:`C++
description <music_tutorial_3>` for explanation. Below we will only highlight the
differences to the C++ API. The full example code is in the
``pymusic`` directory in the MUSIC repository.

.. code-block:: python
    :linenos:

    #!/usr/bin/python
    import music

    [ ... ]

    outdata.map(music.Index.GLOBAL,
        base=firstId,
            size=nlocal)

    [ ...]

    runtime = setup.runtime(TICK)
    tickt = runtime.time()
    while tickt < simtime:

        for i in range(width):
        send_poisson(outdata, RATE, tickt, i)

        runtime.tick()
        tickt = runtime.time()

The sending code is almost completely identical to its C++ counterpart.
Make sure python is used as interpreter for the code (and make sure this
file is executable). Import music in the expected way.

Unlike the C++ API, the index is not an object, but simply a label
indicating global or local indexing. The ``map()`` call
thus need to get the first ID and the number of elements mapped to this
rank directly. Also note that the ``map()`` functions have
a somewhat unexpected parameter order, so it’s best to use named
parameters just in case.

The runtime looks the same as the C++ counterpart as well. We get the
current simulation time, and repeatedly send new sets of events as long
as the current time is smaller than the simulation time.

.. code-block:: python
    :linenos:

    import Queue

    in_q = Queue.Queue()

    # Our input handler function
    def inhandler(t, indextype, channel_id):
        in_q.put([t, channel_id])

        [ ... ]

    indata.map(inhandler,
            music.Index.GLOBAL,
            base=firstId,
            size=nlocal,
            accLatency=IN_LATENCY)

    tickt = runtime.time()
    while tickt < simtime:

        runtime.tick()
        tickt = runtime.time()

        while not in_q.empty():
            ev = in_q.get()
            f.write("{0}\t{1:8.4f}\n".format(ev[1], ev[0]))

Here is the structure for the receiving process, modelled on the C++
code. We use a Python ``Queue``  class to implement
our event queue.

The input handler function has signature
``(float time, int indextype, int channel_id)``. The
``time`` and ``channel_id`` are the event
times and IDs as before. The ``indextype`` is the type of
the map index for this input and is ``music.Index.LOCAL``
or ``music.Index.GLOBAL``.

The ``map()`` function keyword for accepatable latency is
``accLatency``, and the ``maxBuffered``
keyword we mentioned in the previous section is, unsurprisingly,
``maxBuffered``. The runtime is, again, the same as for
C++.

As the ``pymusic`` bindings are still quite new the
documentation is still lagging behind. This quick introduction should
nevertheless bee enough for you to get going with the bindings. And
should you need further help, the authors are only an email away.


