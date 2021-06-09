The NEST documentation style guide
==================================

.. contents:: On this page, you will find
   :local:
   :depth: 1

Why do we have a style guide?
-----------------------------

This style guide was created to provide a single reference point for content
creation in NEST. This helps ensure the NEST user-level documentation remains
clear and consistent. The style choices we make here are meant to follow the
major trends in technical writing for software.

Contribute to the docs
~~~~~~~~~~~~~~~~~~~~~~~

If you want to add or modify documentation, make sure to read through this guide before writing a contribution.

We have templates for :doc:`Python example scripts <../templates/pyapi_template>` and the :doc:`PyNEST API docstrings <../templates/example_template>`.

You will also need to know how to build the documentation locally on your machine: See
:doc:`../../documentation_workflow/user_documentation_workflow`.

For additional information, including the Git workflow and making a pull request see :doc:`../index`.


A few tips on writing documentation
-----------------------------------

- Be concise: Use short words and sentences; avoid unnecessary modifiers.

- Be consistent: Follow this style guide and relevant templates,

- Be specific: Avoid vague language; clarity is key.

- Focus your message: Start with what is most important, and keep text relevant
  within given section.

- Write for your reader: Try to understand what your reader needs to know;
  include context to what you are saying. Remember that most NEST users are computational neuroscientists,
  including students, not software developers!

- Provide examples: Show, do not tell; if you can use code snippets, screenshots,
  or figures to illustrate a point, do so!

- Avoid terms like "simply", "just", or "easy": Your knowledge does not always equal
  your readers.  Do not make your reader feel bad because they "simply" did not
  understand something.

- Write positively: Use positive language rather than negative language.

Remember that the primary purpose of the documentation is to provide
information to NEST users who are looking for it.

The language we use
-------------------

* We follow spelling and grammar rules of `American English <https://www.merriam-webster.com/>`_.

* The markup language we use is :ref:`reStructuredText <sec-reST_style>`. This includes documentation
  blocks in files written in Python and C++.

* We avoid slang or jargon that is unnecessary or could be confusing to readers.

We do, however, use terminology appropriate to NEST and neuroscience when
specific and exact terms are needed, which is often. But explanations or
references should be provided for clarity in introductory texts and the :ref:`sec_glossary`.

If you have further questions about style rules not addressed here, use
`the Microsoft style guide <https://docs.microsoft.com/en-us/style-guide/welcome/>`_ as a resource
or ask on the :ref:`mailing list <nest_community>`.

How to phrase content in NEST
-----------------------------

Voice
~~~~~~

Voice is *how* we talk to people through writing. The voice encompasses
substance, style, and tone.

Our voice is

* empathetic: We write thinking of whom we are writing for and what their needs
  are.
* relaxed: We write in a natural, conversational way rather than an authoritative
  way.
* factual: We write in a neutral style avoiding irony, humor, or cultural references.

Use "you" to indicate the reader and "we" to indicate yourself (the writer and
possibly the NEST team).


Active voice
~~~~~~~~~~~~

* Prefer the active voice, where the subject acts on object.

* If the object needs emphasis rather than the subject, use passive voice. But ensure
  that you cannot improve the sentence by using the active voice.


Headings and subheadings
~~~~~~~~~~~~~~~~~~~~~~~~

* Headings and subheadings describe the purpose of the section.

* Begin with a descriptive verb or begin with "How to ..."

* Use the verb stem and not the gerund ("ing") form of the verb. Use "Add a
  model" instead of  "Adding a model."

* Avoid section names like "Introduction" or "Part 1".

* One-word subheadings are acceptable, if the section is short and the meaning is clear.

* Use sentence case for headings and subheadings, that is, begin with an uppercase
  letter but with all other words in lower case (except proper nouns).


+-----------------------------------+----------------+
| Good examples:                    | Bad examples:  |
+===================================+================+
| Create your first neural network  | Start here     |
+-----------------------------------+----------------+
| How to set up and configure MUSIC | MUSIC and NEST |
+-----------------------------------+----------------+
| Add a device to your network      | Adding devices |
+-----------------------------------+----------------+

Sentences and paragraphs
~~~~~~~~~~~~~~~~~~~~~~~~

* Avoid using "so" in sentences.

* Try to keep sentences short, or break up long sentences with short ones.

* Avoid lengthy paragraphs with more than 5 or 6 sentences.
  If writing multiple paragraphs, they should be broken up by example code, figures, or bullet lists.

* Keep in mind that texts should be skimmable.

Pronouns
~~~~~~~~

* Use the pronouns "you" to indicate the reader and "we" to indicate NEST and its members.

* Avoid the pronoun "I."

* Avoid gendered terms (e.g., use "police officer" instead of "policeman").

* Instead of "guys" or "girls" use inclusive language such as everyone, all,
  members, or folks.

* "They" is an acceptable singular third person pronoun
  (see `the dictionary definition here <https://www.merriam-webster.com/dictionary/they>`_).

How to write specific content in NEST
-------------------------------------

Numbers
~~~~~~~

* Numbers 0–9 should be spelled out, unless they are measurements or coordinates.

* Numbers should be spelled out if they begin a sentence. In most cases, however,
  the numeral/ordinal format is preferred.

* We use the period for the decimal point (`57.45`).

* The thousand separator is the comma except when showing a code example.

   Example:

   We have over 5,000 connections.
   The number of connections is ``x = 5001``.

* Make sure you use the correct unit (e.g., millivolts for voltage) and the
  unit's symbol (mV).

* For additional mathematical notation, use the :ref:`math role or directive <math_style>`.

Lists
~~~~~

* Use the serial comma in lists.

* Use numbered lists for step-by-step instructions only. Ensure that each step contains only one or two actions.

* Use bullet lists if the number of items is extensive or each item is a long phrase or sentence.

* If the text of a list forms a complete sentence, use proper punctuation and
  end with period.

* If the text of a list forms an incomplete sentence, do not end with period.

* If the entire bullet/numbered list belongs to a sentence, end each item with a comma and the second-last item with ", and".

* If the last item is the end of the sentence, end it with a period. Otherwise, use the punctuation required to correctly connect
  to the remainder of the sentence.


Abbreviations and acronyms
~~~~~~~~~~~~~~~~~~~~~~~~~~

* Spell out acronyms on first appearance on each page or article it appears, for example: Random number generator (RNG).

* If the abbreviation/acronym is well known (e.g., HTML), you do not need to spell
  it out.

* Use "e.g.," and "i.e.," only in parentheses and figure and table captions; otherwise, use "for example," and "that is,".
  Note the use of comma following the terms.


Contractions
~~~~~~~~~~~~

* Avoid contractions. For example, use "do not" instead of "don't".

Commas
~~~~~~

* Use the serial comma (apples, bananas, and grapes) for lists.

* Use the comma as separator for thousands (37,000).

* To join two sentences into one, you must use a conjunction (and, or, but)
  along with the comma, or use the semicolon.


Ampersand
~~~~~~~~~

* Avoid the ampersand "`&`" and use "`and`" instead, unless the ampersand is part
  of a proper name (e.g., Ben \& Jerry's).

* The ampersand is a special character and can be used, for example, to :ref:`align multi-line equations <math_style>`.


Capitalization
~~~~~~~~~~~~~~

* Capitalize the first word of a heading, but use lower case for the rest.

* Capitalize first word in bullet/numbered list.

* Capitalize proper nouns and follow company policy in naming conventions.
  (e.g., macOS, LaTeX, Python, NumPy, NEST).

* Capitalize the first word after a colon.

.. _sec-reST_style:

reStructuredText markup
-----------------------

reStructuredText is a plain text markup language and parser. It is the default language of the Sphinx documentation
generator, which NEST uses for generating documentation.

reStructuredText uses directives, which are blocks of explicit markup used for math, images, code, admonitions, and much
more. The syntax looks like this ``.. directive-name::``. The directive content follows the directive name after a blank
line and is indented relative to the directive start.

In addition to directives, reStructuredText has roles, which insert semantic markup into documents.
Roles look like this ``:role-name:`content```.

We will only cover a few examples here. You can find more information at the following links:


* `reStructuredText User Documentation <https://docutils.sourceforge.io/rst.html#id24>`_

* `reStructuredText Primer <https://www.sphinx-doc.org/en/master/usage/restructuredtext/basics.html>`_

* `Sphinx directives <https://www.sphinx-doc.org/en/master/usage/restructuredtext/directives.html>`_

* `Sphinx roles <https://www.sphinx-doc.org/en/master/usage/restructuredtext/roles.html>`_


.. note::

   reStructuredText is sensitive to indentation and newlines.

   * Directives, headings, labels, and tables should be separated from other text by a newline, excluding directive options.
   * Directive options must be indented to the same level as the directive content.
   * Text in multiline content should stay aligned with the first line.
   * If the output format seems incorrect, double check the spaces and newlines
     of the text.


Code and code snippets
~~~~~~~~~~~~~~~~~~~~~~

Code blocks are written by using the code-block directive.

Example syntax

   .. code-block:: none

    .. code-block:: cpp

        int main()
        {
          std::cout << "Hello, World!";
          return 0;
        }


Code is rendered as

    .. code-block:: cpp

        int main()
        {
          std::cout << "Hello, World!";
          return 0;
        }


For Python examples that show input and output, use the following syntax::

   >>> input begins with 3, right-angled brackets
   Output is directly below input without any brackets.
   A blank line must end the example.

For in-text code use the role :code: or double backticks::

   ``cout << "Hello, World!`` or
   :code:`cout << "Hello, World!"`

.. _math_style:

Math equations
~~~~~~~~~~~~~~

The input language for mathematics is LaTeX markup. See `Mathematics into Type
<http://www.ams.org/arc/styleguide/mit-2.pdf>`_ for a guide to styling LaTeX math.


For equations that take a whole line (or more), use the math directive::

    .. math::

        f(x) = \int_{-\infty}^{\infty} \hat{f}(\xi) e^{2 \pi i x \xi} \, d\xi.

Output rendered as

    .. math::

        f(x) = \int_{-\infty}^{\infty} \hat{f}(\xi) e^{2 \pi i x \xi} \, d\xi.

If the equation runs over several lines you can use double backslashes ``\\`` as a separator at the end of each line.
You can also align lines in an equation by using the ``&`` where you want an equation aligned::


    .. math::

         (a + b)^2  &=  (a + b)(a + b) \\
                    &=  a^2 + 2ab + b^2

Rendered as

    .. math::

         (a + b)^2  &=  (a + b)(a + b) \\
                    &=  a^2 + 2ab + b^2

For in-text math, use the math role::

   Now we can see :math:`x=1` for this example.

Rendered as

   Now we can see :math:`x=1` for this example.

.. _sec_admonition:

Admonitions
~~~~~~~~~~~

Admonitions are directives that render as highlighted blocks to draw the reader's attention to a particular point.

Use them sparingly.


Use the admonition

* "See also" to reference internal or external links (only in cases where the reference should stand out),

* "Note" to add additional information that the reader needs to be aware of,

* "Warning" to indicate that something might go wrong without the provided information, and

* "Danger" if the situation may cause severe, possibly irreversible, problems.


If you want a custom admonition, use:

.. code-block:: none

   .. admonition:: Custom label

      Here is some text

Rendered as


   .. admonition:: Custom label

         Here is some text


References
~~~~~~~~~~

For referencing reStructuredText files within the documentation, use the ``:doc:`` role. It requires the relative path to
the file::

   :doc:`sample_doc`

In this case, the link text will be the title of the given document:

   :doc:`sample_doc`

You can specify the text you want to use for the link by doing the following::

   :doc:`custom label <sample_doc>`

This will be rendered as

   :doc:`custom label <sample_doc>`

For cross-referencing specific section headings, figures, or other arbitrary places within a file, use the ``:ref:`` role.

The ``:ref:`` role requires a reference label that looks like this ``.. _type_ref-label:``.

.. code-block:: none

   .. _sec_my-ref-label:

   Section to cross-reference
   --------------------------

   Some content in this section.

The ``:ref:`` role for cross-referencing has the following syntax::

 :ref:`sec_my-ref-label`

Rendered as

 :ref:`sec_my-ref-label`


* Each reference label must be unique in the documentation.

* The label must begin with an underscore "_" for Sphinx to recognize it. But the reference to the label (i.e., ``:ref:`ref-label```)
  does not include the underscore.

* Use "sec\_" (section), "fig\_" (figure), "eq\_" (equation), "tab\_" (table),  at the beginning of each reference label to denote the type of reference.

* Separate the reference label from the text it is referencing with a newline.

* To reference figures, equations, or arbitrary places in a file, you must include a custom
  label in the reference for it to work::

    :ref:`custom label <eq_my-arbitrary-place-label>`

Rendered as

    :ref:`custom label <eq_my-arbitrary-place-label>`


Link to PyNEST API objects
~~~~~~~~~~~~~~~~~~~~~~~~~~

To link PyNEST API functions used in the documentation to the API reference page, use the following syntax::

   :py:func:`.Create`


Rendered as

   :py:func:`.Create`


You can link other Python objects such as classes, methods, and attributes.
For example, here is the class syntax ``:py:class:.ClassName`` and the method syntax ``:py:meth:.method``.

.. note::

   The object name is prefixed with a dot.
   This is required for Sphinx to find the PyNEST object, unless the object is defined in the same file you are including the link.


.. note::

   The methods ``get()`` and ``set()`` can be found in both the classes :py:class:`.NodeCollection` and
   :py:class:`.SynapseCollection`, and thus, you must explicitly state which class method you are referring to
   with the following syntax:

   * ``:py:meth:`.SynapseCollection.get``` rendered as :py:meth:`.SynapseCollection.get` or
   * ``:py:meth:`.NodeCollection.get``` rendered as :py:meth:`.NodeCollection.get`.

   To hide the class name in the link text, prefix the entire name with the tilde "~" in the following manner:

   * ``:py:meth:`~.NodeCollection.get``` rendered as :py:meth:`~.NodeCollection.get`.


Sometimes in the documentation you want to show a complete function call, as in ``nest.Create("iaf_psc_apha")``.
In these cases, the link cannot be used.


See `the Sphinx documentation on referencing Python objects
<https://www.sphinx-doc.org/en/master/usage/restructuredtext/domains.html#cross-referencing-python-objects>`_ for more
information.


reStructuredText text formatting
--------------------------------

Underlines for headings
~~~~~~~~~~~~~~~~~~~~~~~

reStructuredText uses several types of underline markers for headings. It is
important that the length of the underline is exactly as long as the words
in the heading.

In general, we try to follow the pattern:

* First heading: ``===``
* Second heading: ``---``
* Third heading: ``~~~``
* Fourth heading: ``^^^``

"Double quotes"
~~~~~~~~~~~~~~~

We use double quotes for strings in code, for example,  ``nest.Create("iaf_psc_alpha")``. This applies to
reStructuredText files as well as Python and C++ code. This rule is based on PEP 257, which (only) dictates the use of
double quotes in triple quoted strings; for consistency, double quotes are used throughout the codebase.

Double or single quotes should not be used to emphasize important concepts in the text.

.. _sec_dbltick:

\``Double backticks\``
~~~~~~~~~~~~~~~~~~~~~~

Use double backticks for all code and command related terms, such as function call examples, paths, variables, and parameters.
In addition, meta and special characters (such as the ampersand ``&``) should also be written in double backticks.

For example::

    ``nest.Create("iaf_psc_alpha")``

    ``/path/to/source/file.rst``

    "The key ``rule`` in the connectivity specification dictionary ``conn_spec`` . . . "


\**Strong emphasis\** vs \*emphasis\*
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

* Avoid using strong emphasis (boldface) in texts. If you want text to stand out use an appropriate :ref:`admonition <sec_admonition>`.

* Use the plain emphasis (italics) to signify context differences *within* a text.


NumPy style docstrings
----------------------

* In PyNEST code, we follow most of the rules for NumPy style docstrings as
  `explained in the NumPy style guide <https://numpydoc.readthedocs.io/en/latest/format.html>`_.

* However, we use different formatting marks than what is stated in their guide. See section on
  :ref:`double backticks <sec_dbltick>`.

* If you are contributing to the :doc:`PyNEST API <example_template>`, make sure you carefully read the NumPy guide, along
  with this one.


Bibliography style
------------------

The reStructuredText bibliography style is used throughout the documentation so that links
are autogenerated and a consistent format is used.

For in-text citations, we use the reStructuredText numeric style ``[1]_``.

Rendered as

    The following example is based on Smith [1]_.

    Sanders et al. [2]_ contains a technically detailed example.

Please ensure your reference follows the following guidelines.

*  Do not add formatting markup such as italics, bold, or underline.
*  Use a period after every section of bibliography.
*  Use et al. for references with more than five authors.
*  Put surname before first name for all authors.
*  Do not put commas after surname.
*  Use inital for first name of all authors.
*  Put year, in parentheses, after authors.
*  Write article titles in sentence case.
*  Write the full title of journal.
*  Insert a colon between volume and page-range.
*  Add issue in parentheses after volume (optional).
*  Include a linked DOI, if available.

Example of the reStructuredText syntax:

.. code-block:: none

 References
 -----------

 .. [1] Smith J. and Jones M (2009). Title of cool paper. Journal of
        Awesomeness. 3:7-29. <DOI>

 .. [2] Sander M., et al (2011). Biology of the sauropod dinosaurs: The
        evolution of gigantism. Biological Reviews. 86(1):117-155.
        https://doi.org/10.1111/j.1469-185X.2010.00137.x


Rendered as

.. [1] Smith J. and Jones M (2009). Title of cool paper. Journal of
       Awesomeness. 3:7-29. <DOI>

.. [2] Sander M., et al (2011). Biology of the sauropod dinosaurs: The
       evolution of gigantism. Biological Reviews. 86(1):117-155.
       https://doi.org/10.1111/j.1469-185X.2010.00137.x


Links to external resources
---------------------------

* `American English dictionary <https://www.merriam-webster.com/>`_

* `The Microsoft style guide <https://docs.microsoft.com/en-us/style-guide/welcome/>`_

* `reStructuredText docutils documentation <https://docutils.sourceforge.io/rst.html#id24>`_

* `reStructuredText Sphinx documentation <https://www.sphinx-doc.org/en/master/usage/restructuredtext/basics.html>`_

* `LaTeX math styling <http://www.ams.org/arc/styleguide/mit-2.pdf>`_

* `Sphinx documentation on referencing Python objects
  <https://www.sphinx-doc.org/en/master/usage/restructuredtext/domains.html#cross-referencing-python-objects>`_

* `NumPy style guide <https://numpydoc.readthedocs.io/en/latest/format.html>`_
