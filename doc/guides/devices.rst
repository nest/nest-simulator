/** @BeginDocumentation
   Name: Device - General properties of devices.

   Description:

   Devices are elements that inject signals into a network (stimulating
   devices) or record data from it (recording devices). The specific
   properties of these classes of devices are documents in separate
   documents for each of these two classes, and the specific devices.
   This page only documents general properties.

   The only general properties for all devices are activation and
   inactivation times. These are controlled by the parameters start, stop
   and origin. Briefly speaking a device is active from start to stop,
   while origin provides a global offset, i.e., actual start and stop
   times are origin+start and origin+stop. This can be used to implement
   experiment repetitions, where only origin needs to be increased.

   The precise meaning of start and stop depends on the type of the
   device and is documented in the specific documentation pages. Generally
   speaking, any device emitting signals will emit signals in [start, stop),
   while a recording device will pick up signals with time stamps
   (start, stop].

   In general, the following must hold:
   1.  start+origin > 0
   2.  stop >= start
   3.  If stop == start, the device is inactive.

   Parameters:
   /start  - Actication time, relative to origin.
   /stop   - Inactivation time, relative to origin.
   /origin - Reference time for start and stop.

   SeeAlso: StimulatingDevice, RecordingDevice
*/
