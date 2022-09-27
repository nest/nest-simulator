# -*- coding: utf-8 -*-
#
# pong.py
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

r"""Classes for running simulations of the classic game Pong
----------------------------------------------------------------
The Class GameOfPong contains all necessary functionality for running simple
simulations of Pong games.

See Also
---------
`Original implementation <https://github.com/electronicvisions/model-sw-pong>`_

References
----------
.. [1] Wunderlich T., et al (2019). Demonstrating advantages of
       neuromorphic computation: a pilot study. Frontiers in neuroscience, 13,
       260. https://doi.org/10.3389/fnins.2019.00260

:Authors: T Wunderlich, Electronic Vision(s), J Gille
"""
import numpy as np

LEFT_SCORE = -1
RIGHT_SCORE = +1
GAME_CONTINUES = 0

MOVE_DOWN = -1
MOVE_UP = +1
DONT_MOVE = 0


class GameObject:
    def __init__(self, game, x_pos=0.5, y_pos=0.5, velocity=0.2,
                 direction=(0, 0)):
        """Base class for Ball and Paddle that contains basic functionality for
        an object inside a game.

        Args:
            game (GameOfPong): Instance of Pong game.
            x_pos (float, optional): Initial x position. Defaults to 0.5.
            y_pos (float, optional): Initial y position. Defaults to 0.5.
            velocity (float, optional): Change in position per iteration.
            Defaults to 0.2.
            direction (list, optional): direction vector. Defaults to [0,0].
        """
        self.x_pos = x_pos
        self.y_pos = y_pos
        self.velocity = velocity
        self.direction = direction
        self.game = game
        self.update_cell()

    def get_cell(self):
        return self.cell

    def get_pos(self):
        return self.x_pos, self.y_pos

    def update_cell(self):
        """Update the cell in the game grid based on position.
        """
        x_cell = int(np.floor(
            (self.x_pos / self.game.x_length) * self.game.x_grid))
        y_cell = int(np.floor(
            (self.y_pos / self.game.y_length) * self.game.y_grid))
        self.cell = [x_cell, y_cell]


class Ball(GameObject):
    """Class representing the ball.

        Args:
            radius (float): Radius of ball in unit length.

        For other args, see :class:`GameObject`.
    """

    def __init__(self, game, x_pos=0.8, y_pos=0.5, velocity=0.025,
                 direction=(-1 / 2., 1 / 2.), radius=0.025):
        super().__init__(game, x_pos, y_pos, velocity, direction)
        self.ball_radius = radius  # Unit length
        self.update_cell()


class Paddle(GameObject):
    """Class representing the paddles on either end of the playing field.

        Args:
            direction (int, optional): Either -1, 0, or 1 for downward, neutral
            or upwards motion, respectively. Defaults to 0.
            left (boolean): If True, paddle is placed on the left side of the
            board, otherwise on the right side.

        For other args, see :class:`GameObject`.
    """
    length = 0.2  # Paddle length in the scale of GameOfPong.y_length

    def __init__(self, game, left, y_pos=0.5, velocity=0.05, direction=0):
        x_pos = 0. if left else game.x_length
        super().__init__(game, x_pos, y_pos, velocity,
                         direction)
        self.update_cell()

    def move_up(self):
        self.direction = MOVE_UP

    def move_down(self):
        self.direction = MOVE_DOWN

    def dont_move(self):
        self.direction = DONT_MOVE


class GameOfPong(object):
    """Class representing a game of Pong. Playing field is 1.6 by 1.0 units
    in size, discretized into x_grid*y_grid cells.
    """
    x_grid = 32
    y_grid = 20
    x_length = 1.6
    y_length = 1.0

    def __init__(self):
        self.r_paddle = Paddle(self, False)
        self.l_paddle = Paddle(self, True)

        self.reset_ball()
        self.result = 0

    def reset_ball(self, towards_left=False):
        """Reset the ball position to the center of the field after a goal.

        Args:
            towards_left (bool, optional): if True, ball direction is
            initialized towards the left side of the field, otherwise towards
            the right. Defaults to False.
        """
        initial_vx = 0.5 + 0.5 * np.random.random()
        initial_vy = 1. - initial_vx
        if towards_left:
            initial_vx *= -1
        initial_vy *= np.random.choice([-1., 1.])

        self.ball = Ball(self, direction=[initial_vx, initial_vy])
        self.ball.y_pos = np.random.random() * self.y_length

    def update_ball_direction(self):
        """In case of a collision, update the direction of the ball. Also
        determine if the ball is in either player's net.

        Returns:
            Either GAME_CONTINUES, LEFT_SCORE or RIGHT_SCORE depending on ball
            and paddle position.
        """
        if self.ball.y_pos + self.ball.ball_radius >= self.y_length:
            # Ball on upper edge
            self.ball.direction[1] = -1 * abs(self.ball.direction[1])
        elif self.ball.y_pos - self.ball.ball_radius <= 0:
            # Ball on lower edge
            self.ball.direction[1] = abs(self.ball.direction[1])

        if self.ball.x_pos - self.ball.ball_radius <= 0:
            # Ball on left edge
            if abs(self.l_paddle.y_pos - self.ball.y_pos) <= Paddle.length / 2:
                # Ball hits left paddle
                self.ball.direction[0] = abs(self.ball.direction[0])
            else:
                return RIGHT_SCORE
        elif self.ball.x_pos + self.ball.ball_radius >= self.x_length:
            # Ball on right edge
            if abs(self.r_paddle.y_pos - self.ball.y_pos) <= Paddle.length / 2:
                # Ball hits right paddle
                self.ball.direction[0] = -1 * abs(self.ball.direction[0])
            else:
                return LEFT_SCORE
        return GAME_CONTINUES

    def propagate_ball_and_paddles(self):
        """Update ball and paddle coordinates based on direction and velocity.
        """
        for paddle in [self.r_paddle, self.l_paddle]:
            paddle.y_pos += paddle.direction * paddle.velocity
            if paddle.y_pos < 0:
                paddle.y_pos = 0
            if paddle.y_pos > self.y_length:
                paddle.y_pos = self.y_length
            paddle.update_cell()
        self.ball.y_pos += self.ball.velocity * self.ball.direction[1]
        self.ball.x_pos += self.ball.velocity * self.ball.direction[0]
        self.ball.update_cell()

    def get_ball_cell(self):
        return self.ball.get_cell()

    def step(self):
        """Perform one game step by handling collisions, propagating all game
        objects and returning the new game state.

        Returns:
            Either GAME_CONTINUES, LEFT_SCORE or RIGHT_SCORE depending on ball
            and paddle position. see update_ball_direction()
        """
        ball_status = self.update_ball_direction()
        self.propagate_ball_and_paddles()
        self.result = ball_status
        return ball_status
