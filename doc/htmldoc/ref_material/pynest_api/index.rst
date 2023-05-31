.. _pynest_api:

PyNEST API
==========

The ``nest`` module contains methods and attributes to control the NEST kernel.
This interface is known as PyNEST. You can browse the various functions to use in your NEST script.




:doc:`Functions related to nodes <nest.lib.hl_api_nodes>`
----------------------------------------------------------



.. automodule::   nest.lib.hl_api_nodes

   .. autosummary::

      {% for key, value in api_dict.items() %}
          {% if key == 'hl_api_nodes' %}
              {% for item in value %}
                  {{ item }}
              {% endfor %}
          {% endif %}
      {% endfor %}

:doc:`Functions related to models <nest.lib.hl_api_models>`
-----------------------------------------------------------


.. automodule::   nest.lib.hl_api_models


   .. autosummary::

      {% for key, value in api_dict.items() %}
          {% if key == 'hl_api_models' %}
              {% for item in value %}
                  {{ item }}
              {% endfor %}
          {% endif %}
      {% endfor %}


:doc:`Functions related to parameters for nodes and synapses  <nest.lib.hl_api_types>`
---------------------------------------------------------------------------------------


.. automodule::   nest.lib.hl_api_types


   .. autosummary::

      {% for key, value in api_dict.items() %}
          {% if key == 'hl_api_types' %}
              {% for item in value %}
                  {{ item }}
              {% endfor %}
          {% endif %}
      {% endfor %}


.. automodule:: nest.lib.hl_api_info

   .. autosummary::

      get_argv
      GetStatus
      SetStatus

:doc:`Functions related to connections <nest.lib.hl_api_connections>`
---------------------------------------------------------------------


.. automodule::   nest.lib.hl_api_connections


   .. autosummary::

      {% for key, value in api_dict.items() %}
          {% if key == 'hl_api_connections' %}
              {% for item in value %}
                  {{ item }}
              {% endfor %}
          {% endif %}
      {% endfor %}


:doc:`Functions related to simulations  <nest.lib.hl_api_simulation>`
---------------------------------------------------------------------


.. automodule::   nest.lib.hl_api_simulation


   .. autosummary::

      {% for key, value in api_dict.items() %}
          {% if key == 'hl_api_simulation' %}
              {% for item in value %}
                  {{ item }}
              {% endfor %}
          {% endif %}
      {% endfor %}



:doc:`Functions related to spatial networks  <nest.lib.hl_api_spatial>`
------------------------------------------------------------------------

.. automodule::   nest.lib.hl_api_spatial


   .. autosummary::

      {% for key, value in api_dict.items() %}
          {% if key == 'lib.hl_api_spatial' %}
              {% for item in value %}
                  {{ item }}
              {% endfor %}
          {% endif %}
      {% endfor %}


.. automodule:: nest.spatial_distributions.hl_api_spatial_distributions


   .. autosummary::

      {% for key, value in api_dict.items() %}
          {% if key == 'hl_api_spatial_distributions' %}
              {% for item in value %}
                  {{ item }}
              {% endfor %}
          {% endif %}
      {% endfor %}


.. automodule:: nest.spatial.hl_api_spatial


   .. autosummary::

      {% for key, value in api_dict.items() %}
          {% if key == 'hl_api_spatial' %}
              {% for item in value %}
                  {{ item }}
              {% endfor %}
          {% endif %}
      {% endfor %}


:doc:`Functions related to math and logic  <nest.math.hl_api_math>`
-------------------------------------------------------------------

.. automodule::   nest.math.hl_api_math


   .. autosummary::

      {% for key, value in api_dict.items() %}
          {% if key == 'hl_api_math' %}
              {% for item in value %}
                  {{ item }}
              {% endfor %}
          {% endif %}
      {% endfor %}


.. automodule::   nest.logic.hl_api_logic


   .. autosummary::

      {% for key, value in api_dict.items() %}
          {% if key == 'hl_api_logic' %}
              {% for item in value %}
                  {{ item }}
              {% endfor %}
          {% endif %}
      {% endfor %}


:doc:`Functions related to randomizaton  <nest.random.hl_api_random>`
---------------------------------------------------------------------

.. automodule::   nest.random.hl_api_random


   .. autosummary::

      {% for key, value in api_dict.items() %}
          {% if key == 'hl_api_random' %}
              {% for item in value %}
                  {{ item }}
              {% endfor %}
          {% endif %}
      {% endfor %}


:doc:`Functions related to parallel computing  <nest.lib.hl_api_parallel_computing>`
------------------------------------------------------------------------------------

.. automodule::   nest.lib.hl_api_parallel_computing


   .. autosummary::

      {% for key, value in api_dict.items() %}
          {% if key == 'hl_api_parallel_computing' %}
              {% for item in value %}
                  {{ item }}
              {% endfor %}
          {% endif %}
      {% endfor %}


:doc:`Functions related NEST server  <nest.server.hl_api_server>`
-----------------------------------------------------------------

.. automodule::   nest.server.hl_api_server


   .. autosummary::

      {% for key, value in api_dict.items() %}
          {% if key == 'hl_api_server' %}
              {% for item in value %}
                  {{ item }}
              {% endfor %}
          {% endif %}
      {% endfor %}


:doc:`Functions related to voltage trace  <nest.voltage_trace>`
----------------------------------------------------------------

.. automodule::   nest.voltage_trace


   .. autosummary::

      {% for key, value in api_dict.items() %}
          {% if key == 'voltage_trace' %}
              {% for item in value %}
                  {{ item }}
              {% endfor %}
          {% endif %}
      {% endfor %}


:doc:`Functions related to raster plots <nest.raster_plot>`
------------------------------------------------------------

.. automodule::   nest.raster_plot


   .. autosummary::

      {% for key, value in api_dict.items() %}
          {% if key == 'raster_plot' %}
              {% for item in value %}
                  {{ item }}
              {% endfor %}
          {% endif %}
      {% endfor %}


:doc:`Functions related to plotting networks  <nest.visualization>`
-------------------------------------------------------------------

.. automodule::   nest.visualization


   .. autosummary::

      {% for key, value in api_dict.items() %}
          {% if key == 'visualization' %}
              {% for item in value %}
                  {{ item }}
              {% endfor %}
          {% endif %}
      {% endfor %}


:doc:`Functions related to kernel attributes  <kernel_attributes>`
------------------------------------------------------------------

.. autoclass:: nest.NestModule

   .. autosummary::

      {% for key, value in api_dict.items() %}
          {% if key == 'nestModule' %}
              {% for item in value | sort %}
                  {{ item }}
              {% endfor %}
          {% endif %}
      {% endfor %}


:doc:`Functions related to help and info  <nest.lib.hl_api_helper>`
-------------------------------------------------------------------

.. automodule::   nest.lib.hl_api_helper


   .. autosummary::

      {% for key, value in api_dict.items() %}
          {% if key == 'hl_api_helper' %}
              {% for item in value %}
                  {{ item }}
              {% endfor %}
          {% endif %}
      {% endfor %}


.. automodule:: nest.lib.hl_api_info

   .. autosummary::

      authors
      help
      helpdesk
      message
      get_verbosity
      set_verbosity
      sysinfo


.. toctree::
   :hidden:
   :glob:

   *
