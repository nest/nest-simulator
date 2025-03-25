Event Manager
=============

.. mermaid::

   classDiagram
    class EventManager {
        +schedule_event(Node node, Event event)
        +deliver_event(Event event)
    }

    EventManager --> SimulationManager : manages
    EventManager --> NodeManager : manages
