

Collecting spikes from neurons
##############################

The most universal collector device is the ``spike_detector``, which
collects and records all *spikes* it receives from neurons that are
connected to it. Each spike received by the spike detector is
immediately handed over to the selected recording backend for further
processing.

Any node from which spikes are to be recorded, must be connected to
the spike detector using the standard ``Connect`` command. The
connection ``weights`` and ``delays`` are ignored by the spike detector, which
means that the spike detector records the time of spike creation
rather than that of their arrival.

::

   >>> neurons = nest.Create('iaf_psc_alpha', 5)
   >>> sd = nest.Create('spike_detector')
   >>> nest.Connect(neurons, sd)

The call to ``Connect`` will fail if the connection direction is reversed (i.e., connecting
*sd* to *neurons*).

