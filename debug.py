import nest
import pandas as pd

OTHER_LABEL = "other-{}"

nest.ResetKernel()

nrn = nest.Create("parrot_neuron")

# Create two connections so we get lists back from pre_conns.get() and can build a DataFrame
nest.Connect(nrn, nrn)
nest.Connect(nrn, nrn)

pre_conns = nest.GetConnections()
print(pre_conns)
if pre_conns:
    # need to do this here, Disconnect invalidates pre_conns
    df = pd.DataFrame.from_dict(pre_conns.get()).drop(labels="target_thread", axis=1)
    df.to_csv(OTHER_LABEL.format(nest.num_processes) + f"-{nest.Rank()}.dat", index=False)  # noqa: F821

nest.Disconnect(nrn, nrn)
nest.Disconnect(nrn, nrn)
post_conns = nest.GetConnections()
assert not post_conns
