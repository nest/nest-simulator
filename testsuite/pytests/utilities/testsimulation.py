import dataclasses

import numpy as np

import nest
import testutil


@dataclasses.dataclass
class Simulation:
    local_num_threads: int = 1
    resolution: float = 0.1
    duration: float = 8.0
    weight: float = 100.0
    delay: float = 1.0

    def setup(self):
        pass

    def simulate(self):
        nest.Simulate(self.duration)
        if hasattr(self, "voltmeter"):
            return np.column_stack(
                (self.voltmeter.events["times"], self.voltmeter.events["V_m"])
            )

    def __init_subclass__(cls, **kwargs):
        super().__init_subclass__(**kwargs)
        testutil.create_dataclass_fixtures(cls)
