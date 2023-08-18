PyNEST API listing
==================

{% for key, value in api_dict.items() -%}


:doc:`{{ key }}`
--------------------------------------------

.. automodule:: {{ key }}
   :noindex:

   .. autosummary::

      {% for item in value %}
           {{ item }}
       {% endfor %}

{%- endfor %}



.. toctree::
   :hidden:
   :glob:

   *
