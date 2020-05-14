:orphan:

The SetAcceptableLatency command
--------------------------------

In the introduction we showed how spike events are delivered as quickly
as possible, then queued at the receiving node until it is time for them
to be applied. MUSIC does its very best to make the transmission time
look like zero time, so the total in-simulation delay from the source to
the target is the delay from the input proxies to the targets. The spike
events have to be delivered through the MUSIC connection to the
recipient nodes no later than that time.

In practice, MUSIC can — and, when you have recurrent connections, has
to — use a bit of in-simulation time to deliver events. It’s often more
effective to send several steps worth of events in one packet than to
send one packet of data per time step. In order to do this, MUSIC needs
to know how much time is available for such delays.

The ``SetAcceptableLatency`` command specifies this time
for each input port. A simulation model may add, delete or alter
connections at any point during a simulation, and this may happen
locally in a large, distributed network, so it is in practice not
possible for MUSIC to figure out the minimum time by itself. We
(hopefully) know what our code will be doing, so we set this manually.

If the acceptable latency is longer than the minimum delay across the
connections from an input proxy, spikes may be delivered to those
targets later than they were supposed to be, thereby breaking the
simulation in subtle or obvious ways. In simple feed-forward cases like
in our example, MUSIC does not need to use any of that extra delay so
things may work fine, but in complex simulations with recurrent
connections this may no longer be the case.

.. note::

   Please note that MUSIC and the recording backend for Arbor are mutually exclusive
   and cannot be enabled at the same time.


