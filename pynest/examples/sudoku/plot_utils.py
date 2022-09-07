#%%
"""
This module contains the functions needed to graphically display
the solution to Sudoku puzzle. This module depends on matplotlib
library. The graphical output displays the cell evaluated in each
iteration, time taken to solve and shades the cells based on difficulty.
"""

__author__ = 'krishnakumarramamoorthy'

import time

import matplotlib.pyplot as plt
import matplotlib.patches as patch

import numpy as np


def visualize_solution(input_matrix, solved_matrix, ts, out_path):
    """
    Plotting function that takes sudoku solution and intermediate data
    and plots them. It create three axes, one for plotting the progress,
    one for printing the summary metrics, and one for solved sudoku.
    """
    # prepare figure window for plotting
    plt.figure().patch.set_facecolor('white')
    plt.suptitle('Sudoku Solver\n', fontsize=20, color=plt.cm.Blues(0.9))

    # plot the performance: number of iterations vs. numbers filled
    ax1 = plt.subplot2grid((20, 3), (1, 0), rowspan=17, colspan=2)
    # plot_decorate_performance_data(ax1, x, y, ylabels)

    # work on sudoku box area
    ax2 = plt.subplot2grid((20, 3), (10, 2), rowspan=10, colspan=1)
    decorate_sudoku_box(ax2)
    fill_numbers(ax2, input_matrix, solved_matrix)

    # work on statistics area
    ax3 = plt.subplot2grid((20, 3), (1, 2), rowspan=7, colspan=1)
    # write_statistics(ax3, time_to_solution, time_to_plot, total_iterations)

    # save figure and show
    plt.savefig(out_path)
    plt.show()



def plot_decorate_performance_data(ax1, x, y, ylabels):
    """
    Modify the objects in performance plot. Add labels, resize font, etc.
    """
    ax1.plot(x, y)
    ax1.set_title('Progress', fontsize=11, color='black', alpha=0.9)
    plt.xlabel('How long did it take to solve? \n (Number of Evaluations)', fontsize=10)
    plt.ylabel('Which cell was evaluated? (Unfilled Cell Index)', fontsize=10)
    # label the y-axis with the cell location in sudoku grid
    plt.yticks(y, ylabels, color='gray', fontsize=8)
    plt.xticks(color='gray')


def decorate_sudoku_box(ax2):
    """
    Modify line and color properties of sudoku grid to make it look good
    """
    [x, y, xr, yr] = generate_sudoku_box_lines()
    ax2.set_title('Solution\n\n', fontsize=11, color='black', alpha=0.9)
    ax2.plot(x, y, color='gray')
    ax2.plot(xr, yr, color='gray', linewidth=2)
    # move the x tick to top
    ax2.xaxis.tick_top()
    # turn off the tick markers
    ax2.tick_params(bottom='off', top='off', left='off', right='off')
    # rewrite the tick labels so that they are aligned at the center of each box and not at the border
    plt.xticks(np.arange(0.5, 9.5, 1), np.arange(1, 10, 1), color='gray', fontsize=8)
    # start numbering y labels from the top
    plt.yticks(np.arange(0.5, 9.5, 1), np.arange(1, 10, 1), color='gray', fontsize=8)


def fill_numbers(ax2, input_matrix, solved_matrix):
    # fill the numbers
    for i in range(9):
        for j in range(9):
            # quirk: when plotting matrix, transpose it; i is in y-axis and j is in x-axis
            if input_matrix[i][j] == 0:
                ax2.text(j + 0.5, i + 0.5, solved_matrix[i][j], horizontalalignment='center',
                         verticalalignment='center', color='black', fontsize=10)
            else:
                ax2.text(j + 0.5, i + 0.5, solved_matrix[i][j], horizontalalignment='center',
                         verticalalignment='center', color='gray', alpha=0.7, fontsize=10)


def write_statistics(ax3, time_to_solution, time_to_plot, total_iterations):
    ax3.get_xaxis().set_visible(False)
    ax3.get_yaxis().set_visible(False)
    ax3.spines['left'].set_color('white')
    ax3.spines['right'].set_color('white')
    ax3.spines['top'].set_color('white')
    ax3.spines['bottom'].set_color('white')
    ax3.text(0.5, 0.9, '{:.2f} secs\n'.format(time_to_solution),
             horizontalalignment='center', verticalalignment='center', fontsize=16)
    ax3.text(0.5, 0.85, 'Time to solution', horizontalalignment='center', verticalalignment='center',
             fontsize=12, color='gray')
    ax3.text(0.5, 0.6, '{:.2f} secs\n'.format(time_to_plot), horizontalalignment='center',
             verticalalignment='center', fontsize=16)
    ax3.text(0.5, 0.55, 'Time to plot', horizontalalignment='center', verticalalignment='center',
             fontsize=12, color='gray')
    ax3.text(0.5, 0.3, '{} \n'.format(total_iterations), horizontalalignment='center',
             verticalalignment='center', fontsize=16)
    ax3.text(0.5, 0.25, 'Number of evaluations', horizontalalignment='center', verticalalignment='center',
             fontsize=12, color='gray')


def generate_sudoku_box_lines():
    # lines for cells
    x = []
    y = []
    # lines for regions
    xr = []
    yr = []

    # data for vertical lines
    for i in range(9):
        x.append(i)
        x.append(i)
        y.append(0)
        y.append(9)
        x.append(None)
        y.append(None)
        if divmod(i, 3)[1] == 0:
            xr.append(i)
            xr.append(i)
            yr.append(0)
            yr.append(9)
            xr.append(None)
            yr.append(None)

    # data for horizontal lines
    for j in range(9):
        x.append(0)
        x.append(9)
        y.append(j)
        y.append(j)
        x.append(None)
        y.append(None)
        if divmod(j, 3)[1] == 0:
            xr.append(0)
            xr.append(9)
            yr.append(j)
            yr.append(j)
            xr.append(None)
            yr.append(None)

    return [x, y, xr, yr]


# %%
