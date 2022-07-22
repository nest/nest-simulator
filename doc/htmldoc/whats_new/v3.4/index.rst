.. _release_3.4:

What's new in NEST 3.4
======================

This page contains a summary of important breaking and non-breaking changes
from NEST 3.1 to NEST 3.2. In addition to the `release
notes on GitHub <https://github.com/nest/nest-simulator/releases/>`_,
this page also contains transition information that helps you to
update your simulation scripts when you come from an older version of
NEST.

If you transition from a version earlier than 3.4, please see our
extensive :ref:`transition guide from NEST 2.x to 3.0
<refguide_2_3>` or :ref:`release updates for previous releases in 3.x <whats_new>`.

Documentation restructuring
~~~~~~~~~~~~~~~~~~~~~~~~~~~

To improve findability of content, the documentation has been reorganized. The table of contents
is simplified and the content is grouped based on topic (neurons, synapses etc) rather than type of documentation (e.g., 'guides').


Renaming ``calibrate`` to  ``pre_run_hook``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The function :py:func:`calibrate`, used in many models, is now renamed to :py:func:`pre_run_hook` to better describe 
what the function does.
