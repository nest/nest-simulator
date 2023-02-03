Kernel
======

.. mermaid::

   sequenceDiagram
      user -> simulation_manager: prepare()
      simulation_manager -> node_manager: prepare_nodes()
      node_manager -> node: init()
      node_manager -> node: pre_run_hook()
      Note left of node_manager: state:\nprepared
      user -> simulation_manager: run()
      Note right of simulation_manager: 1
      user -> simulation_manager: run()
      Note left of user: store results\n...
      user -> simulation_manager: cleanup()

.. mermaid::

   sequenceDiagram
      participant Alice
      participant Bob
      Alice->John: Hello John, how are you?
      loop Healthcheck
          John->John: Fight against hypochondria
      end
      Note right of John: Rational thoughts <br/>prevail...
      John-->Alice: Great!
      John->Bob: How about you?
      Bob-->John: Jolly good!

