.. _iomanager:

io manager
==========



{% for key in io_manager_list %}
{% for a_value in io_manager_list[key] %}

* {{ a_value }}



{% endfor %}
{% endfor %}

{% for item in cpp_class_list %}
{% if 'IOManager' in item %}

* :cpp:class:`{{ item }}`


.. doxygenclass:: {{item}}
   :members:
   :allow-dot-graphs:

{% endif %}
{% endfor %}


