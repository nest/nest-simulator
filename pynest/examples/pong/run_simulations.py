# -*- coding: utf-8 -*-
#
# run_simulations.py
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

r"""Application to train networks to play pong against each other
----------------------------------------------------------------
This program makes two spiking neural networks of two layers each compete
against each other in encoding an input-output mapping within their weights.

The script is centered around a simulation of the classic game Pong
in which the vertical position of the ball determines the input for both
'players' and their output activation after a predetermined simulation
time changes the paddle positions within the game.

The output of the script stores information about the state of the game,
and both networks after every simulation step. this data is saved to three
.pkl files after the simulation is complete, which can be used to visualize
the output (e.g., using :doc:`generate_gif.py <generate_gif>`).

The idea for this simulation as well as the core of the R-STDP and Pong
implementation are from [1]_ and were created by Timo Wunderlich and Electronic
Vision(s) (The original implementation can be found
`here <https://github.com/electronicvisions/model-sw-pong>`_).
The visualization and implementation of dopaminergic learning, as well as
changes to the existing codebase were developed by Johannes Gille (2022).

See Also
---------
:doc:`Code for visualizing the output <generate_gif>`


References
----------
.. [1] Wunderlich T., et al (2019). Demonstrating advantages of
       neuromorphic computation: a pilot study. Frontiers in neuroscience, 13,
       260. https://doi.org/10.3389/fnins.2019.00260

:Authors: J Gille, T Wunderlich, Electronic Vision(s)
"""

import argparse
import datetime
import gzip
import logging
import nest
import os
import sys
import time

import numpy as np
import pickle

import pong
from networks import POLL_TIME, PongNetDopa, PongNetRSTDP


class AIPong:
    def __init__(self, p1, p2, out_dir=""):
        """A class to run and store pong simulations of two competing spiking
        neural networks.

        Args:
            p1 (PongNet): Network to play on the left side.
            p2 (PongNet): Network to play on the right side.
            out_folder (str, optional): Name of the output folder. Defaults to
            current time stamp (YYYY-mm-dd-HH-MM-SS).
        """
        self.game = pong.GameOfPong()
        self.player1 = p1
        self.player2 = p2

        if out_dir == "":
            out_dir = '{0:%Y-%m-%d-%H-%M-%S}'.format(datetime.datetime.now())
        if os.path.exists(out_dir):
            print(f"output folder {out_dir} already exists!")
            sys.exit()
        os.mkdir(out_dir)
        self.out_dir = out_dir

        logging.info(f"setup complete for a pong game between: {p1} and {p2}.")

    def run_games(self, max_runs=10000):
        """Run a simulation of pong games and store the results.

        Args:
            max_runs (int, optional): Number of iterations to simulate.
            Defaults to 10000.
        """
        self.game_data = []
        l_score, r_score = 0, 0

        start_time = time.time()
        self.run = 0
        biological_time = 0

        logging.info(f"Starting simulation of {max_runs} iterations of "
                     f"{POLL_TIME}ms each.")
        while self.run < max_runs:
            logging.debug("")
            logging.debug(f"Iteration {self.run}:")
            self.input_index = self.game.ball.get_cell()[1]
            self.player1.set_input_spiketrain(
                self.input_index, biological_time)
            self.player2.set_input_spiketrain(
                self.input_index, biological_time)

            if self.run % 100 == 0:
                logging.info(
                    f"{round(time.time() - start_time, 2)}: Run "
                    f"{self.run}, score: {l_score, r_score}, mean rewards: "
                    f"{round(np.mean(self.player1.mean_reward), 3)}, "
                    f"{round(np.mean(self.player2.mean_reward), 3)}")

            logging.debug("Running simulation...")
            nest.Simulate(POLL_TIME)
            biological_time = nest.GetKernelStatus("biological_time")

            for network, paddle in zip(
                    [self.player1, self.player2],
                    [self.game.l_paddle, self.game.r_paddle]):

                network.apply_synaptic_plasticity(biological_time)
                network.reset()

                position_diff = network.winning_neuron - paddle.get_cell()[1]
                if position_diff > 0:
                    paddle.move_up()
                elif position_diff == 0:
                    paddle.dont_move()
                else:
                    paddle.move_down()

            self.game.step()
            self.run += 1
            self.game_data.append(
                [self.game.ball.get_pos(),
                 self.game.l_paddle.get_pos(),
                 self.game.r_paddle.get_pos(),
                 (l_score, r_score)])

            if self.game.result == pong.RIGHT_SCORE:
                self.game.reset_ball(False)
                r_score += 1
            elif self.game.result == pong.LEFT_SCORE:
                self.game.reset_ball(True)
                l_score += 1

        end_time = time.time()
        logging.info(
            f"Simulation of {max_runs} runs complete after: "
            f"{datetime.timedelta(seconds=end_time - start_time)}")

        self.game_data = np.array(self.game_data)

        out_data = dict()
        out_data["ball_pos"] = self.game_data[:, 0]
        out_data["left_paddle"] = self.game_data[:, 1]
        out_data["right_paddle"] = self.game_data[:, 2]
        out_data["score"] = self.game_data[:, 3]

        logging.info("saving game data...")
        with open(os.path.join(self.out_dir, "gamestate.pkl"), "wb") as file:
            pickle.dump(out_data, file)

        logging.info("saving network data...")

        for net, filename in zip([self.player1, self.player2],
                                 ["data_left.pkl.gz", "data_right.pkl.gz"]):
            with gzip.open(os.path.join(self.out_dir, filename), "w") as file:
                output = {"network_type": repr(net),
                          "with_noise": net.apply_noise}
                performance_data = net.get_performance_data()
                output["rewards"] = performance_data[0]
                output["weights"] = performance_data[1]
                pickle.dump(output, file)

        logging.info("Done.")


if __name__ == "__main__":
    nest.set_verbosity("M_WARNING")

    parser = argparse.ArgumentParser()
    parser.add_argument("--runs",
                        type=int,
                        default=5000,
                        help="Number of game steps to simulate.")
    parser.add_argument("--debug",
                        action="store_true",
                        help="Verbose debugging output.")
    parser.add_argument("--out_dir",
                        type=str,
                        default="",
                        help="Directory to save experiments to. Defaults to \
                        current time stamp (YYYY-mm-dd-HH-MM-SS)")
    parser.add_argument(
        "--players", nargs=2, type=str, choices=["r", "rn", "d", "dn"],
        default=["r", "rn"],
        help="""types of networks that compete against each other. four learning
        rule configuations are available: r:  r-STDP without noise, rn: r-STDP
        with noisy input, d:  dopaminergic synapses without noise,
        dn: dopaminergic synapses with noisy input.""")

    args = parser.parse_args()

    level = logging.DEBUG if args.debug else logging.INFO
    format = '%(asctime)s - %(message)s'
    datefmt = '%H:%M:%S'
    logging.basicConfig(level=level, format=format, datefmt=datefmt)

    p1, p2 = args.players
    if p1[0] == p2[0] == 'd':
        logging.error("""Nest currently (August 2022) does not support
        addressing multiple populations of dopaminergic synapses because all of
        them recieve their signal from a single volume transmitter. For this
        reason, no two dopaminergic networks can be trained simultaneously. One
        of the players needs to be changed to the R-STDP type.""")
        sys.exit()

    apply_noise = len(p1) > 1
    if p1[0] == "r":
        p1 = PongNetRSTDP(apply_noise)
    else:
        p1 = PongNetDopa(apply_noise)

    apply_noise = len(p2) > 1
    if p2[0] == "r":
        p2 = PongNetRSTDP(apply_noise)
    else:
        p2 = PongNetDopa(apply_noise)

    AIPong(p1, p2, args.out_dir).run_games(max_runs=args.runs)
