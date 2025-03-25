Model Manager
=============

The provided C++ code defines a ModelManager class within the nest namespace, which is part of a larger simulation
framework. The ModelManager is responsible for managing node and connection models,
including their registration, copying, and configuration. It handles the initialization
and finalization of models, sets default parameters, and manages memory for node models.
The class also provides functionality for calibrating models and retrieving their status.
The code is designed to work in a multi-threaded environment, ensuring that
models are correctly managed across different threads. The ModelManager plays a crucial
role in the simulation framework by providing a centralized way to handle various models and their configurations.

.. image:: /static/img/model_manager.svg

.. mermaid::

   classDiagram
    class ModelManager {
        +registerModel(name: string, model: Model)
        +getModel(name: string) : Model
        +initialize()
        -modeldict: Dictionary
        -node_models: map<string, Model>
        -connection_models: map<string, ConnectionModel>
        -proxynode_model: Model
        -proxy_nodes: list<Node>
    }
    ModelManager --|> ManagerInterface



.. doxygenclass:: nest::ModelManager
   :members:

.. .. mermaid::

  classDiagram
    class ModelManager {
        -node_models_: vector<Model*>
        -connection_models_: vector<vector<ConnectionModel*>>
        -modeldict_: Dictionary*
        -synapsedict_: Dictionary*
        -proxynode_model_: GenericModel<proxynode>*
        -proxy_nodes_: vector<vector<Node*>>
        -model_defaults_modified_: bool
        +initialize(bool): void
        +finalize(bool): void
        +get_num_connection_models(): size_t
        +set_status(DictionaryDatum): void
        +get_status(DictionaryDatum): void
        +copy_model(Name, Name, DictionaryDatum): void
        +register_node_model_(Model*): size_t
        +copy_node_model_(size_t, Name, DictionaryDatum): void
        +copy_connection_model_(size_t, Name, DictionaryDatum): void
        +set_model_defaults(Name, DictionaryDatum): bool
        +set_node_defaults_(size_t, DictionaryDatum): void
        +set_synapse_defaults_(size_t, DictionaryDatum): void
        +get_node_model_id(Name): size_t
        +get_synapse_model_id(string): size_t
        +get_connector_defaults(synindex): DictionaryDatum
        +clear_node_models_(): void
        +clear_connection_models_(): void
        +calibrate(TimeConverter): void
        +memory_info(): void
        +create_proxynode_(size_t, int): Node*
    }

    class Model {
        +get_name(): string
        +set_model_id(size_t): void
        +clone(string): Model*
        +set_status(DictionaryDatum): void
    }

    class Dictionary {
        +lookup(Name): Token
        +insert(Name, size_t): void
        +clear(): void
    }

    class ConnectionModel {
        +clone(string, size_t): ConnectionModel*
        +set_status(DictionaryDatum): void
        +calibrate(TimeConverter): void
    }

    class Node {
        +set_model_id(int): void
    }

    class GenericModel {
        +create(size_t): Node*
    }

    ModelManager "1" *-- "many" Model : manages
    ModelManager "1" *-- "1" Dictionary : uses
    ModelManager "1" *-- "many" ConnectionModel : manages
    ModelManager "1" *-- "1" GenericModel : uses
    ModelManager "1" *-- "many" Node : manages
