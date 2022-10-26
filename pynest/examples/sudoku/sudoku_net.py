# -*- coding: utf-8 -*-
#
# sudoku_net.py
#
# This file is part of NEST.
#
# Copyright (C) 2004 The NEST Initiative
#
# NEST is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 2 of the License, or
# (at your option) any later version.
#
# NEST is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with NEST.  If not, see <http://www.gnu.org/licenses/>.

r"""Class encapsulating the network for solving Sudoku
----------------------------------------------------------------
The class SudokuNet constructs a network of LIF neurons with static synapses
that due to their connectivity rules can converge on valid solutions for Sudoku.


Notes
~~~~~
The functionality of the network lies entirely within the inhibitory
connections between neuron populations. Each population of size ``n_digit``
encodes a digit between 1 and 9 in one of the 81 cells and has outgoing
inhibitory connections to several other populations. Namely all populations
coding for the same digit in the same row, column and 3x3 box of the sudoku
field, since a given digit can only ever occur once in any of those three. It
also inhibits all populations in the same cell which encode different digits
to force the network to converge on a single digit per cell.

If the network is simulated with just this configuration and some background
noise, it converges naturally on states that represent valid solutions for
Sudoku.

If populations coding for specific digits in some of the cells are stimulated
externally (therefore inhibiting all other digits in the same cell), the
network usually converges on a solution compatible with the input
configuration, thus solving the puzzle.

:Authors: J Gille, S Furber, A Rowley
"""
import nest
import numpy as np
import logging


inter_neuron_weight = -0.2   # inhibitory weight for synapses between neurons
weight_stim = 1.3            # weight from stimulation sources to neurons
weight_noise = 1.6           # weight for the background poisson generator
delay = 1.0                  # delay for all synapses

neuron_params = {
    'C_m': 0.25,        # nF    membrane capacitance
    'I_e': 0.5,         # nA    bias current
    'tau_m': 20.0,      # ms    membrane time constant
    't_ref': 2.0,       # ms    refractory period
    'tau_syn_ex': 5.0,  # ms    excitatory synapse time constant
    'tau_syn_in': 5.0,  # ms    inhibitory synapse time constant
    'V_reset': -70.0,   # mV    reset membrane potential
    'E_L': -65.0,       # mV    resting membrane potential
    'V_th': -50.0,      # mV    firing threshold voltage
    'V_m': nest.random.uniform(min=-65, max=-55)
}


class SudokuNet:
    # number of neuron populations (rows*columns*digits)
    n_populations = 9 ** 3

    def __init__(self, pop_size=5, noise_rate=350., stim_rate=200., input=None):
        self.stim_rate = stim_rate          # frequency for input generators
        self.pop_size = pop_size            # number of neurons per population
        # total number of neurons
        self.n_total = self.n_populations * self.pop_size

        logging.info("Creating neuron populations...")
        self.neurons = nest.Create(
            'iaf_psc_exp', self.n_total, params=neuron_params)

        logging.info("Setting up noise...")
        self.noise = nest.Create("poisson_generator", 1, {"rate": noise_rate})
        nest.Connect(self.noise, self.neurons, 'all_to_all',
                     {'synapse_model': 'static_synapse', "delay": delay,
                      'weight': weight_noise})

        # one stimulation source for every digit in every cell
        self.stim = nest.Create("poisson_generator", self.n_populations,
                                {'rate': self.stim_rate})

        # one spike recorder for every digit in every cell
        self.spikerecorders = nest.Create('spike_recorder', self.n_populations)

        # Matrix that stores indices of all neurons in a structured way
        # for easy access during connection setup.
        # Dimensions: (row, column, digit value, individual neuron)
        self.neuron_indices = np.reshape(np.arange(self.n_total),
                                         (9, 9, 9, self.pop_size))

        # Matrix that stores indices of inputs and outputs of the network
        # (stimulation sources and spike recorders) to be connected to the
        # neurons. Dimensions: (row, column, digit value)
        self.io_indices = np.reshape(np.arange(self.n_populations), (9, 9, 9))

        logging.info("Creating inter-neuron and IO-connections...")
        for row in range(9):
            # First and last row of the current 3x3 box.
            box_row_start = (row // 3) * 3
            box_row_end = box_row_start + 3

            for column in range(9):
                # First and last column of the current 3x3 box.
                box_col_start = (column // 3) * 3
                box_col_end = box_col_start + 3

                # Obtain all populations in the surrounding 3x3 box.
                current_box = self.neuron_indices[box_row_start:box_row_end,
                                                  box_col_start:box_col_end]

                for digit in range(9):
                    # population coding for the current row, column and digit
                    sources = self.neuron_indices[row, column, digit]

                    # populations in the same row coding for the same digit
                    row_targets = self.neuron_indices[row, :, digit]
                    # same as above for the current column
                    col_targets = self.neuron_indices[:, column, digit]
                    # populations coding for the same digit in the current box
                    box_targets = current_box[:, :, digit]
                    # neurons coding for different digits in the current cell
                    digit_targets = self.neuron_indices[row, column, :]

                    targets = np.concatenate(
                        (row_targets, col_targets, box_targets, digit_targets),
                        axis=None)
                    # Remove duplicates to avoid multapses
                    targets = np.unique(targets)
                    # Remove the sources from the targets to avoid the
                    # population inhibiting itself.
                    targets = np.setdiff1d(targets, sources)

                    # Transform indices into NodeCollections
                    sources = self.neurons[sources]
                    targets = self.neurons[targets]

                    # Create inhibitory connections
                    nest.Connect(
                        sources, targets, 'all_to_all',
                        {'synapse_model': 'static_synapse', "delay": delay,
                         'weight': inter_neuron_weight})

                    # connect the stimulation source to the current population.
                    # Weights are initialized to 0 and altered in
                    # set_input_config()
                    nest.Connect(
                        self.stim[self.io_indices[row, column, digit]],
                        sources, 'all_to_all', {"delay": delay, "weight": 0.})

                    # connect current population to a single spike_recorder
                    nest.Connect(
                        sources, self.spikerecorders
                        [self.io_indices[row, column, digit]])

        if input is not None:
            logging.info("setting input...")
            self.set_input_config(input)

        logging.info("Setup complete.")

    def reset_input(self):
        """sets all weights between input and network neurons to 0."""
        nest.GetConnections(self.stim).set({"weight": 0.})

    def set_input_config(self, input):
        """sets the connection weights from stimulation sources to populations
        in order to stimulate the network according to a puzzle configuration.

        Parameters
        ----------
        input : np.array
            a np.array of shape (9,9) where each entry is the value of the corresponding
            cell in the sudoku field. Zero-valued entries are ignored.
        """
        self.reset_input()

        for row in range(9):
            for column in range(9):
                value = input[row, column]

                # only apply stimulation where the input configuration dictates a number
                if value != 0:
                    connections = nest.GetConnections(
                        self.stim[self.io_indices[row, column, value-1]])
                    connections.set({"weight": weight_stim})

    def get_spike_trains(self):
        """returns all events recorded by the spike recorders.

        Returns:
            np.array
                array of all the data from all spike recorders
        """
        return np.array(self.spikerecorders.get("events"))

    def reset(self):
        """resets the network in three steps:
                setting all input weights to 0
                resetting all membrane potentials to their default value
                deleting all recorded spikes
        """
        self.reset_input()
        self.reset_V_m()
        self.reset_spike_recorders()

    def reset_V_m(self):
        """resets membrane potential of all neurons to (uniformly random) default values."""
        self.neurons.V_m = nest.random.uniform(-65, 55)

    def reset_spike_recorders(self):
        """deletes all recorded spikes from the spike recorders connected to the network."""
        self.spikerecorders.n_events = 0

    def set_noise_rate(self, rate):
        """sets the rate of the Poisson generator that feeds noise into the network.

        Parameters
        ----------
        rate : float
            average spike frequency in Hz
        """
        self.noise.rate = rate
