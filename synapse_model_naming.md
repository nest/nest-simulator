---
layout: index
---

# Conventions for Synapse Type Names

1. Each connector needs to have a meaningful name, _CName_ in this document
2. The filenames for the connector are lower case and according to the following set of rules:
    * Connection header: `cname_connection.h`
    * Connection implementation: `cname_connection.cpp`
3. Class names for connectors are written in CamelCase and have to follow these templates:
    * Connector class: `CNameConnector`
    * Connector properties class: `CNameConnectorProperties`
4. The connector has to be registered under the name `cname_synapse` with the simulation kernel
