.. _sec_model_description:

Two population STDP network model
=================================

Network model
-------------

In this example, we employ a simple network model describing the dynamics of a local cortical circuit at the spatial scale of
~1mm within a single cortical layer. It is derived from the model proposed in [1]_,
but accounts for the synaptic weight dynamics for connections between excitatory neurons. The weight dynamics are described
by the spike-timing-dependent plasticity (STDP) model derived in [2]_.
This network model is used as the basis for scaling experiments comparing the model ``ignore-and-fire`` with the ``iaf_psc_alpha``.
You can find

Summary of network model
------------------------

.. list-table::
   :stub-columns: 1

   * - **Populations**
     - excitatory population :math:`\mathcal{E}`, inhibitory population :math:`\mathcal{I}`, external Poissonian spike sources :math:`\mathcal{X}`
   * - **Connectivity**
     - sparse random connectivity respecting Dale’s principle
   * - **Neurons**
     - leaky integrate-and-fire (LIF)
   * - **Synapses**
     - linear input integration with alpha-function-shaped postsynaptic currents (PSCs), spike-timing dependent plasticity (STDP) for connections between excitatory neurons
   * - **Input**
     - stationary, uncorrelated Poissonian spike trains

   * -
     - .. figure:: /static/img/NetworkSketch_TwoPopulationNetworkPlastic.svg

           Network sketch (see `Fig. 8  <https://doi.org/10.1371/journal.pcbi.1010086.g008>`_
           in Senk et al. [3]_).

Detailed desciption of network model
------------------------------------

Populations
~~~~~~~~~~~

.. table::

      +---------------------+----------------------+----------------------------------+
      | **Name**            | **Elements**         | **Size**                         |
      +=====================+======================+==================================+
      | :math:`\mathcal{E}` | LIF neurons          | :math:`N_\text{E}=\beta{}N`      |
      |                     |                      |                                  |
      +---------------------+----------------------+----------------------------------+
      | :math:`\mathcal{I}` | LIF neurons          | :math:`N_\text{I}=N-N_\text{E}`  |
      |                     |                      |                                  |
      +---------------------+----------------------+----------------------------------+
      | :math:`\mathcal{X}` | realizations of a    | :math:`N`                        |
      |                     | Poisson point        |                                  |
      |                     | process              |                                  |
      +---------------------+----------------------+----------------------------------+

Connectivity
~~~~~~~~~~~~

.. table::

   +---------------------+----------------------+---------------------------------------------------------+
   | **Source**          | **Target**           | **Pattern**                                             |
   +=====================+======================+=========================================================+
   | :math:`\mathcal{E}` | :math:`\mathcal{E}`  | -  random,                                              |
   |                     |                      |    independent;                                         |
   |                     |                      |    homogeneous                                          |
   |                     |                      |    in-degree                                            |
   |                     |                      |    :math:`K_{\text{E},i}=K_\text{E}`                    |
   |                     |                      |                                                         |
   |                     |                      |    (:math:`\forall{}i\in\mathcal{E}`)                   |
   |                     |                      |                                                         |
   |                     |                      |                                                         |
   |                     |                      | -  plastic synaptic weights                             |
   |                     |                      |    :math:`J_{ij}(t)`                                    |
   |                     |                      |                                                         |
   |                     |                      |    (:math:`\forall{}i\in\mathcal{E},j\in\mathcal{E}`)   |
   |                     |                      |                                                         |
   |                     |                      | - homogeneous                                           |
   |                     |                      |                                                         |
   |                     |                      |   spike-transmission                                    |
   |                     |                      |   delays                                                |
   |                     |                      |   :math:`d_{ij}=d`                                      |
   |                     |                      |                                                         |
   |                     |                      |   (:math:`\forall{}i\in\mathcal{E},j\in\mathcal{E}`)    |
   |                     |                      |                                                         |
   +---------------------+----------------------+---------------------------------------------------------+
   | :math:`\mathcal{E}` | :math:`\mathcal{I}`  | -  random,                                              |
   |                     |                      |    independent;                                         |
   |                     |                      |    homogeneous                                          |
   |                     |                      |    in-degree                                            |
   |                     |                      |    :math:`K_{\text{E},i}=K_\text{E}`                    |
   |                     |                      |                                                         |
   |                     |                      |    (:math:`\forall{}i\in\mathcal{I}`)                   |
   |                     |                      |                                                         |
   |                     |                      |                                                         |
   |                     |                      | -  fixed synaptic                                       |
   |                     |                      |    weights                                              |
   |                     |                      |    :math:`J_{ij}\in\{0,J\}`                             |
   |                     |                      |                                                         |
   |                     |                      |    (:math:`\forall{}i\in\mathcal{I},j\in\mathcal{E}`)   |
   |                     |                      |                                                         |
   |                     |                      |                                                         |
   |                     |                      |                                                         |
   |                     |                      | -  homogeneous                                          |
   |                     |                      |                                                         |
   |                     |                      |    spike-transmission                                   |
   |                     |                      |    delays                                               |
   |                     |                      |    :math:`d_{ij}=d`                                     |
   |                     |                      |                                                         |
   |                     |                      |    (:math:`\forall{}i\in\mathcal{I},j\in\mathcal{E}`)   |
   |                     |                      |                                                         |
   +---------------------+----------------------+---------------------------------------------------------+
   | :math:`\mathcal{I}` | :math:`\mathcal \    | -  random,                                              |
   |                     | {E}\cup\mathcal{I}`  |    independent;                                         |
   |                     |                      |    homogeneous                                          |
   |                     |                      |    in-degree                                            |
   |                     |                      |    :math:`K_{\text{I},i}=K_\text{I}`                    |
   |                     |                      |                                                         |
   |                     |                      |    (:math:`forall{}i\in\mathcal{E}\cup\mathcal{I}`)\    |
   |                     |                      |    j\in\mathcal{I}`)                                    |
   |                     |                      |                                                         |
   |                     |                      | -  fixed synaptic                                       |
   |                     |                      |    weights                                              |
   |                     |                      |    :math:`J_{ij}\in\{-gJ,0\}`                           |
   |                     |                      |                                                         |
   |                     |                      |    (:math:`\forall{}i\in\mathcal{E}\cup\mathcal{I}, \   |
   |                     |                      |    j\in\mathcal{I}`)                                    |
   |                     |                      |                                                         |
   |                     |                      |                                                         |
   |                     |                      | -  homogeneous                                          |
   |                     |                      |                                                         |
   |                     |                      |    spike-transmission                                   |
   |                     |                      |    delays                                               |
   |                     |                      |    :math:`d_{ij}=d`                                     |
   |                     |                      |                                                         |
   |                     |                      |    (:math:`\forall{}i\in\mathcal{E}\cup\mathcal{I}, \   |
   |                     |                      |    j\in\mathcal{I}`)                                    |
   |                     |                      |                                                         |
   |                     |                      |                                                         |
   +---------------------+----------------------+---------------------------------------------------------+
   | :math:`\mathcal{X}` | :math:`\mathcal \    | -  one-to-one                                           |
   |                     | {E}\cup\mathcal{I}`  |                                                         |
   |                     |                      | -  fixed synaptic                                       |
   |                     |                      |    weights                                              |
   |                     |                      |    :math:`J_{ij}=J`                                     |
   |                     |                      |                                                         |
   |                     |                      |    (:math:`\forall{}i\in\mathcal{E}\cup\mathcal{I}, \   |
   |                     |                      |    j\in\mathcal{X}`)                                    |
   |                     |                      |                                                         |
   |                     |                      | -  homogeneous                                          |
   |                     |                      |                                                         |
   |                     |                      |    spike-transmission                                   |
   |                     |                      |    delays                                               |
   |                     |                      |    :math:`d_{ij}=d`                                     |
   |                     |                      |                                                         |
   |                     |                      |    (:math:`\forall{}i\in\mathcal{E}\cup\mathcal{I}, \   |
   |                     |                      |    j\in\mathcal{X}`)                                    |
   |                     |                      |                                                         |
   +---------------------+----------------------+---------------------------------------------------------+



Neuron
~~~~~~

.. list-table::

   * - **Leaky integrate-and-fire (iaf) dynamics**
     - Dynamics of membrane potential :math:`V_{i}(t)` and
       spiking activity :math:`s_i(t)` of neuro :math:`i\in\left\{1,\ldots,N\right\}`:

       * emission of :math:`k`\ th (:math:`k=1,2,\ldots`) spike of neuron
         :math:`i` at time :math:`t_{i}^{k}` if

         .. math::
            V_{i}\left(t_{i}^{k}\right)\geq\theta

         with spike threshold :math:`\theta`

       * reset and refractoriness:

         .. math:: \forall{}k,\ \forall t \in \left(t_{k}^{i},\,t_{k}^{i}+\tau_\text{ref}\right]:\quad V_{i}(t)=V_\text{reset}

         with refractory period :math:`\tau_\text{ref}` and reset potential
         :math:`V_\text{reset}`

       * spike train :math:`\displaystyle s_i(t)=\sum_k \delta(t-t_i^k)`

       * subthreshold dynamics of membrane potential :math:`V_{i}(t)`:

         .. math::

            \begin{aligned}
                                          &\forall{}k,\ \forall t \notin \left[t_{i}^{k},\,t_{i}^{k}+\tau_\text{ref}\right):\\
                                          &\qquad\tau_\text{m}\frac{\text{d}{}V_i(t)}{\text{d}{}t} =
                                          \Bigl[E_\text{L}-V_i(t)\Bigr]+R_\text{m}I_i(t)
                                        \end{aligned}

         with membrane time constant :math:`\tau_\text{m}`, membrane
         resistance :math:`R_\text{m}`, resting potential :math:`E_\text{L}`,
         and total synaptic input current :math:`I_i(t)`


Synapse: transmission
~~~~~~~~~~~~~~~~~~~~~

.. list-table::

   * - **Current-based synapses with alpha-function shaped postsynaptic currents (PSCs)**


     - Total synaptic input current of neuron :math:`i`

       .. math:: I_i(t)=I_{\text{E},i}(t)+I_{\text{I},i}(t)+I_{\text{X},i}(t)

       * excitatory, inhibitory and external synaptic input currents

         .. math::

             %I_{P,i}(t)=\sum_{j\in\mathcal{P}}(\text{PSC}_{ij}*s_j)(t)
                                      %\quad\text{for}\quad
                                      %(P,\mathcal{P})\in\{(\exc,\Epop),(\inh,\Ipop),(\ext,\Xpop)\}
                                      %,
                                       \begin{aligned}
                                         I_{\text{E},i}(t)&=\sum_{j\in\mathcal{E}}\bigl(\text{PSC}_{ij}*s_j\bigr)(t-d_{ij})\\
                                         I_{\text{I},i}(t)&=\sum_{j\in\mathcal{I}}\bigl(\text{PSC}_{ij}*s_j\bigr)(t-d_{ij})\\
                                         I_{\text{X},i}(t)&=\sum_{j\in\mathcal{X}}\bigl(\text{PSC}_{ij}*s_j\bigr)(t-d_{ij})
                                       \end{aligned}

         with spike trains :math:`s_j(t)` of local
         (:math:`j\in\mathcal{E}\cup\mathcal{I}`) and external sources
         (:math:`j\in\mathcal{X}`), spike transmission delays :math:`d_{ij}`,
         and convolution operator “:math:`*`”:
         :math:`\displaystyle\bigl(f*g\bigr)(t)=\int_{-\infty}^\infty\text{d}s\,f(s)g(t-s)`)

       * alpha-function shaped postsynaptic currents

         .. math:: \text{PSC}_{ij}(t)=\hat{I}_{ij}e\tau_\text{s}^{-1}te^{-t/\tau_\text{s}}\Theta(t)

         with synaptic time constant :math:`\tau_\text{s}` and Heaviside
         function :math:`\Theta(\cdot)`

       * postsynaptic potential triggered by a single presynaptic spike

         .. math::

             \text{PSP}_{ij}(t)=
                                      \hat{I}_{ij}\frac{e}{\tau_\text{s}C_\text{m}}
                                      \left(\frac{1}{\tau_\text{m}}-\frac{1}{\tau_\text{s}}\right)^{-2}
                                      \left(\left(\frac{1}{\tau_\text{m}}-\frac{1}{\tau_\text{s}}\right) t e^{-t/\tau_\text{s}} - e^{-t/\tau_\text{s}} + e^{-t/\tau_\text{m}} \right) \Theta(t)

       * PSC amplitude (synaptic weight)

         .. math::

             \hat{I}_{ij}=\text{max}_t\bigl(\text{PSC}_{ij}(t)\bigr)
                                      =\frac{J_{ij}}{J_\text{unit}(\tau_\text{m},\tau_\text{s},C_\text{m})}

         parameterized by PSP amplitude
         :math:`J_{ij}=\text{max}_t\bigl(\text{PSP}_{ij}(t)\bigr)`

         with unit PSP amplitude (PSP amplitude for :math:`\hat{I}_{ij}=1`):

            .. math::

               J_\text{unit}(\tau_\text{m},\tau_\text{s},C_\text{m})
                                         = \frac{e}{C_\text{m}\left(1-\frac{\tau_\text{s}}{\tau_\text{m}}\right)}\left( \frac{e^{-t_\text{max}/\tau_\text{m}} - e^{-t_\text{max}/\tau_\text{s}}}{\frac{1}{\tau_\text{s}} - \frac{1}{\tau_\text{m}}} - t_\text{max}e^{-t_\text{max}/\tau_\text{s}} \right),

         time to PSP maximum

            .. math::

               t_\text{max} =
                                         \frac{1}{\frac{1}{\tau_\text{s}} - \frac{1}{\tau_\text{m}}}\left(-W_{-1}\left(\frac{-\tau_\text{s}e^{-\frac{\tau_\text{s}}{\tau_\text{m}}}}{\tau_\text{m}}\right) - \frac{\tau_\text{s}}{\tau_\text{m}}\right),

         and Lambert-W function :math:`\displaystyle W_{-1}(x)` for
         :math:`\displaystyle x \ge -1/e`



Synapse: plasticity
~~~~~~~~~~~~~~~~~~~

.. list-table::

   * - **Spike-timing dependent plasticity (STDP) with power-law weight dependence and all-to-all spike pairing scheme.**
       See Morrison et al. [2]_ for connections between excitatory neurons.


     - Dynamics of synaptic weights :math:`J_{ij}(t)` :math:`\forall{}i\in\mathcal{E}, j\in\mathcal{E}`:

          .. math::

             \begin{aligned}
                    &\forall J_{ij}\ge{}0: \\[1ex]
                    &\quad
                    \frac{\text{d}}{}J_{ij}{\text{d}{}t}=
                    \lambda^+f^+(J_{ij})\sum_k x^+_j(t)\delta\Bigl(t-[t_i^k+d_{ij}]\Bigr)
                    + \lambda^-f^-(J_{ij})\sum_l x^-_i(t)\delta\Big(t-[t_j^l-d_{ij}]\Bigr)\\[1ex]
                    &\forall{}\{t|J_{ij}(t)<0\}: \quad J_{ij}(t)=0  \quad \text{(clipping)}
                  \end{aligned}

          with

        -  pre- and postsynaptic spike times :math:`\{t_j^l|l=1,2,\ldots\}` and
           :math:`\{t_i^k|k=1,2,\ldots\}`,

        -  magnitude :math:`\lambda^+=\lambda` of weight update for causal
           firing (postsynaptic spike following presynaptic spikes:
           :math:`t_i^k>t_j^l`),

        -  magnitude :math:`\lambda^-=-\alpha\lambda` of weight update for
           acausal firing (presynaptic spike following postsynaptic spikes:
           :math:`t_i^k<t_j^l`),

        -  power-law weight dependence
           :math:`f^+(J_{ij})=J_0(J_{ij}/J_0)^{\mu^+}` of weight update for
           causal firing with exponent :math:`\mu^+` and reference weight
           :math:`J_0`,

        -  linear weight dependence :math:`f^-(J_{ij})=J_{ij}` of weight update
           for acausal firing,

        -  (dendritic) delay :math:`d_{ij}`,

        -  spike trace :math:`x^+_j(t)` of presynaptic neuron :math:`j`,
           evolving according to

           .. math:: \frac{\text{d}{}x^+_j}{\text{d}{}t}=-\frac{x^+_j(t)}{\tau^+}+\sum_l\delta(t-t_j^l)

           with presynaptic spike times :math:`\{t_j^l|l=1,2,\ldots\}` and time
           constant :math:`\tau^+`,

        -  spike trace :math:`x^-_i(t)` of postsynaptic neuron :math:`i`,
           evolving according to

           .. math:: \frac{\text{d}{}x^-_i}{\text{d}{}t}=-\frac{x^-_i(t)}{\tau^-}+\sum_k\delta(t-t_i^k)

           with postsynaptic spike times :math:`\{t_i^k|k=1,2,\ldots\}` and time
           constant :math:`\tau^-`

       .. note::

          The above weight update accounts for *all* pairs of pre- and
          postsynaptic spikes (all-to-all spike pairing scheme). The spike
          histories and the dependence of the weight update on the time lag of
          pre- and postsynaptic firing are fully captured by the spike traces
          :math:`x^+_j(t)` and :math:`x^-_i(t)`.


Stimulus
~~~~~~~~

.. table::

   +-------------------------------------------------+---------------------------------------------------+
   | **Type**                                        | **Description**                                   |
   +=================================================+===================================================+
   | stationary, uncorrelated Poisson spike trains   | :math:`N=|\mathcal{X}|` independent realizations  |
   |                                                 | :math:`s_i(t)` (:math:`i\in\mathcal{X}`) of a     |
   |                                                 | Poisson point process with constant rate          |
   |                                                 | :math:`\nu_\text{X}(t)=\eta\nu_\theta`, where     |
   |                                                 |                                                   |
   |                                                 | .. math::                                         |
   |                                                 |                                                   |
   |                                                 |    \label{eq:rheobase_rate_LIF_alpha}             |
   |                                                 |                                                   |
   |                                                 |                   \nu_\theta=\frac{\theta-E       |
   |                                                 |                   _\text{L}}{R_\text{m}{}         |
   |                                                 |                  \hat{I}_X{}e\tau_\text{s}}       |
   |                                                 |                                                   |
   |                                                 | denotes the rheobase rate, and :math:`\eta` and   |
   |                                                 | :math:`\hat{I}_X=J/J_\text{unit}` the relative    |
   |                                                 | rate and the synaptic weight (PSC amplitude) of   |
   |                                                 | external sources                                  |
   |                                                 |                                                   |
   +-------------------------------------------------+---------------------------------------------------+


Initial conditions
~~~~~~~~~~~~~~~~~~

.. table::

   +--------------------------------------------------+---------------------------------------------------+
   | **Type**                                         | **Description**                                   |
   |                                                  |                                                   |
   +==================================================+===================================================+
   | random initial membrane potentials, homogeneous  | -  membrane potentials:                           |
   | initial synaptic weights and spike traces        |    :math:`V_i(t=0)\sim \                          |
   |                                                  |    \mathcal{U}(V_{0,\text{min}},V_{0,\text{max}})`|
   |                                                  |    randomly and independently drawn from a        |
   |                                                  |    uniform distribution between                   |
   |                                                  |    :math:`V_{0,\text{min}}` and                   |
   |                                                  |    :math:`V_{0,\text{max}}` (:math:`\forall{}i`)  |
   |                                                  |                                                   |
   |                                                  | -  synaptic weights:                              |
   |                                                  |    :math:`\hat{I}_{ij}(t=0)=J/J_\text{unit}`      |
   |                                                  |                                                   |
   |                                                  |    (:math:`\forall{}i\in\mathcal{E},           \  |
   |                                                  |    j\in\mathcal{E}`)                              |
   |                                                  |                                                   |
   |                                                  | -  spike traces:                                  |
   |                                                  |    :math:`x_{+,i}(t=0)=x_{-,i}(t=0)=0`            |
   |                                                  |    (:math:`\forall{}i\in\mathcal{E}`)             |
   +--------------------------------------------------+---------------------------------------------------+

.. _sec_model_parameters:

Model parameters
----------------

.. note::

   Parameters derived from other parameters are marked in :math:`\textcolor{blue}{blue}`.

Network and connectivity
~~~~~~~~~~~~~~~~~~~~~~~~

.. table::

      +----------------------------------+---------------------------+----------------------+
      | **Name**                         | **Value**                 | **Description**      |
      +==================================+===========================+======================+
      | :math:`N`                        | :math:`12500`             | total number of      |
      |                                  |                           | neurons in local     |
      |                                  |                           | network              |
      +----------------------------------+---------------------------+----------------------+
      | :math:`\beta`                    | :math:`0.8`               | relative number of   |
      |                                  |                           | excitatory neurons   |
      +----------------------------------+---------------------------+----------------------+
      | :math:`\color{blue} N_\text{E}`  | :math:`\beta{}N=10000`    | total number of      |
      |                                  |                           | excitatory neurons   |
      +----------------------------------+---------------------------+----------------------+
      | :math:`\color{blue} N_\text{I}`  | :math:`N-N_\text{E}=2500` | total number of      |
      |                                  |                           | inhibitory neurons   |
      +----------------------------------+---------------------------+----------------------+
      | :math:`K`                        | :math:`1250`              | total number of      |
      |                                  |                           | inputs per neuron    |
      |                                  |                           | (in-degree) from     |
      |                                  |                           | local network        |
      +----------------------------------+---------------------------+----------------------+
      | :math:`\color{blue} K_\text{E}`  |                           | number of excitatory |
      |                                  | :math:`\beta{}K=1000`     | inputs per neuron    |
      |                                  |                           | (exc. in-degree)     |
      |                                  |                           | from local network   |
      +----------------------------------+---------------------------+----------------------+
      | :math:`\color{blue} K_\text{I}`  |                           | number of inhibitory |
      |                                  | :math:`K-K_\text{E}=250`  | inputs per neuron    |
      |                                  |                           | (inh. in-degree)     |
      +----------------------------------+---------------------------+----------------------+

Neuron parameters
~~~~~~~~~~~~~~~~~

.. table::

      +---------------------------------+--------------------------------+----------------------+
      | **Name**                        | **Value**                      | **Description**      |
      +=================================+================================+======================+
      | :math:`\theta`                  |                                | spike threshold      |
      |                                 | :math:`20\,\text{mV}`          |                      |
      +---------------------------------+--------------------------------+----------------------+
      | :math:`E_\text{L}`              | :math:`0\,\text{mV}`           | resting potential    |
      +---------------------------------+--------------------------------+----------------------+
      |                                 |                                | membrane time        |
      | :math:`\tau_\text{m}`           | :math:`20\,\text{ms}`          | constant             |
      +---------------------------------+--------------------------------+----------------------+
      | :math:`C_\text{m}`              |                                | membrane capacitance |
      |                                 | :math:`250\,\text{pF}`         |                      |
      +---------------------------------+--------------------------------+----------------------+
      | :math:`\color{blue} R_\text{m}` | :math:`\tau \                  | membrane resistance  |
      |                                 | _\text{m}/C_\text{m}\          |                      |
      |                                 | =80\,\text{M}\Omega`           |                      |
      +---------------------------------+--------------------------------+----------------------+
      |                                 | :math:`0\,\text{mV}`           | reset potential      |
      | :math:`V_\text{reset}`          |                                |                      |
      +---------------------------------+--------------------------------+----------------------+
      |                                 | :math:`2\,\text{ms}`           | absolute refractory  |
      | :math:`\tau_\text{ref}`         |                                | period               |
      +---------------------------------+--------------------------------+----------------------+


Synapse parameters
~~~~~~~~~~~~~~~~~~

.. table::

      +---------------------------------------+-----------------------------+----------------------+
      | **Name**                              | **Value**                   | **Description**      |
      +=======================================+=============================+======================+
      | :math:`J`                             |                             | (initial) weight     |
      |                                       | :math:`0.5\,\,\text{mV}`    | (PSP amplitude) of   |
      |                                       |                             | excitatory synapses  |
      +---------------------------------------+-----------------------------+----------------------+
      | :math:`g`                             | :math:`10`                  | relative strength of |
      |                                       |                             | inhibitory synapses  |
      +---------------------------------------+-----------------------------+----------------------+
      | :math:`\color{blue} J_\text{I}`       | :math:`-g                   | weight (PSP          |
      |                                       | {}J=-5\,\,\text{mV}`        | amplitude) of        |
      |                                       |                             | inhibitory synapses  |
      +---------------------------------------+-----------------------------+----------------------+
      |                                       | :math:`\approx{}\           | unit PSP amplitude   |
      | :math:`\color{blue} J_\text{unit}`    | 0.01567\,\,\text{mV} \      |                      |
      |                                       | /\,\text{pA}`               |                      |
      +---------------------------------------+-----------------------------+----------------------+
      | :math:`\color{blue} \                 | :math:`J/           \       | (initial) weight     |
      | \hat{I}_\text{E}(0)`                  | J_\text{unit}\approx\       | (PSC amplitude) of   |
      |                                       | {}31.9\,\,\text{pA}`        | excitatory synapses  |
      +---------------------------------------+-----------------------------+----------------------+
      |                                       | :math:`-g{}J/     \         | weight (PSC          |
      | :math:`\color{blue} \hat{I}_\text{I}` | J_\text{unit}\approx\       | amplitude) of        |
      |                                       | {}-319\,\,\text{pA}`        | inhibitory synapses  |
      +---------------------------------------+-----------------------------+----------------------+
      |                                       | :math:`J/        \          | weight (PSC          |
      | :math:`\color{blue} \hat{I}_\text{X}` | J_\text{unit}\approx\       | amplitude) of        |
      |                                       | {}31.9\,\,\text{pA}`        | external inputs      |
      +---------------------------------------+-----------------------------+----------------------+
      | :math:`d`                             |                             | spike transmission   |
      |                                       | :math:`1.5\,\,\text{ms}`    | delay                |
      +---------------------------------------+-----------------------------+----------------------+
      |                                       |                             | synaptic time        |
      | :math:`\tau_\text{s}`                 | :math:`2\,\,\text{ms}`      | constant             |
      +---------------------------------------+-----------------------------+----------------------+
      |                                       | :math:`20`                  | magnitude of weight  |
      | :math:`\lambda\color{blue} =\         |                             | update for causal    |
      | \lambda^+`                            |                             | firing               |
      +---------------------------------------+-----------------------------+----------------------+
      | :math:`\mu^+`                         | :math:`0.4`                 | weight dependence    |
      |                                       |                             | exponent for causal  |
      |                                       |                             | firing               |
      +---------------------------------------+-----------------------------+----------------------+
      | :math:`J_0`                           |                             | reference weight     |
      |                                       | :math:`1\,\,\text{pA}`      |                      |
      +---------------------------------------+-----------------------------+----------------------+
      | :math:`\tau^+`                        |                             | time constant of     |
      |                                       | :math:`15\,\,\text{ms}`     | weight update for    |
      |                                       |                             | causal firing        |
      +---------------------------------------+-----------------------------+----------------------+
      | :math:`\alpha`                        | :math:`0.1`                 | relative magnitude   |
      |                                       |                             | of weight update for |
      |                                       |                             | acausal firing       |
      +---------------------------------------+-----------------------------+----------------------+
      | :math:`\color{blue} \lambda^-`        |                             | magnitude of weight  |
      |                                       | :math:`-\alpha\lambda=-2`   | update for acausal   |
      |                                       |                             | firing               |
      +---------------------------------------+-----------------------------+----------------------+
      | :math:`\tau^-`                        |                             | time constant of     |
      |                                       | :math:`30\,\,\text{ms}`     | weight update for    |
      |                                       |                             | acausal firing       |
      +---------------------------------------+-----------------------------+----------------------+

Stimulus parameters
~~~~~~~~~~~~~~~~~~~

.. table::

      +--------------------------------------+------------------------+----------------------+
      | **Name**                             | **Value**              | **Description**      |
      +======================================+========================+======================+
      | :math:`\eta`                         | :math:`1.2`            | relative rate of     |
      |                                      |                        | external Poissonian  |
      |                                      |                        | sources              |
      +--------------------------------------+------------------------+----------------------+
      | :math:`\color{blue} \nu_\theta`      | :math:`1442   \        | rheobase rate        |
      |                                      | \,\text{spikes/s}`     |                      |
      +--------------------------------------+------------------------+----------------------+
      |                                      | :math:`\eta\           | rate of external     |
      | :math:`\color{blue} \nu_{\text{X}}`  | \nu_\theta\approx{}\   | Poissonian sources   |
      |                                      | 1730\,\text{spikes/s}` |                      |
      +--------------------------------------+------------------------+----------------------+

Initial conditions parameters
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. table::

      +---------------------------------------+------------------------+----------------------+
      | **Name**                              | **Value**              | **Description**      |
      +=======================================+========================+======================+
      |                                       | :math:`E_\text{L}\     | minimum initial      |
      | :math:`\color{blue} V_{0,\text{min}}` | =0\,\,\text{mV}`       | membrane potential   |
      +---------------------------------------+------------------------+----------------------+
      |                                       | :math:`\theta\         | maximum initial      |
      | :math:`\color{blue} V_{0,\text{max}}` | = 20\,\,\text{mV}`     | membrane potential   |
      |                                       |                        |                      |
      +---------------------------------------+------------------------+----------------------+


References
----------

.. [1] Brunel N (2000). Dynamics of networks of randomly connected excitatory
       and inhibitory spiking neurons. Journal of Physiology-Paris
       94(5-6):445-463. <https://doi.org/10.1023/A:1008925309027>

.. [2] Morrison A. Aertsen, A. and Diesmann M. 2007.
       Spike-timing-dependent plasticity in balanced random networks.
       Neural Computation. 19(6):1437–1467.

.. [3] Senk J, Kriener B, Djurfeldt M, Voges N, Jiang H-J, Schüttler L, et al. 2022.
       Connectivity concepts in neuronal network modeling. PLoS Comput Biol 18(9): e1010086.
       https://doi.org/10.1371/journal.pcbi.1010086
