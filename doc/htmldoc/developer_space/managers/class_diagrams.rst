Class diagrams
==============

Components of a Class Diagram
Classes:

Represented by rectangles divided into three parts:
Top: Class name (e.g., ModelManager).
Middle: Attributes (e.g., -node_models_: vector<Model*>).
Bottom: Methods (e.g., +initialize(bool): void).
Visibility is indicated by symbols: + for public, - for private.
Relationships (Edges):

Association: A general connection between classes. It's shown as a solid line.
Inheritance (Generalization): Indicates an "is-a" relationship. Shown as a line with a hollow triangle pointing to the superclass.
Aggregation: A "has-a" relationship where one class is a collection of others. Shown as a line with a hollow diamond at the aggregator end.
Composition: A stronger form of aggregation where the parts cannot exist without the whole. Shown as a line with a filled diamond at the whole end.
Dependency: Indicates that one class uses another. Shown as a dashed line with an arrow pointing to the class being used.
Multiplicity:

Indicates how many instances of one class are related to one instance of another class.
Notated at the ends of association lines (e.g., 1..* means one-to-many).
Understanding Edges and Endpoints
Solid Line with No Arrow: Represents a bidirectional association. Both classes are aware of each other.
Solid Line with Arrow: Indicates the direction of the association. The class at the arrow end is aware of the other class.
Dashed Line with Arrow: Represents a dependency, where one class uses another but doesn't necessarily have a direct reference to it.
Diamond Endpoint:
Hollow Diamond: Aggregation. The class at the diamond end is a collection of the other class.
Filled Diamond: Composition. The class at the diamond end owns the lifecycle of the other class.
Example in the Diagram
*ModelManager "1" -- "many" Model : manages

This denotes a one-to-many relationship where one ModelManager manages multiple Model instances.
The * indicates "many" on the Model side, while 1 indicates "one" on the ModelManager side.
*ModelManager "1" -- "1" Dictionary : uses

This denotes a one-to-one relationship where ModelManager uses a single Dictionary.
