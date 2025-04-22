"""
Create an archtictural DOT diagram from YAML input.
"""

import yaml
from graphviz import Digraph
from sphinx.application import Sphinx


def add_subgraph(graph, data):
    """
    Recursively add subgraphs and nodes to the graph,
    preserving the hierarchy and attributes from the YAML.
    """
    if isinstance(data, list):
        i = 0
        while i < len(data):
            item = data[i]
            if "name" in item:
                name = item["name"]
                # Gather all subsequent dicts without 'name' as attributes
                j = i + 1
                attrs = {}
                # Collect all attributes for this subgraph
                while j < len(data) and "name" not in data[j]:
                    attrs.update(data[j])
                    j += 1
                # Create subgraph
                with graph.subgraph(name=f"cluster_{name}") as sg:
                    sg.attr(label=name)
                    # Here we define some attributes for all subgraphs
                    sg.attr(fontname="Helvetica,Arial,sans-serif", fontcolor="white", style="filled")
                    # Apply all collected attributes to the subgraph
                    for key, value in attrs.items():
                        if key != "node" and key != "subgraph":
                            sg.attr(**{key: value})
                        # Add nodes
                    nodes = attrs.get("node", [])
                    if isinstance(nodes, str):
                        nodes = [nodes]
                    for n in nodes:
                        sg.node(n)
                    # Recurse into nested subgraphs
                    if "subgraph" in attrs:
                        add_subgraph(sg, attrs["subgraph"])
                i = j
            else:
                i += 1


def yaml_to_graphviz(yaml_str):
    """
    Convert a YAML string to a Graphviz Digraph.

    Parameters:
    yaml_str (str): The YAML string to convert.

    Returns:
    Digraph: The generated Graphviz Digraph.
    """
    try:
        data = yaml.safe_load(yaml_str)
    except yaml.YAMLError as e:
        raise ValueError(f"Error parsing YAML: {e}") from e
    if not isinstance(data, dict):
        raise ValueError("YAML data must be a dictionary")

    dot = Digraph(
        "G",
        node_attr={
            "fontname": "Helvetica,Arial,sans-serif",
            "shape": "box",
            "fontcolor": "black",
            "style": "filled",
            "fillcolor": "white",
        },
    )
    dot.attr(
        rankdir="LR",
        bgcolor="#ff6633",
        label="Simulation Run",
        labelloc="t",
        fontcolor="white",
        fontname="Helvetica,Arial,sans-serif",
    )
    for main_section in data.values():
        add_subgraph(dot, main_section["subgraph"])
    # dot.render(output_file, format='svg', cleanup=True)
    return dot


# --- Use the full YAML ---
GRAPH_YAML = """
simulation_run:
  subgraph:
    - name: simulate_timer
    - fillcolor: '#072f42'
      subgraph:
        - name: OMP_Parallel
        - fillcolor: '#0E6A93'
          subgraph:
            - name: time_driven_loop
            - fillcolor: '#4b210c'
              subgraph:
                - name: deliver_spike_data_timer
                - fillcolor: '#072f42'
                  node: deliver_events
                - name: if_detailed_timers_enabled
                - fillcolor: 'darkgrey'
                  node: OMP_Barrier
                - name: update_timer
                - fillcolor: 'darkgrey'
                  node:
                    - node_update
                    - OMP_Barrier
            - node: OMP_barrier
            - name: OMP_Master
            - fillcolor: '#0E6A93'
              subgraph:
                - name: gather_spike_data_timer
                - fillcolor: '#072f42'
                  subgraph:
                    - name: gather_spike_data
                    - fillcolor: '#4b210c'
                      subgraph:
                        - name: do_once_repeat_if_failed
                        - fillcolor: 'darkgrey'
                          subgraph:
                          - name: if_buffers_can_shrink
                            node: shrink_MPI_buffer_size
                          - name: collocate_spike_data_timer
                          - fillcolor: '#072f42'
                            node:
                              - reset_MPI_buffer_markers
                              - collocate_spike_data
                              - set_end_markers_in_MPI_buffer
                          - name: communicate_spike_data_timer
                          - fillcolor: '#072f42'
                            node: communicate_spike_data
            - node: advance_time
"""


def yaml2dot(app, env, docnames):
    """
    Sphinx event handler to convert YAML data to a Graphviz DOT file.
    """

    dot = yaml_to_graphviz(GRAPH_YAML)
    with open("simulation_run.dot", "w", encoding="UTF-8") as f:
        f.write(dot.source)


def setup(app):
    """
    Setup function for the Sphinx extension.

    This function connects the `yaml2dot` event handler to the
    `env-before-read-docs` event, which is triggered before Sphinx reads
    the documentation files.
    """

    app.connect("env-before-read-docs", yaml2dot)

    return {
        "version": "0.1",
        "parallel_read_safe": True,
        "parallel_write_safe": True,
    }
