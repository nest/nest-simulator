import nest
import pytest


def test_ticket_451():
    """
    Guard against infinite loops in Random*Connect.

    This test ensures that conditions leading to infinite loops in
    random connection routines are caught when restrictions on multapses
    and autapses are taken into account.

    Author: Hans Ekkehard Plesser, 2010-09-20
    """

    # Create three iaf_psc_alpha neurons
    nodes = nest.Create("iaf_psc_alpha", 3)
    first = nodes[:1]  # Take first node

    # Test fixed indegree connections
    with pytest.raises(Exception):
        nest.Connect(
            nodes, first, {"rule": "fixed_indegree", "indegree": 4, "allow_multapses": False, "allow_autapses": True}
        )

    nest.Connect(
        nodes, first, {"rule": "fixed_indegree", "indegree": 3, "allow_multapses": False, "allow_autapses": True}
    )
    nest.Connect(
        nodes, first, {"rule": "fixed_indegree", "indegree": 2, "allow_multapses": False, "allow_autapses": True}
    )

    with pytest.raises(Exception):
        nest.Connect(
            nodes, first, {"rule": "fixed_indegree", "indegree": 4, "allow_multapses": False, "allow_autapses": False}
        )

    nest.Connect(
        nodes, first, {"rule": "fixed_indegree", "indegree": 2, "allow_multapses": False, "allow_autapses": False}
    )
    nest.Connect(
        nodes, first, {"rule": "fixed_indegree", "indegree": 1, "allow_multapses": True, "allow_autapses": True}
    )

    # Test fixed outdegree connections
    with pytest.raises(Exception):
        nest.Connect(
            first, nodes, {"rule": "fixed_outdegree", "outdegree": 4, "allow_multapses": False, "allow_autapses": True}
        )

    nest.Connect(
        first, nodes, {"rule": "fixed_outdegree", "outdegree": 3, "allow_multapses": False, "allow_autapses": True}
    )
    nest.Connect(
        first, nodes, {"rule": "fixed_outdegree", "outdegree": 2, "allow_multapses": False, "allow_autapses": True}
    )

    with pytest.raises(Exception):
        nest.Connect(
            first, nodes, {"rule": "fixed_outdegree", "outdegree": 4, "allow_multapses": False, "allow_autapses": False}
        )

    nest.Connect(
        first, nodes, {"rule": "fixed_outdegree", "outdegree": 2, "allow_multapses": False, "allow_autapses": False}
    )
    nest.Connect(
        first, nodes, {"rule": "fixed_outdegree", "outdegree": 1, "allow_multapses": True, "allow_autapses": False}
    )
