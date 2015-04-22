---
layout: index
---

[« Back to the index](index)

<hr>

# Conventions for synapse model names

1. Each connector needs to have a meaningful name, _CName_ in this document
2. The filenames for the connector are lower case and according to the following set of rules:
    1. Connection header: `cname_connection.h`
    2. Connection implementation: `cname_connection.cpp`
3. Class names for connectors are written in CamelCase and have to follow these templates:
    1. Connector class: `CNameConnector`
    2. Connector properties class: `CNameConnectorProperties`
4. The connector has to be registered under the name `cname_synapse` with the simulation kernel
