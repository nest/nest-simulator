Integrating neural models using exact integration 
=================================================

The simple integrate-and fire model
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

For the simple integrate-and-fire model the voltage :math:`V` is given as a solution of the equation:

.. math::
    C\frac{dV}{dt}=I.

This is just the derivate of the law of capacitance :math:`Q=CV`. When an input current is applied, the membrane voltage increases with time until it reaches a constant threshold :math:`V_{\text{th}}`, at which point a delta function spike occurs.

A shortcoming of the simple integrate-and-fire model is that it implements no time-dependent memory. If the model receives a below-threshold signal at some time, it will retain that voltage boost until it fires again. This characteristic is not in line with observed neuronal behavior.

The leaky integrate-and fire model
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

In the leaky integrate-and-fire model, the memory problem is solved by adding a "leak" term :math:`\frac{-1}{R}V` (:math:`R` is the resistance and :math:`\tau=RC`) to the membrane potential:

.. math::
    \frac{dV}{dt}=\frac{-1}{\tau}V+\frac{1}{C}I.
    :label: membrane

This reflects the diffusion of ions that occurs through the membrane when some equilibrium is not reached in the cell.

Solving a  homogeneous linear differential equation
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

To solve :math:numref:`membrane` we start by looking at a simpler differential equation:

.. math::
    \frac{df}{dt}=af\text{, where } f:\mathbb{R}\to\mathbb{R} \text{ and } a\in\mathbb{R}.

Here the solution is given by :math:`f(t)=e^{at}`.

Solving a non-homogeneous linear differential equation
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
When you add another function :math:`g` to the right hand side of our linear differential equation,

.. math::
    \frac{df}{dt}=af+g

this is now a non-homogeneous differential equation. Things (can) become more complicated.

Solving it with variation of constants
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

This kind of differential equation is usually solved with "variation of constants" which gives us the following solution:

.. math::
    f(t)=e^{ct}\int_{0}^t g(s)e^{-cs}ds.

This is obviously not a particularly handy solution since calculating the integral in every step is very costly.

Solving it with exact integration
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

With exact integration, these costly computations can be avoided. 

Restrictions to :math:`g`
------------------------
But only for certain functions :math:`g`! I.e. if :math:`g` satisfies (is a solution of):

.. math::
    \left(\frac{d}{dt}\right)^n g= \sum_{i=1}^{n}a_i\left(\frac{d}{dt}\right)^{i-1} g

for some :math:`n\in \mathbb{N}` and a sequence :math:`(a_i)_{i\in\mathbb{N}}\subset \mathbb{R}`.

For example this would be the case for :math:`g=\frac{e}{\tau_{syn}}t e^{-t/\tau_{\text{syn}}}` (an alpha funciton), where :math:`\tau_{\text{syn}}` is the rise time.

Reformulating the problem
^^^^^^^^^^^^^^^^^^^^^^^^^

The non-homogeneous differential equation is reformulated as a multidimensional homogeneous linear differential equation:

.. math::
    \frac{d}{dt}y=Ay

where 

.. math::
    A=\begin{pmatrix}
        a_{n}  & a_{n-1} & \cdots & \cdots & a_1    & 0 \\
        1      & 0       & \cdots & 0      & 0      & 0 \\
        0      & \ddots  & \ddots & \vdots & \vdots & \vdots \\
        \vdots & \ddots  & \ddots & 0      & 0      & 0 \\
        0      & 0       & \ddots & 1      & 0      & 0 \\
        0      & 0       & \cdots & 0      & \frac{1}{C} & -\frac{1}{\tau} \\
    \end{pmatrix}

by choosing :math:`y_1,...,y_n` canonically as:

.. math::
    \begin{align*}
        y_1 &= \left(\frac{d}{dt}\right)^{n-1}g\\
        \vdots &= \vdots\\
        y_{n-1} &= \frac{d}{dt}g\\
        y_{n} &= g\\
        y_{n+1} &= f.
    \end{align*}

This makes ist very easy to determine the solution as

.. math::
    y(t)= e^{At}y_0

and 

.. math::
    y_{t+h}=y(t+h)=e^{A(t+h)}\cdot y_0=e^{Ah}\cdot e^{At}\cdot y_0=e^{Ah}\cdot y_t.

This means that once we have calculated :math:`A`, propagation consists of multiplications only.

Example: The leaky integrate and fire model with alpha-function shaped inputs (iaf_psc_alpha)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The dynamics of the membrane potential :math:`V` is given by:

.. math::
    \frac{dV}{dt}=\frac{-1}{\tau}V+\frac{1}{C}I

where :math:`\tau` is the membrane time constant and :math:`C` is the capacitance. :math:`I` is the sum of the synaptic currents and any external input:

Postsynaptic currents are alpha-shaped, i.e. the time course of the synaptic current :math:`\iota` due to one incoming spike is

.. math::
    \iota (t)= \frac{e}{\tau_{syn}}t e^{-t/\tau_{\text{syn}}}.

The total input :math:`I` to the neuron at a certain time :math:`t` is the sum of all incoming spikes at all grid points in time :math:`t_i\le t` plus an additional piecewise constant external input :math:`I_{\text{ext}}`:

.. math::
    I(t)=\sum_{i\in\mathbb{N}, t_i\le t }\sum_{k\in S_{t_i}}\hat{\iota}_k \frac{e}{\tau_{\text{syn}}}(t-t_i) e^{-(t-t_i)/\tau_{\text{syn}}}+I_{\text{ext}}

:math:`S_t` is the set of indices that deliver a spike to the neuron at time :math:`t`, :math:`\tau_{\text{syn}}` is the rise time and :math:`\iota_k` represents the "weight" of synapse :math:`k`.

Exact integration for the iaf_psc_alpha model
---------------------------------------------

First we make the substitutions:

.. math::
    \begin{align*}
        y_1 &= \frac{d}{dt}\iota+\frac{1}{\tau_{syn}}\iota \\
        y_2 &= \iota \\
        y_3 &= V
    \end{align*}

for the equation 

.. math::
    \frac{dV}{dt}=\frac{-1}{Tau}V+\frac{1}{C}\iota

we get the homogeneous differential equation (for :math:`y=(y_1,y_2,y_3)^t`)

.. math::
    \frac{d}{dt}y= Ay=
    \begin{pmatrix}
    \frac{1}{\tau_{syn}}& 0 & 0\\ 
    1 & \frac{1}{\tau_{syn}} & 0\\ 
    0 & \frac{1}{C} & -\frac {1}{\tau}
    \end{pmatrix}
    y.

The solution of this differential equation is given by :math:`y(t)=e^{At}y(0)` and can be solved stepwise for a fixed time step :math:`h`:

.. math::
    y_{t+h}=y(t+h)=e^{A(t+h)}y(0)=e^{Ah}e^{At}y(0)=e^{Ah}y(t)=e^{Ah}y_t.

The complete update for the neuron can be written as

.. math::
    y_{t+h}=e^{Ah}y_t + x_{t+h}

where 

.. math::
    x_{t+h}+\begin{pmatrix}\frac{e}{\tau_{\text{syn}}}\\0\\0\end{pmatrix}\sum_{k\in S_{t+h}}\hat{\iota}_k

as the linearity of the system permits the initial conditions for all spikes arriving at a given grid point to be lumped together in the term :math:`x_{t+h}`. :math:`S_{t+h}` is the set of indices :math:`k\in 1,....,K` of synapses that deliver a spike to the neuron at time :math:`t+h`.

The matrix :math:`e^{Ah}` in the C++ implementation of the model in NEST is constructed `here <https://github.com/nest/nest-simulator/blob/b3fc263e073f46f0732c10efb34fcc90f3b6771c/models/iaf_psc_alpha.cpp#L243>`_.

Every matrix entry is calculated twice. For inhibitory post synaptic inputs (with a time constant :math:`\tau_{syn_{in}}`) and excitatory post synaptic inputs (with a time constant :math:`\tau_{syn_{ex}}`).

And the update is performed `here <https://github.com/nest/nest-simulator/blob/b3fc263e073f46f0732c10efb34fcc90f3b6771c/models/iaf_psc_alpha.cpp#L305>`_. The first multiplication evolves the external input. The others are the multiplication of the matrix :math:`e^{Ah}` with :math:`y`. (For inhibitory and excitatory inputs)

References
~~~~~~~~~~

.. [1] RotterV S & Diesmann M (1999) Exact simulation of time-invariant linear
    systems with applications to neuronal modeling. Biologial Cybernetics
    81:381-402. DOI: https://doi.org/10.1007/s004220050570
