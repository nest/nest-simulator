Stimulation and recording devices
=================================

Devices are elements that inject signals into a network (stimulating
devices) or record data from it (recording devices). The specific
properties are documented in the respective documentation for each
device. This page documents general device properties.

The only general properties for all devices are activation and
inactivation times. These are controlled by the following parameters:

- *start*: actication time, relative to origin
- *stop*: inactivation time, relative to origin
- *origin*: reference time for start and stop.


Briefly speaking, a device is active from *start*
to *stop*, while *origin* provides a global offset, i.e., actual start
and stop times are *origin*+*start* and *origin*+*stop*,
respectively. The main use for these properties is to ease repetetive
experiments by only increasing the value of *origin* for each
iteration.

The precise meaning of *start* and *stop* depends on the exact type of
the device and is documented in the specific documentation pages. In
general, any device emitting signals will emit signals only in the
interval [start, stop), while recording devices will pick up signals
only if the time stamps fall within (start, stop].

As a general rule, the following must hold:
1.  start+origin > 0
2.  stop >= start
3.  If stop == start, the device is inactive.
