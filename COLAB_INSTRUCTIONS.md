# Running NEST Simulator in Google Colab

## Why Google Colab?

Google Colab provides **FREE computational resources** that you can use to run NEST simulations:

✅ **Free Resources**:
- CPU: Multi-core processors
- RAM: ~12-13 GB
- Storage: Temporary session storage
- GPU: Available (though NEST doesn't use GPU)

✅ **Advantages**:
- No installation on your local machine
- Access from any device with a browser
- Easy sharing with collaborators
- Pre-configured Python environment
- Save results to Google Drive

✅ **Perfect for**:
- Learning and experimenting with NEST
- Small to medium-sized simulations
- Prototyping and testing
- Educational purposes

## How to Use the Colab Notebook

### Option 1: Direct Link (Easiest)

1. **Open the notebook in Colab**: Click this button:

   [![Open In Colab](https://colab.research.google.com/assets/colab-badge.svg)](https://colab.research.google.com/github/Furkanbeyazyuz/nest-simulator/blob/claude/install-nest-simulator-011CULSS8M9xKVfddAfzNUzg/NEST_Google_Colab_Setup.ipynb)

2. **Run the installation cell**:
   - Click on the first code cell
   - Press `Shift + Enter` or click the ▶️ play button
   - Wait 2-3 minutes for NEST to install

3. **Run the examples**:
   - Continue running cells with `Shift + Enter`
   - Modify and experiment!

### Option 2: Upload Notebook Manually

1. Go to [Google Colab](https://colab.research.google.com/)
2. Click **File → Upload notebook**
3. Upload `NEST_Google_Colab_Setup.ipynb` from this repository
4. Run the cells as described above

### Option 3: From Google Drive

1. Save the notebook file to your Google Drive
2. Right-click the file → **Open with → Google Colaboratory**
3. Run the cells

## What's Included in the Notebook

### 🔧 Installation
- Automatic installation of NEST Simulator using conda
- All dependencies handled automatically
- Takes about 2-3 minutes

### 📚 Three Complete Examples

#### Example 1: Single Neuron
- Basic integrate-and-fire neuron
- Constant current injection
- Voltage recording and spike detection
- **Learn**: Basic NEST operations

#### Example 2: Small Network
- 80 excitatory + 20 inhibitory neurons
- Poisson background input
- Random connectivity
- **Learn**: Network creation and connectivity

#### Example 3: Balanced Random Network
- 1000 neurons (scaled Brunel model)
- E/I balance
- Population dynamics
- **Learn**: Large-scale network simulation

### 📊 Visualization
- Membrane potential plots
- Raster plots
- Population firing rate
- Network statistics

## Computational Resources

### Free Tier (Standard Colab)
- **Runtime**: Up to 12 hours per session
- **RAM**: ~12-13 GB
- **CPU**: Multi-core processor
- **Cost**: FREE

### Colab Pro (Optional, $9.99/month)
- **Runtime**: Up to 24 hours
- **RAM**: ~25-30 GB
- **CPU**: Faster processors
- **Priority access**: Better availability

### Recommended Network Sizes

| Network Type | Free Tier | Colab Pro |
|--------------|-----------|-----------|
| Learning/Testing | < 1,000 neurons | < 10,000 neurons |
| Small simulations | < 5,000 neurons | < 50,000 neurons |
| Research | < 10,000 neurons | < 100,000 neurons |

## Tips for Best Performance

### 1. Efficient Recording
```python
# ❌ Don't record from all neurons
nest.Connect(all_neurons, spike_recorder)

# ✅ Record from a subset
nest.Connect(all_neurons[:100], spike_recorder)
```

### 2. Reduce Output
```python
# Suppress NEST messages
nest.set_verbosity('M_WARNING')
```

### 3. Use Parallel Processing
```python
# Use multiple threads
nest.SetKernelStatus({'local_num_threads': 2})
```

### 4. Manage Session Time
- Start with small simulations to test
- Save important results regularly
- Download results before session expires

## Saving Your Results

### Save to Google Drive
```python
from google.colab import drive
drive.mount('/content/drive')

# Save figure
plt.savefig('/content/drive/MyDrive/simulation.png', dpi=300)

# Save data
import numpy as np
np.save('/content/drive/MyDrive/spike_times.npy', spike_times)
```

### Download Directly
```python
from google.colab import files

# Save and download
plt.savefig('simulation.png')
files.download('simulation.png')
```

## Common Issues and Solutions

### Issue 1: Session Disconnected
**Solution**: Colab sessions timeout after inactivity
- Keep browser tab active
- Use Colab Pro for longer sessions
- Save work frequently

### Issue 2: Out of Memory
**Solution**: Reduce simulation size
- Decrease number of neurons
- Record from fewer neurons
- Reduce simulation time
- Use Colab Pro for more RAM

### Issue 3: Installation Fails
**Solution**: Restart runtime
- **Runtime → Restart runtime**
- Re-run installation cell

## Upgrading Your Workflow

### For Larger Simulations
If you need more computational power:

1. **High-Performance Computing (HPC)**
   - Universities often provide HPC clusters
   - Can simulate millions of neurons
   - Full MPI support

2. **Cloud Computing**
   - AWS, Google Cloud, Azure
   - Pay-per-use
   - Scalable resources

3. **Local Installation**
   - Install on your own computer
   - See `INSTALLATION_NOTES.md`
   - Full control over resources

## Learning Resources

### NEST Documentation
- [NEST Homepage](https://nest-simulator.org)
- [Tutorials](https://nest-simulator.readthedocs.io/en/stable/tutorials/index.html)
- [Examples](https://nest-simulator.readthedocs.io/en/stable/examples/index.html)
- [Model Directory](https://nest-simulator.readthedocs.io/en/stable/models/index.html)

### Computational Neuroscience
- [Neuronal Dynamics Book](https://neuronaldynamics.epfl.ch/)
- [NEST Mailing List](https://www.nest-simulator.org/community/)

## Quick Start Checklist

- [ ] Open notebook in Google Colab
- [ ] Run installation cell (wait 2-3 minutes)
- [ ] Run verification cell
- [ ] Try Example 1: Single neuron
- [ ] Try Example 2: Small network
- [ ] Try Example 3: Balanced network
- [ ] Modify parameters and experiment
- [ ] Save your results

## Support

If you encounter issues:
1. Check the [NEST FAQ](https://nest-simulator.readthedocs.io/en/stable/faqs/index.html)
2. Review Colab documentation
3. Ask on [NEST mailing list](https://www.nest-simulator.org/community/)
4. Report bugs on [GitHub](https://github.com/nest/nest-simulator/issues)

---

**Ready to start?** Click the badge below to open the notebook:

[![Open In Colab](https://colab.research.google.com/assets/colab-badge.svg)](https://colab.research.google.com/github/Furkanbeyazyuz/nest-simulator/blob/claude/install-nest-simulator-011CULSS8M9xKVfddAfzNUzg/NEST_Google_Colab_Setup.ipynb)

Happy simulating! 🧠⚡
