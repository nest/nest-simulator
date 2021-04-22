The NEST documentation style guide
==================================

.. contents:: On this page, you'll find
   :local:

Why do we have a style guide?
-----------------------------

This style guide was created to provide a single reference point for content
creation in NEST. This helps ensure the NEST user-level documentation remains
clear and consistent. The style choices we make here are meant to follow the
major trends in technical writing for software.

Contribute to the docs
~~~~~~~~~~~~~~~~~~~~~~~

If you want to add or modify documentation, make sure to read through this guide before writing a contribution.

We have templates for :doc:`Python example scripts <pyapi_template>` and the :doc:`PyNEST API docstrings <example_template>`.

You will also need to know how to build the documentation locally on your machine: See
:doc:`../../documentation_workflow/user_documentation_workflow`.

For additional information, including the Git workflow and making a pull request see :doc:`../index`.


General guidelines about writing for NEST
-----------------------------------------

A few tips on writing documentation
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

- Be concise: Use short words and sentences; avoid unnecessary modifiers

- Be consistent: Follow this style guide and relevant templates

- Be specific: Avoid vague language; clarity is key

- Focus your message: Start with what's most important, and keep text relevant
  within given section

- Write for your reader: Try to understand what your reader needs to know;
  include context to what you're saying. Remember that most NEST users are computational neuroscientists,
  including students, not software developers!

- Provide examples: Show, don't tell; if you can use code snippets, screenshots,
  or figures to illustrate a point, do so!

- Avoid terms like "simply", "just", or "easy": your knowledge does not always equal
  your readers.  Don't make your reader feel bad because they "simply" didn't
  understand something.

- Write positively: Use positive language rather than negative language

Remember that the primary purpose of the documentation is to provide
information to NEST users who are looking for it.

Voice
~~~~~~

Voice is *how* we talk to people through writing. The voice encompasses
substance, style, and tone.

Our voice is

- empathetic: we write thinking of who we are writing for and what their needs
  are
- relaxed: we write in natural, conversational way rather than an authoritative
  way
- factual: we write in a neutral style avoiding irony, humor, or cultural references

Use "you" to indicate the reader and "we" to indicate yourself (the writer and
possibly the NEST team).

The language we use
~~~~~~~~~~~~~~~~~~~

We follow spelling and grammar rules of `American English <https://www.merriam-webster.com/>`_.

The markup language we use is :ref:`reStructuredText <sec-reST_style>`. This includes documentation
blocks in files written in Python and C++.

We avoid slang or jargon that is unnecessary or could be confusing to readers.

We do, however, use terminology appropriate to NEST and neuroscience when
specific and exact terms are needed, which is often. But explanations or
references should be provided for clarity in introductory texts and the glossary.

If you have further questions about style not addressed here, see <<STANDARDIZED STYLE GUIDE>>
or ask on the mailing list.

Active voice
~~~~~~~~~~~~

Prefer the active voice, where the subject acts on object.

If the object needs emphasis rather than subject, use passive voice. But ensure
that you cannot improve the sentence by using the active voice.

.. _sec-reST_style:

reStructuredText markup and formatting
--------------------------------------

reStructuredText is a plain text markup language and parser. It is the default language of the Sphinx documentation
generator, which NEST uses for generating documentation.

reStructuredText uses directives, which are blocks of explicit markup used for math, images, code, admonitions, and much
more. The syntax looks like this ``.. directive-name::``. The directive content follows after a blank line and is indented
relative to the directive start.

In addition to directives, reStructuredText has roles, which insert semantic markup into documents.
Roles look like this ``:role-name:`content```.

We will only cover a few examples here. You can find more information in the following links:


* `reStructuredText User Documentation <https://docutils.sourceforge.io/rst.html#id24>`_

* `reStructuredText Primer <https://www.sphinx-doc.org/en/master/usage/restructuredtext/basics.html>`_

* `Sphinx directives <https://www.sphinx-doc.org/en/master/usage/restructuredtext/directives.html>`_

* `Sphinx roles <https://www.sphinx-doc.org/en/master/usage/restructuredtext/roles.html>`_


.. note::

   reStructuredText is sensitive to indentation and new lines.

   * Directives, headings, labels, and tables should be separated from other text by a new line, excluding directive options.
   * Directive options must be indented to the same level as the directive content.
   * Text in multiline content should stay aligned with the first line.
   * If the output format seems incorrect, double check the spaces and newlines
     of the text.

NumPy style docstrings
~~~~~~~~~~~~~~~~~~~~~~

In PyNEST code, we follow the rules for NumPy style docstrings as
`explained here <https://numpydoc.readthedocs.io/en/latest/format.html>`_.

If you're contributing to the :doc:`PyNEST API <example_template>`, make sure you carefully read the NumPy guide.

Code and code snippets
~~~~~~~~~~~~~~~~~~~~~~

Code blocks are written using the code-block directive.

Example syntax::


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


For Python examples that show input and output use the following syntax::

   >>> input begins with 3, right-angled brackets
   Output is directly below input without any brackets.
   A blank line must end the example.

For in-text code use the role :code: or double back ticks::

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

If the equation runs over several lines you can use ``\\`` as a separator at the end of each line.
You can also align lines in an equation, using the ``&`` where you want equation aligned::


    .. math::

         (a + b)^2  &=  (a + b)(a + b) \\
                    &=  a^2 + 2ab + b^2

For in-text math, use the math role::

   Now we can see :math:`x=1` for this example.

These will be rendered as

   Now we can see :math:`x=1` for this example.

Admonitions
~~~~~~~~~~~

Admonitions are directives that render as highlighted blocks to draw the reader's attention to a particular point.

Use them sparingly.


Use the admonition

* "See also" to reference internal or external links (only in cases where the reference should stand out),

* "Note" to add additional information that the reader needs to be aware of,

* "Warning" to indicate that something might go wrong without the provided information, and

* "Danger" if the situation may cause severe, possibly irreversible, problems.


If you want a custom admonition use::

   .. admonition:: custom label

         Here is some text

Rendered as


   .. admonition:: custom label

         Here is some text


References
~~~~~~~~~~

For referencing reStructuredText files within the documentation, use the ``:doc:`` role. It requires the relative path to
the file::

   :doc:`path/to/file`

In this case, the link caption will be the title of the given document.

You can specify the text you want to use for the link by doing the following::

   :doc:`custom label <path/file>`

This will be rendered as

   :doc:`Top header of file`

   :doc:`custom label <file>`

For cross-referencing specific section headings, figures, or other arbitrary places within file, use the ``:ref:`` role.

The ``:ref:`` role requires a reference label that looks like this ``.. _ref-label:``. Each reference label must be unique
in the documentation. Separate the reference label from the text it is referecing with a new line::

   .. _my-ref-label:

   Section to cross-reference
   --------------------------

   Some content in this section.
   It includes the cross-referecing role :ref:`my-ref-label`.


To reference figures or arbitrary places in a file, you must include a custom
label in the reference for it to work::

    :ref:`custom label <my-arbitrary-place-label>`



Special links
~~~~~~~~~~~~~

.. attention::

  The items in this section are still in development and have not been incorporated into nest:master!

Links to functions
^^^^^^^^^^^^^^^^^^

To link PyNEST API functions used in-text to the API reference page use the following syntax::

   :py:func:`.Create`

Rendered as

   :py:func:`.Create`

Classes, methods etc. can also be linked: ``:py:class:`` ``:py:meth:``.
If you want to explictly show a complete function call, like ``nest.Create("iaf_psc_apha")``, the link cannot be used.

.. note::

   Functions within classes NodeCollection and SynapseCollection require different syntax as follows

   ``:py:func:`~nest.lib.hl_api_types.SynapseCollection.funcname```

   ``:py:func:`~nest.lib.hl_api_types.NodeCollection.funcname```


Links to glossary
^^^^^^^^^^^^^^^^^

To link terms to the glossary page use the HoverXTooltip role :hxt_ref: from Mahdi Enan (INM-6)::

  :hxt_ref:`E_L`

Links to certain external projects
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

With the Sphinx extension 'intersphinx', projects that also use Sphinx/ReadtheDocs can be referenced the same way as
your local project. You can use the reference label role (``:ref:``), document role (``:doc:``), and Python role
(``:py:func:``, ``:py:class:``). You only need to add the intersphinx unique identifer to the reference, which
looks like this ``:doc:`custom label <unique-identifier:filename>```. See section in userdoc/conf.py "intersphinx_mapping" to
see which projects are currently included along with their unique identifier.

Examples of syntax::

  :doc:`tutorial for nestml <nestml:tutorials>`
  :py:func:`pyNN.utility.get_simulator`


.. note::

   Depending on how a project is documented, you may only be able to use the ``:doc:`` role or the ``:ref:`` role.
   To find out, you need to look into the objects.inv file, which can be obtained with the following code

   ``python -msphinx.ext.intersphinx https://docs.project.org/objects.inv``

   Objects in objects.inv are categorized into different sections.
   The std:label refers to objects that use the ``:ref:`` role. And std:doc refers to objects that use the ``:doc:`` role.

Bibliography style
------------------

The reStructuredText bibliography style is used throughout documentation so links
are autogenerated and a consistent format is used.

For in-text citations, we use the reStructuredText numeric style ``[1]_``.

For example:

    The following example is based on Smith [1]_.
    Sanders et al. [2]_ contains a technically detailed example.

Please ensure your reference follows the following guidelines:

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

.. code-block:: none

 References
 -----------

 .. [1] Smith J. and Jones M (2009). Title of cool paper. Journal of
        Awesomeness. 3:7-29. <DOI>

 .. [2] Sander M., et al (2011). Biology of the sauropod dinosaurs: the
        evolution of gigantism. Biological Reviews. 86(1):117-155.
        https://doi.org/10.1111/j.1469-185X.2010.00137.x


Underlines for headings
~~~~~~~~~~~~~~~~~~~~~~~

ReStructuredText uses several types of underline markers for headings. It's
important that the length of the underline is exactly as long as the words
in the heading.

In general, we try to follow the pattern of

* First heading: ===
* Second heading: ---
* Third heading: ~~~
* Fourth heading: ^^^

"Double quotes"
~~~~~~~~~~~~~~~

We use double quotes for strings in code, for example,  ``nest.Create("iaf_psc_alpha")``. This applies to
reStructuredText files as well as Python and C++ code. This rule is based on PEP 257, which (only) dictates the use of
double quotes in triple quoted strings; for consistency, double quotes are used throughout the codebase.

Double or single quotes should not be used to emphasize important concepts in the text.


\``Double backticks\``
~~~~~~~~~~~~~~~~~~~~~~~

Use double backticks for


  - inline code
  - objects/functions
  - model names
  - NEST-specific vocabulary
  - function calls (e.g., ``nest.Create("iaf_psc_alpha")`` or  (``Create`` )
  - Paths (e.g, You can find the models in ``nest-simulator/pynest/examples``)
  - Key value pairs (``{key: value}``)
  - Variables with assigned values ``x = 10``


\`Single backticks\`
~~~~~~~~~~~~~~~~~~~~

Use single backticks for

- Dictionary keys (if no value is provided)
- Parameters
- Variable names
- Values

but use double backticks when showing a complete example of variable with
assigned value (e.g., \``volt = 37.0``)

An example::

   Here we use the ``nest.Create()`` function to instantiate our model, in this case
   ``iaf_psc_alpha``. We can modify the parameter `V_m` and set the value to
   `50.0`.

Rendered as


   Here we use the ``Create`` function to instantiate our model, in this case
   ``iaf_psc_alpha``. We can modify the parameters `V_m` and set the value to
   `50.0`.

\**Strong emphasis\**
~~~~~~~~~~~~~~~~~~~~~

If you want to emphasize a word or phrase in text, you can use **strong emphasis**.

Boldface should only be used in exceptional cases when overlooking the emphasized text could cause problems, but
the text in question is too short to warrant an admonition box.

How we write
------------

Headings and subheadings
~~~~~~~~~~~~~~~~~~~~~~~~

Headings and subheadings describe the purpose of the section.

Begin with a descriptive verb or begin with `How to ...`

Headings explain the section in a short phrase.

Use the verb stem and not the gerund ('ing') form of verbs. Not 'Adding a
model', but  'Add a model'.

Avoid section names like `Introduction` or `Part 1`.

One-word subheadings are acceptable, if the section is short and the meaning is clear.

Use sentence case for headings and subheadings, i.e., begin with an uppercase
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


Numbers
~~~~~~~

Numbers 0-9 should be spelled out, unless they are measurements or coordinates.

Numbers should be spelled out if they begin a sentence. In most cases, however,
the numeral/ordinal format is preferred.

For additional mathematical notation, use the :ref:`math role or directive <math_style>`.

We use the period for the decimal point. (`57.45`)

The thousand separator is the comma except when showing a code example

   Example:

   We have over 5,000 connections.
   The number of connections is ``x = 5001``

Make sure you use the correct unit (e.g., millivolts for voltage) and the
unit's symbol (`mV`).

Lists
~~~~~

Use the serial comma in lists.

Use numbered lists for step-by-step instructions only. Do not have more that two
related actions in one step.

Use bullet lists if the number of items is extensive or each item is a long phrase or sentence.

If the text of a list forms a complete sentence, use proper punctuation and
end with period.

If the text of a list forms an incomplete sentence, do not end with period.

If the entire bullet/numbered list belongs to a sentence, end each item with a comma and the second-last item with ", and".
If last item is the end of the sentence, end it with a period. Otherwise use the punctuation required to correctly connect
to the remainder of the sentence.

Pronouns
~~~~~~~~

Use the pronouns "you" (reader) and "we" (NEST) whenever possible.

Avoid the pronoun "I".

Avoid gendered terms (use "police officer" instead of "policeman").

Instead of "guys" or "girls" use inclusive language such as everyone, all,
members, or folks.

"They" is an acceptable singular third person pronoun
(see `here <www.merriam-webster.com/dictionary/they>`_).

Abbreviations and acronyms
~~~~~~~~~~~~~~~~~~~~~~~~~~

Spell out acronyms on first appearance on each page or article it appears.
For example: Random number generator (RNG)

If the abbreviation/acronym is well known (e.g., HTML) you do not need to spell
it out.


Commas
~~~~~~

Use the serial comma (apples, bananas, and grapes) for lists.

Use the comma as separator for thousands (37,000).

To join two sentences into one, you must use a conjunction (and, or , but)
along with the comma, or use the semicolon.

Sentences and paragraphs
~~~~~~~~~~~~~~~~~~~~~~~~

Avoid using `So` and `However` at the beginning of sentences.
Try to keep sentences short, or break up long sentences with short ones.

Avoid lengthy paragraphs with more than 5 or 6 sentences.
If writing multiple paragraphs, they should be broken up by example code, figures, or bullet lists.
Keep in mind that texts should be skimmable.

Ampersand
~~~~~~~~~

Avoid the ampersand '`&`' and use '`and`' instead unless the ampersand is part
of a proper name (e.g., Ben \& Jerry's).


Capitalization
~~~~~~~~~~~~~~

Capitalize first word of heading, but use lower case for the rest.

Capitalize first word in bullet/numbered list.

Capitalize proper nouns and follow company policy in naming conventions
(e.g., macOS, LaTeX, Python, NumPy, NEST).
