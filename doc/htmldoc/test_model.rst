
Test model
==========

.. raw:: html

   <h2>Model Selector</h2>
    <label for="tag-select">Select Tags:</label>
    <select id="tag-select" multiple>
        <!-- Options will be populated here by JavaScript -->
    </select>
    <button onclick="updateModelDisplay()">Show Models</button>
    <div id="model-list"></div>
    <script src="script.js"></script>

A-Z list
--------

{% for key, values in tag_dict.items() %}

{% if "neuron" in key %}
<div class="color-shape {{ key | replace("_AND_", " ") }}">
{% for value in values %}
<a href="{{ value  }}.html">
{{ value }}
{% endfor %}</a>
{% endif %}

{% if "synapse" in key %}
<div class="color-shape {{ key | replace("_AND_", " ") }}">
{% for value in values %}
<a href="{{ value  }}.html">
{{ value }}
{% endfor %}</a>
{% endif %}

{% if "device" in key %}
<div class="color-shape {{ key | replace("_AND_", " ") }}">
{% for value in values %}
<a href="{{ value  }}.html">
{{ value }}
{% endfor %}</a>
{% endif %}
</div>
{% endfor %}

Neurons
-------

print each tag that also contains neurons

Tags
neuron
current-based | integrate-and-fire | conductance-based | precise | e-prop | point-process | rate | parrot | adaptive threshold |
instantaneous rate | continuous delay | compartmental |

synapse
spike-timing-dependent plasticity | Hill-Tononi | Clopath | gap junction | static | short-term plasticity |

astrocyte

device
MUSIC | generator | recorder |


if tag clicked
load models for tag

'neuron', 'point process', 'current-based', 'device', 'rate', 'MUSIC', 'synapse', 'spike-timing-dependent plasticity',
'continuous delay', 'integrate-and-fire', 'conductance-based', 'short-term plasticity', 'precise',
'generator', 'detector', 'parrot', 'recorder', 'adaptive threshold', 'Clopath plasticity',
'spike', 'compartmental model', 'Hill-Tononi plasticity', 'instantaneous rate', 'instantaneous',
'binary', 'e-prop plasticity', 'astrocyte', 'Hodgkin-Huxley', 'static', 'gap junction']


.. raw:: html

  <h1>Isotope - combination filters</h1>

  <div class="filters">

    <div class="ui-group">
      <h3>Model type</h3>
      <div class="button-group js-radio-button-group" data-filter-group="color">
        <button class="button is-checked" data-filter="">any</button>
        <button class="button" data-filter=".neuron">neuron</button>
        <button class="button" data-filter=".synapse">synapse</button>
        <button class="button" data-filter=".device">device</button>
      </div>
    </div>

    <div class="ui-group">
      <h3>properties</h3>
      <div class="button-group js-radio-button-group" data-filter-group="size">
        <button class="button is-checked" data-filter="">any</button>
        <button class="button" data-filter=".conductance-based">conductance-based</button>
        <button class="button" data-filter=".current-based">current-based</button>
      </div>
    </div>
    <div class="ui-group">
      <h3>properties</h3>
      <div class="button-group js-radio-button-group" data-filter-group="shape">
        <button class="button" data-filter=".adaptive_threshold">adaptive_threshold</button>
        <button class="button" data-filter=".precise">Precise</button>
        <button class="button" data-filter=".binary">Binary</button>
        <button class="button" data-filter=".generator">generator</button>
        <button class="button" data-filter=".recorder">recorder</button>

      </div>
    </div>
   </div>


   <div class="grid">
    {% for key, values in tag_dict.items() %}

    <div class="color-shape {{ key | replace("_AND_", " ") }}">
    <a href=" value  .html">
    {{ key }}
    </div>
    {% endfor %}

   </div>
