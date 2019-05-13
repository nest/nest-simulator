
Restructured Text
------------------

Restructured Text is a markup language designed for technical documentation writing
and is used in NEST documentation.

Here, we've included some useful directives and roles for reST.
Please follow the reST guidelines and the corresponding templates for writing docs,
as this will help produce consistent, clear and beautiful documentation!

For further details see http://www.sphinx-doc.org/en/master/usage/restructuredtext/index.html



Basic Formatting
------------------

Indentation is significant in reST, so all lines of the same paragraph must be
left-aligned to the same level of indentation. Paragraphs, titles, and directives
should be separated from other text by blank lines.

 one asterisk: *text* for emphasis (italics),
 two asterisks: **text** for strong emphasis (boldface), and
 backquotes (literal): ``text`` for inline code.
 bullet lists begin with *
 numbered lists begin with #



Title
=======

Subsection
------------

Subsubsection
~~~~~~~~~~~~~~~

Subsubsubsection
^^^^^^^^^^^^^^^^^


A few examples of roles and directives
--------------------------------------

Literal code blocks can be introduced with the special marker :: .
The literal block must be indented and separated by blank lines

This is a normal text paragraph. The next paragraph is a code sample::

   Here is the code sample
   Note that it is indented

   It can span multiple lines.

This is a normal text paragraph again.


Alternatively you can use the code-block directive and specify the language

.. code-block:: python

   code here

Math is based on the LaTeX syntax and uses the reST math directive

.. math::

   equation here

or :math:`X-1` for inline math. Note the use of back ticks.


Referencing bibliographies
---------------------------

For in-text citations, we use the reST numeric style [1]_.

Any time you cite a reference in your text you need to include full citation
 it in its own section called References:

 References
 -----------

 .. [1] Smith J. and Jones M. 2009. Title of cool paper. Journal of Awesomeness.
       3:7-29. doi link

Note the authors first names are initials and always follow the surname. If more
you have more than 3 authors use et al.

Cross-referencing and hyperlinks
----------------------------------------

All titles are considered internal hyperlinks and can be used to cross-
reference sections :

  You can include a link to a section by writing `Section_title`_
  Note the underscore at the end.

Linking to other documents in the NEST repository:

   :doc:`link text <relative/path/to/file>`
   or
   :doc:`filename`


To link to an exernal webpage you can simply write the URL, or you can use
link text:

    `link text <https://www.nest-simulator.org>`_


