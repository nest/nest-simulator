Kernel
======

Sequence diagram test:

```sequence
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
```
