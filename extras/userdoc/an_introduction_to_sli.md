<!-- TOC -->
-   [About NEST](about.md)
-   [Download](download.md)
-   [Features](features.md)
-   [Documentation](documentation.md)
    -   [Installing NEST](installation.md)
    -   [Introduction to PyNEST](introduction-to-pynest.md)
        -   [Part 1: Neurons and simple neural networks](part-1-neurons-and-simple-neural-networks.md)
        -   [Part 2: Populations of neurons](part-2-populations-of-neurons.md)
        -   [Part 3: Connecting networks with synapses](part-3-connecting-networks-with-synapses.md)
        -   [Part 4: Topologically structured networks](part-4-topologically-structured-networks.md)
    -   [Example Networks](examples/examples.md)
    -   [FAQ](frequently_asked_questions.md)
    -   [Developer Manual](http://nest.github.io/nest-simulator/)
-   [Publications](publications.md)
-   [Community](community.md)

<!-- /TOC -->

An Introduction to SLI
======================

[ [Documentation](documentation.md "Documentation") ]

<table>
<colgroup>
<col width="50%" />
<col width="50%" />
</colgroup>
<tbody>
<tr class="odd">
<td align="left"><h2>Contents</h2>
<ul>
<li><a href="#Introduction">1 Introduction</a></li>
<li><a href="#Command_line_switches">2 Command line switches</a>
<ul>
<li><a href="#Supplying_SLI_scripts_with_parameters">2.1 Supplying SLI scripts with parameters</a></li>
</ul></li>
<li><a href="#SLI_user_manual">3 SLI user manual</a></li>
</ul></td>
</tr>
</tbody>
</table>

Introduction
------------

NEST can be started by typing

``` {.prettyprint}
<prefix>/bin/nest
```

at the command prompt. You should then see something like this:

``` {.prettyprint}
gewaltig@jasmin-vm:~$ nest
            -- N E S T 2 beta --

  Copyright 1995-2009 The NEST Initiative
   Version 1.9-svn Feb  6 2010 00:33:50

This program is provided AS IS and comes with
NO WARRANTY. See the file LICENSE for details.

Problems or suggestions?
  Website     : <a class="external free" href="http://www.nest-initiative.org" rel="nofollow">http://www.nest-initiative.org</a>
  Mailing list: nest_user@nest-initiative.org

Type 'help' to get more information.
Type 'quit' or CTRL-D to quit NEST.

SLI ]
```

Command line switches
---------------------

Type

``` {.prettyprint}
nest --help
```

to find out about NEST's command-line parameters.

``` {.prettyprint}
gewaltig@jasmin-vm:~$ nest --help
usage: nest [options] [file ..]
  -h  --help                print usage and exit
  -v  --version             print version information and exit.

  -   --batch               read input from a stdin/pipe
      --userargs=arg1:...   put user defined arguments in statusdict::userargs
  -d  --debug               start in debug mode (implies --verbosity=ALL)
      --verbosity=ALL       turn on all messages.
      --verbosity=DEBUG|STATUS|INFO|WARNING|ERROR|FATAL
                            show messages of this priority and above.
      --verbosity=QUIET     turn off all messages.
```

### Supplying SLI scripts with parameters

Using the `--userargs=arg1:...` command line switch, it is possible to supply a SLI script with parameters from the outside of NEST. A common use case for this are parameter sweeps, where the parameters are defined in a bash script and multiple instances of NEST are used to test one parameter each. A bash script for this could look like this:

``` {.prettyprint}
for lambda in `seq 1 20`; do
  for gamma in `seq 1 5`; do
    nest --userargs=lambda=$lambda:$gamma=$gamma simulation.sli
  done
done
```

The corresponding SLI script `simulation.sli` could use the supplied parameters like this:

``` {.prettyprint}
/args mark statusdict/userargs :: {(=) breakup} Map { arrayload pop int exch cvlit exch } forall >> def
args /lambda get ==
```

The first line first gets the array of user supplied arguments (`userargs`) from the `statusdict` and breaks each element at the "="-symbol. It then converts the first element (lambda, gamma) to a literal and the second argument (the number) to an integer. Using `mark` and `>>`, the content of the userargs array is added to a dictionary, which is stored under the name `args`. The second line just prints the content of the lamda variable.

SLI user manual
---------------

This manual gives a brief overview of the SLI programming language.

1.  [First Steps](http://www.nest-simulator.org/first_steps/ "First Steps")
2.  [Objects and data types](http://www.nest-simulator.org/objects_and_data_types/ "Objects and data types")
3.  [Programming in SLI](http://www.nest-simulator.org/programming_in_sli/ "Programming in SLI")
4.  [Using files and keyboard input](http://www.nest-simulator.org/using_files_and_keyboard_input/ "Using files and keyboard input")
5.  [Neural simulations](http://www.nest-simulator.org/neural_simulations/ "Neural simulations")
