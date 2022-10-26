# -*- coding: utf-8 -*-
#
# helpers.py
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

r"""Helper functions for the Sudoku solver
----------------------------------------------------------------

:Authors: J Gille, S Furber, A Rowley
"""
import numpy as np
import matplotlib.patches as patch


def get_puzzle(puzzle_index):
    """returns one of 8 Sudoku configuration to be solved.

    Parameters
    ----------
    puzzle_index : int
        index between 0 and 7 indicating the puzzle number

    Returns
    -------
    np.array
        array of shape (9,9) representing the puzzle configuration.
        Array is zero wherever no input is given, and contains the corresponding
        digit otherwise.
    """
    init_config = None

    if puzzle_index == 0:
        # Dream problem: make the network come up with a valid sudoku without
        # any restrictions
        init_config = np.zeros((9, 9), dtype=np.uint8)
    elif puzzle_index == 1:
        # Diabolical problem:
        init_config = [[0, 0, 1,  0, 0, 8,  0, 7, 3],
                       [0, 0, 5,  6, 0, 0,  0, 0, 1],
                       [7, 0, 0,  0, 0, 1,  0, 0, 0],

                       [0, 9, 0,  8, 1, 0,  0, 0, 0],
                       [5, 3, 0,  0, 0, 0,  0, 4, 6],
                       [0, 0, 0,  0, 6, 5,  0, 3, 0],

                       [0, 0, 0,  1, 0, 0,  0, 0, 4],
                       [8, 0, 0,  0, 0, 9,  3, 0, 0],
                       [9, 4, 0,  5, 0, 0,  7, 0, 0]]

    elif puzzle_index == 2:
        init_config = [[2, 0, 0,  0, 0, 6,  0, 3, 0],
                       [4, 8, 0,  0, 1, 9,  0, 0, 0],
                       [0, 0, 7,  0, 2, 0,  9, 0, 0],

                       [0, 0, 0,  3, 0, 0,  0, 9, 0],
                       [7, 0, 8,  0, 0, 0,  1, 0, 5],
                       [0, 4, 0,  0, 0, 7,  0, 0, 0],

                       [0, 0, 4,  0, 9, 0,  6, 0, 0],
                       [0, 0, 0,  6, 4, 0,  0, 1, 9],
                       [0, 5, 0,  1, 0, 0,  0, 0, 8]]

    elif puzzle_index == 3:
        init_config = [[0, 0, 3,  2, 0, 0,  0, 7, 0],
                       [0, 0, 5,  0, 0, 0,  3, 0, 0],
                       [0, 0, 8,  9, 7, 0,  0, 5, 0],

                       [0, 0, 0,  8, 9, 0,  0, 0, 0],
                       [0, 5, 0,  0, 0, 0,  0, 2, 0],
                       [0, 0, 0,  0, 6, 1,  0, 0, 0],

                       [0, 1, 0,  0, 2, 5,  6, 0, 0],
                       [0, 0, 4,  0, 0, 0,  8, 0, 0],
                       [0, 9, 0,  0, 0, 7,  5, 0, 0]]

    elif puzzle_index == 4:
        init_config = [[0, 1, 0,  0, 0, 0,  0, 0, 2],
                       [8, 7, 0,  0, 0, 0,  5, 0, 4],
                       [5, 0, 2,  0, 0, 0,  0, 9, 0],

                       [0, 5, 0,  4, 0, 9,  0, 0, 1],
                       [0, 0, 0,  7, 3, 2,  0, 0, 0],
                       [9, 0, 0,  5, 0, 1,  0, 4, 0],

                       [0, 2, 0,  0, 0, 0,  4, 0, 8],
                       [4, 0, 6,  0, 0, 0,  0, 1, 3],
                       [1, 0, 0,  0, 0, 0,  0, 2, 0]]

    elif puzzle_index == 5:
        init_config = [[8, 9, 0,  2, 0, 0,  0, 7, 0],
                       [0, 0, 0,  0, 8, 0,  0, 0, 0],
                       [0, 4, 1,  0, 3, 0,  5, 0, 0],

                       [2, 5, 8,  0, 0, 0,  0, 0, 6],
                       [0, 0, 0,  0, 0, 0,  0, 0, 0],
                       [6, 0, 0,  0, 0, 0,  1, 4, 7],

                       [0, 0, 7,  0, 1, 0,  4, 3, 0],
                       [0, 0, 0,  0, 2, 0,  0, 0, 0],
                       [0, 2, 0,  0, 0, 7,  0, 5, 1]]

    elif puzzle_index == 6:
        # "World's hardest sudoku":
        # http://www.telegraph.co.uk/news/science/science-news/9359579/Worlds-hardest-sudoku-can-you-crack-it.html
        init_config = [[8, 0, 0,  0, 0, 0,  0, 0, 0],
                       [0, 0, 3,  6, 0, 0,  0, 0, 0],
                       [0, 7, 0,  0, 9, 0,  2, 0, 0],

                       [0, 5, 0,  0, 0, 7,  0, 0, 0],
                       [0, 0, 0,  0, 4, 5,  7, 0, 0],
                       [0, 0, 0,  1, 0, 0,  0, 3, 0],

                       [0, 0, 1,  0, 0, 0,  0, 6, 8],
                       [0, 0, 8,  5, 0, 0,  0, 1, 0],
                       [0, 9, 0,  0, 0, 0,  4, 0, 0]]

    elif puzzle_index == 7:
        init_config = [[1, 0, 0,  4, 0, 0,  0, 0, 0],
                       [7, 0, 0,  5, 0, 0,  6, 0, 3],
                       [0, 0, 0,  0, 3, 0,  4, 2, 0],

                       [0, 0, 9,  0, 0, 0,  0, 3, 5],
                       [0, 0, 0,  3, 0, 5,  0, 0, 0],
                       [6, 3, 0,  0, 0, 0,  1, 0, 0],

                       [0, 2, 6,  0, 5, 0,  0, 0, 0],
                       [9, 0, 4,  0, 0, 6,  0, 0, 7],
                       [0, 0, 0,  0, 0, 8,  0, 0, 2]]

    else:
        raise ValueError(f"No puzzle for index {puzzle_index} found.")

    return np.array(init_config)


def validate_solution(puzzle, solution):
    """validate a proposed solution for a sudoku puzzle

    Parameters
    ----------
    puzzle : np.array
        array of shape (9,9) encoding the puzzle.
        see get_puzzle().
    solution : np.array
        array of shape (9,9) encoding the proposed solution.

    Returns
    -------
    bool
        True if the overall solution is valid, False otherwise.
    np.array (3,3)
        True wherever a 3x3 box is valid
    np.array (9,)
        True wherever a row is valid
    np.array (9,)
        True wherever a column is valid
    """

    boxes = np.ones((3, 3), dtype=bool)
    rows = np.ones(9, dtype=bool)
    cols = np.ones(9, dtype=bool)

    expected_numbers = set(range(1, 10))

    # validate boxes
    for i in range(3):
        for j in range(3):
            box = solution[3*i:3*i+3, 3*j:3*j+3]
            if expected_numbers != set(box.flatten()):
                boxes[i, j] = False

    # validate rows and columns
    for i in range(9):
        if expected_numbers != set(solution[i, :]):
            rows[i] = False
        if expected_numbers != set(solution[:, i]):
            cols[i] = False

    # It is possible (in rare cases) that the network finds a valid
    # solution that does not conform to the initial puzzle configuration, that is,
    # one of the cells where input is applied is overridden by the rest of the
    # network. This is taken care of here.
    input_cells = np.where(puzzle != 0)
    puzzle_matched = puzzle[input_cells] == solution[input_cells]

    # overall solution is valid iff all of the components are.
    valid = boxes.all() and rows.all() and cols.all() and puzzle_matched.all()

    return valid, boxes, rows, cols


def plot_field(puzzle, solution, ax, with_color=False):
    """generates a graphical representation of a Sudoku field. Digits that are
    given by the puzzle are represented as bold and black, while calculated
    digits are represented in grey and italic.

    Parameters
    ----------
    puzzle : np.array
        array of shape (9,9) that represents the puzzle that is being solved.
        See get_puzzle()
    solution : np.array
        array of shape (9,9) representing the solution.
    ax : plt.Axes
        Axes object on which to draw the field.
    with_color : bool
        if True, green and red are used to
        indicate which parts of the solution are valid and which are not.
        Otherwise, only black and white are used. Defaults to False.
    """
    decorate_sudoku_box(ax)
    fill_numbers(ax, puzzle, solution)

    # color every other box in light grey
    for i in range(0, 9, 3):
        for j in range(0, 9, 3):
            if (i+j) % 2 == 0:
                ax.add_patch(patch.Rectangle((j, i), 3, 3, facecolor="grey", alpha=0.5))

    # Plotting invisible lines ensures a consistent scaling of the padding around
    # the plot even when no hbars or vbars are drawn to indicate errors in the solution.
    ax.vlines(0, -0.5, 9.5, alpha=0)
    ax.hlines(0, -0.5, 9.5, alpha=0)

    if with_color:
        valid, boxes, rows, cols = validate_solution(puzzle, solution)

        # draw red lines around the field to indicate validity of rows and cols
        for f, values in zip((ax.vlines, ax.hlines), (rows, cols)):
            locations = np.where(np.invert(values))[0] + 0.5
            f(locations, -0.5, -0.1, color="red", linewidth=4)
            f(locations, 9.1, 9.5, color="red", linewidth=4)

        # draw a red frame around invalid boxes
        for i in range(3):
            for j in range(3):
                if not boxes[i, j]:
                    ax.add_patch(patch.Rectangle((3*j, 3*j), 3, 3,
                                                 color="red", fill=False, linewidth=3.5))


def fill_numbers(ax, puzzle, solution):
    """Fill the digits of a proposed solution into an Axes object which represents the
    Sudoku field.

    Parameters
    ----------
    ax : plt.Axes
        Axes in which to draw the numbers
    puzzle : np.array
        array of shape (9,9) that represents the puzzle that is being solved.
        See get_puzzle()
    solution : np.array
        array of shape (9,9) representing the solution.
    """
    for i in range(9):
        for j in range(9):
            if solution[i][j] == 0:
                continue
            if puzzle[i][j] != 0:
                text_style = 'normal'
                if puzzle[i][j] == solution[i][j]:
                    text_col = 'black'
                else:
                    # If the network proposes a solution where a digit
                    # from the input configuration is altered, that
                    # digit is colored in red.
                    text_col = 'red'
            else:
                text_style = 'italic'
                text_col = 'gray'

            ax.text(j + 0.5, i + 0.5, solution[i][j], horizontalalignment='center',
                    verticalalignment='center', color=text_col, fontsize=15, style=text_style)


def decorate_sudoku_box(ax):
    """Decorate the Axes object to resemble an empty sudoku field.

    Parameters
    ----------
    ax : plt.Axes
        Axes to be decorated
    """
    [x, y, xr, yr] = generate_sudoku_box_lines()
    ax.plot(x, y, color='gray')
    ax.plot(y, x, color='gray')

    ax.plot(xr, yr, color='black', linewidth=2)
    ax.plot(yr, xr, color='black', linewidth=2)

    ax.axis("off")
    ax.set_aspect("equal")


def generate_sudoku_box_lines():
    """Generate coordinates for the lines that divide the Sudoku field

    Returns
    -------
    list
        List of 4 lists that represent the coordinates which separate
        the Sudoku field. The first pair is used to separate all cells, and
        the second pair is drawn around 3x3 boxes.
    """

    # lines for cells
    x = []
    y = []
    # lines for regions
    xr = []
    yr = []

    for i in range(10):
        if i % 3 != 0:
            x.extend([i, i, None])
            y.extend([0, 9, None])
        else:
            xr.extend([i, i, None])
            yr.extend([0, 9, None])

    return [x, y, xr, yr]
