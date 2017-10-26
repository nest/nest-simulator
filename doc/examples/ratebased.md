# Integrate rate-based models in a spiking neural network

<br>

>Hahne, J., Dahmen, D., Schuecker, J., Frommer, A., Bolten, M., Helias, M., & Diesmann, M. (2017). Integration of Continuous-Time Dynamics in a Spiking Neural Network Simulator. *Frontiers in Neuroinformatics*, 11, 34. <http://doi.org/10.3389/fninf.2017.00034>

<br><br>
### Description

TEXT FROM PAPER. Here we demonstrate a unified simulation framework that supports the **combination of spiking neuron models and rate-based units** for multi-scale modeling, enables the quantitative validation of **mean-field approaches** by spiking network simulations, and provides an **increase in reliability** by usage of the same simulation code and the same network model specifications for both model classes. 

The iterative method is enabled by default, as it is also employed for simulations with gap junctions, where it improves performance and even accuracy of the simulation, regardless of network size and parallelization. 
<br><br>

### Rate-based models

Rate-based models available in the NEST reference implementation:

|Gain Model | \\(\Phi(x)\\) or \\(\Psi(x)\\)|
|---|---|
|[`lin_rate`](/path/to/index)|\\(g * x\\) with \\(g \in &#8477; \\)|
|[`tanh_rate`](/path/to/index)| tanh\\((g * x)\\) with \\(g \in &#8477;\\)|
|[`thresholdlin_rate`](/path/to/index) | \\(g * (x - \Theta) * H(x - \Theta)\\) with \\(g \in &#8477;\\)|

**Usage:** `<gain_model>_ipn`
<br>
#### Additional rate model 
[`siegert_neuron`](/path/to/index/sieger_neuron) - see section on [mean-field analysis of complex networks](/section/below) below.

<br><br>

### See Also

[RELATED_EXAMPLE](/path/to/link.md) with explanation why it is related

<br><br>

## Example using an excitatory-inhibitory network of linear rate units

Create an excitatory-inhibitory network of a linear rate unit. There is no fundamental difference to scripts for the simulation of spiking neural networks.

Prototypical network model of excitatory and inhibitory units (Equation 20 in Hahne et al. 2017): 

$$
{\tau dX^{i}(t) = \left( {- X^{i} + {\sum\limits_{j = 1}^{N}w^{ij}}X^{j}(t)} \right)dt + \sqrt{\tau}\sigma} 
$$

Due to the linearity of the model, the cross-covariance between units \\(i\\) and \\(j\\) can be calculated (Equation 21 in Hahne et al. 2017):

$$
{c(t) = {\sum\limits_{i,j}\frac{v^{i\text{T}}\sigma^{2}v^{j}}{\lambda_{i} + \lambda_{j}}}u^{i}u^{j\text{T}}\left( {H(t)\frac{1}{\tau}e^{- \lambda_{i}\frac{t}{\tau}} + H( - t)\frac{1}{\tau}e^{\lambda_{j}\frac{t}{\tau}}} \right)}
$$

where \\(H\\) denotes the Heaviside function. The \\(\lambda_i\\) indicate the eigenvalues of the matrix \\(\mathbb{1} - W\\) corresponding to the \\(i\\)-th left and right eigenvectors *v^i^* and *v^j^*.
<br><br>
### Code

Import necessary modules

```python
import  nest
```
**Disable** usage of **waveform-relaxation** method. This is advisable for simulations on local workstations or clusters, where the waveform-relaxation method typically does not improve performance

```python
nest.SetKernelStatus({'resolution':  h,  'use_wfr':  False})
```
<br>
**Create** **rate units** and **recording device**:


```python
	n_e  =  nest.Create('lin_rate_ipn',  NE,
	                    params  =  {'linear_summation':  True,
	                    'mean':  mu,  'std':  sigma,  'tau':  tau})
	n_i  =  nest.Create('lin_rate_ipn',  NI,
	                    params  =  {'linear_summation':  True,
	                    'mean':  mu,  'std':  sigma,  'tau':  tau})
	mm  =  nest.Create('multimeter',  params  =  {'record_from':  ['rate'],
                       'interval':  h,  'start':  T_start})
```

The parameter [`linear_summation`](/path/to/index/linear_summation) characterizes the type of nonlinearity (\\(\Phi\\) = True or \\(\Psi\\) = False) of the rate model. The [`record_from`](/path/to/index/record_from) parameter needs to be set to [`rate`](/path/to/index/rate) to pick up the corresponding state variable. 

<br> 
Specify **synapse** and **connection** dictionaries:
```python
	syn_e  =  {'weight':  w,  'delay':  d,
	           		'model':  'delay_rate_connection'}
	syn_i  =  {'weight':  -g  *  w,  'delay':  d,
	                'model':  'delay_rate_connection'}
	conn_e  =  {'rule':  'fixed_outdegree',  'outdegree':  KE}
	conn_i  =  {'rule':  'fixed_outdegree',  'outdegree':  KI}
```
As this particular network model includes delayed rate connections, the synapse model [`delay_rate_connection`](/path/to/index/delay_rate_connection) is chosen.
> In order to create instantaneous rate connections instead, you can change the synapse model to [`rate_connection`](/path/to/index/rate_connection) and remove the parameter [`delay`](path/to/index/delay) from the synapse dictionary.

<br>
**Connect** ** rate units** and the ** recording device** to those rate units:
```python
	nest.Connect(n_e,  n_e,  conn_e,  syn_e)
	nest.Connect(n_i,  n_i,  conn_i,  syn_i)
	nest.Connect(n_e,  n_i,  conn_i,  syn_e)
	nest.Connect(n_i,  n_e,  conn_e,  syn_i)
	
	nest.Connect(mm,  n_e  +  n_i)
	
	nest.Simulate(T)
```
<br><br>
### Output
...


***
<br><br>
## Mean-field analysis of complex networks

Here single rate units of type [`siegert_neuron`](/path/to/index/siegert_neuron) represent an entire population of spiking neurons. The units are coupled by connections of type [`diffusion_connection`](/path/to/index/diffusion_connection). This connection type is identical to type [`rate_connection`](/path/to/index/rate_connection) for instantaneous rate connections except for the two parameters [`drift_factor`](/path/to/index/drift_factor) and [`diffusion_factor`](/path/to/index/diffusion_factor) substituting the parameter weight. 
The model used in this example (Equation 30 from Hahne et al. 2017):

$$
{\tau\frac{\text{d}X_{\alpha}}{dt} = - X_{\alpha} + \Phi_{\alpha}(X)\operatorname{}} 
$$
<br>
### Code
```python
import nest
```
Create one `siegert_neuron` for the excitatory and inhibitory population:

```python
s_ex = nest.Create('siegert_neuron', 1)
s_in = nest.Create('siegert_neuron', 1)
```

Create connections origination from the excitatory unit:

```python
syn_e = {'drift_factor': tau_m * K * w, 
         'diffusion_factor': tau_m * K * w * w,
         'model': 'diffusion_connection'}

nest.Connect(s_ex, s_ex + s_in, 'all_to_all', syn_e)
```
<br><br>

### Output
...

 definitions of the parameters (h,NE,NI,mu,sigma,tau,T_start,w,d,g,KE,KI,T).
