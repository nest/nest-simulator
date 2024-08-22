Breathe test
===============

List of managers
----------------


{% for item in cpp_class_list %}
{% if 'Manager' in item %}

* :cpp:class:`{{ item }}`


{% endif %}
{% endfor %}



devices
-------


.. .. doxygengroup:: Devices
   :project: NEST Simulator
   :members:
