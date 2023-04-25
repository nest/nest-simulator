import pytest
import nest
import numpy as np


@pytest.fixture
def simulation():
    return Simulation()


class Simulation:
    pass


@pytest.fixture
def noop():
    pass


# Create autoused parameter fixtures so that tests can indirectly parametrize the
# simulation fixtures nicely.
for parameter, default in (
    # Call lambdas to avoid mutable default arguments
    ("resolution", lambda: 0.1),
    ("duration", lambda: 8.0),
    ("local_num_threads", lambda: 1),
    ("amplitude", lambda: 1000.0),
    ("origin", lambda: 0.0),
    ("arrival", lambda: 3.0),
    ("weight", lambda: 100.0),
    ("delay", lambda: 1.0),
    ("pre_setup_simulation", lambda: noop),
    ("post_setup_simulation", lambda: noop),
    ("n1_params", lambda: {}),
    ("n2_params", lambda: {}),
    ("syn_spec", lambda: {}),
):
    globals()[parameter] = pytest.fixture(autouse=True, name=parameter)(
        # Bind the default to a kwarg to avoid closure lookup of last iterated `default`.
        lambda request, d=default: getattr(request, "param", d())
    )


@pytest.fixture
def run_simulation(simulation, duration):
    def run_simulation_fixture():
        nest.Simulate(duration)
        vm = simulation.voltmeter
        return np.column_stack((vm.events["times"], vm.events["V_m"]))

    return run_simulation_fixture


@pytest.fixture
def simulate(
    request,
    simulation,
    resolution,
    local_num_threads,
    pre_setup_simulation,
    setup_simulation,
    post_setup_simulation,
    run_simulation,
):
    for fix in ("setup_simulation",):
        if not callable(locals()[fix]):
            raise RuntimeError(
                f"'{fix}' fixture not callable. "
                "Please supply a fixture factory!"
            )

    nest.ResetKernel()
    nest.local_num_threads = local_num_threads
    nest.resolution = resolution

    def simulate_fixture():
        # Reset the simulation container, to prevent user errors
        simulation.__dict__ = {}
        setup_simulation()
        request.getfixturevalue(post_setup_simulation.__name__)
        return simulation, run_simulation()

    return simulate_fixture