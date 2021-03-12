The NEST documentation style guide
==================================

.. contents::
  :local:

Why do we have a styleguide?
-----------------------------

This style guide was created to provide a single reference point for content creation in NEST. This helps ensure
the NEST user-level documentation remains clear and consistent. The style choices we make here are meant to follow the
major trends in technical writing for software.

General guidelines about writing for NEST
-----------------------------------------

How to *write good* : a few tips
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

- Be concise - Use short words and sentences; avoid unnecessary modifiers

- Be consistent - Follow this styleguide and relevant templates

- Be specific - Avoid vague language; clarity is key

- Focus your message - Start with what's most important, and keep text relevant within given section

- Write for your reader - Try to understand what your reader needs to know; include context to what you're saying

- Provide examples - Show don't tell; if you can use code snippets, screenshots, or figures to illustrate a point, do so!

- Avoid terms like simply, just, or easy - your knowledge does not always equal your readers.  Don't make your reader feel bad because they -simply-
  didn't understand something.

- Write positively -  Use positive language rather than negative language

Remember that the primary purpose of the documentation is to provide
information to NEST users who are looking for it.

Voice
~~~~~~

Voice is *how* we talk to people through writing. The voice encompasses substance, style, and tone.

Our voice is

- empathetic - we write thinking of who we are writing for and what their needs are
- relaxed - we write in natural, conversational way rather than an authoritative way

Use 'you' to indicate the reader and 'we' to indicate yourself (the writer and possibly the NEST team).

The language we use
~~~~~~~~~~~~~~~~~~~

We follow spelling and grammar rules of American English.

The markup language we use is reStructuredText. This includes documentation blocks in files written in Python and cpp.

We write in plain (global) English, and avoid slang or jargon that is unnecessary.

We do, however, use terminology appropriate to NEST and neuroscience when specific and exact terms are needed, which is often. But explanations or references
should be provided for clarity in introductory texts.

If you have further questions about style not addressed here, see ?Microsoft style guide?

Active voice
~~~~~~~~~~~~

Prefer the active voice, where the subject acts on object.

If the object needs emphasis rather than subject, use passive voice. But ensure that you cannot improve the sentence
by using the active voice.


Types of formatting to use in writing
--------------------------------------

We use specific formatting marks to help readers scan through and find what they need, as well
as provide references to important terms.


Underlines for headings
~~~~~~~~~~~~~~~~~~~~~~~

ReStructuredtext uses several types of underline markers for headings. It's important that the length of the
underline is at least as long as the words in the heading.

In general, we try to follow the pattern of

* First heading: ===
* Second heading: ---
* Third heading: ~~~
* Fourth heading: ^^^

"Double quotes"
~~~~~~~~~~~~~~~

We use double quotes for strings in code, for example (nest.Create("iaf_psc_alpha")).

Double or single quotes should not be used as an emphasis marker for text.


\``Double back tick\``
~~~~~~~~~~~~~~~~~~~~~~~

Use the double back tick for:


  - objects/functions
  - model names
  - important vocabulary
  - function call (e.g., ``nest.Create("iaf_psc_alpha")`` or  (``Create`` )
  - Paths (e.g, You can find the models in ``nest-simulator/pynest/examples``)
  - Key value pairs (``{key: value}``)
  - Variables with assigned values ``x = 10``


\`Single back tick\`
~~~~~~~~~~~~~~~~~~~~~

Use single back ticks for

- Keys
- Parameters
- Variable names
- Values

but use double back ticks when showing a complete example of variable with assigned value (e.g., \``volt = 37.0``

An example:

::

   Here we use the ``Create`` function to instantiate our model, in this case ``iaf_psc_alpha``. We can modify
   the parameter `V_m` and set the value to `50.0`.

Rendered as


 Here we use the ``Create`` function to instantiate our model, in this case ``iaf_psc_alpha``. We can modify
 the parameters `V_m` and set the value to `50.0`.

\**Boldface\**
~~~~~~~~~~~~~~~

If you want to emphasize a word or phrase in text, you can use **boldface**.

NumPy style docstrings
~~~~~~~~~~~~~~~~~~~~~~

In the PyNEST code, we follow the rules for NumPy style docstrings as
`explained here <https://numpydoc.readthedocs.io/en/latest/format.html>`


reStructuredText markup
-----------------------

Let us take a brief look into the markup of reStructuredText. You can find the full CheatSheet
`here <https://thomas-cokelaer.info/tutorials/sphinx/rest_syntax.html>`_.

.. note::

   reStructuredText is sensitive to indentation. A single newline should always be between any text and headings,
   tables, reference labels, or directives.
   If the output format seems incorrect, double check the spaces and newlines of the text.

Code and code snippets
~~~~~~~~~~~~~~~~~~~~~~

For writing Python code, you can use the double colon


 \::

Rendered as

   ::

       python code here

       x = 1

       for i in line:
           x = i
           print(thing.x)

For other code styles use the code-block directive (you can also use this one for python)



\   .. code-block:: cpp

Rendered as

    .. code-block:: cpp

       int main()
       {
         cout << "Hello, World!";
         return 0;
       }


For input/output examples use the following syntax

::

   >>> input begins with 3, right-angled brackets
   output is directly below input without any brackets


Directives
~~~~~~~~~~~

This is not a complete guide to the directives of reStructuredText. For more options see `Sphinx directives <https://www.sphinx-doc.org/en/master/usage/restructuredtext/directives.html>`_
or `Docutils section on directives <http://docutils.sourceforge.net/docs/ref/rst/directives.html>`_.

Equations
^^^^^^^^^

For equations, use LaTeX markup:

::

    .. math::

            f(x) = \int_{-\infty}^{\infty} \hat f(\xi)\ e^{2 \pi i x \xi}\,d\xi,

Rendered as


    .. math::

            f(x) = \int_{-\infty}^{\infty} \hat f(\xi)\ e^{2 \pi i x \xi}\,d\xi,


Admonitions
^^^^^^^^^^^

Use admonitions to draw readers attention to a particular point. Possible admonitions types include

"see also", "attention", "caution", "danger", "error", "hint", "important", "note", "tip", "warning", "admonition"

If you want a custom admonition use

::

   .. admonition:: custom name

         Here is some text

Rendered as


   .. admonition:: custom name

         Here is some text


Roles
~~~~~~

See `the roles section in the Sphinx docs <https://www.sphinx-doc.org/en/master/usage/restructuredtext/roles.html>`_ for details

In-text math
^^^^^^^^^^^^

For in-text math, use

::

   Now we can see :math:`x=1` for this example.

This will be rendered as


   Now we can see :math:`x=1` for this example.

Referencing
^^^^^^^^^^^

For referencing rst files in repository, use

::

   :doc:`file`

::

   :doc:`custom name <path/file>`

This will be rendered as

   :doc:`file`

   :doc:`custom name <file>`


For section headings, you can use either of the two examples below. But to reference figures or arbitrary places in a file,
you must include a custom name (second example) in the reference for it to work.

::

    :ref:`ref_name`

or

::

    :ref:`custom name <ref_name>`

Rendered as


    :ref:`ref_name`

    :ref:`custom name <ref_name>`


.. note::
    The `ref_name` needs to be above the section you want to reference with the following syntax

    ::

        .. _ref_name:

        secton_header
        --------------


Bibliography style for NEST
----------------------------

The reStructuredText reference style is used throughout documentation so links are autogenerated and a consistent format is used.

For in-text citations, we use the reST numeric style ``[1]_``.


For example:

.. code-block:: none

    The following example is based on Smith [1]_.
    [2]_ contains a technically detailed description.

Please ensure your reference follows the following guidelines:

* References with more than 5 authors use 'et al.'.
* Use initials for first name of authors
* Surname precedes first name for all authors
* No comma follows surname
* Full stop after every section of bibliography.
* No formatting such as italics, bold or underline.
* Full title of journal
* Article titles written in sentence case
* Year follows author(s,), in parentheses
* Volume, can be optionally followed by issue in parentheses,  a colon separates volume and page range.
* Include a linked DOI, if available

.. code-block:: none

 References
 -----------

 .. [1] Smith J. and Jones M (2009). Title of cool paper. Journal of Awesomeness.
       3:7-29. <DOI>

 .. [2] Sander M., et al (2011). Biology of the sauropod dinosaurs: the evolution
        of gigantism. Biological Reviews. 86(1):117-155. https://doi.org/10.1111/j.1469-185X.2010.00137.x

How we write . . .
---------------------

Headings and subheadings
~~~~~~~~~~~~~~~~~~~~~~~~~

Headings and subheadings should describe the purpose of the section.

Begin with a descriptive verb or begin with `How to ...`

Heading should explain the section in a short phrase.

Use the verb stem and not the gerund ('ing') form of verbs. Not 'Adding a model', but  'Add a model'.

Avoid section names like `Introduction` or `Part 1`.

One word subheadings are fine, if section is short and the meaning is clear.

Headings and subheadings should always begin with an uppercase letter but all other words
should be lower case (except proper nouns).




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
~~~~~~~~

Numbers should be spelled out if they begin a sentence. In most cases, however, the numeral/ordinal format is preferred.

For additional mathematical notation, use the math role or directive.

We use the period for the decimal point. (`57.45`)

The thousand seperator is the comma except when showing a code example

   Example:

   We have over 5,000 connections.
   The number of connections is ``x = 5000``

Make sure you use the correct unit (e.g., millivolts for voltage) and the unit's syntax (`V_m`).

Lists
~~~~~~~

Use ordered lists for step-by-step instructions only. Do not have more that 2 related actions in one step.

Use bullet lists to improve clarity of long lists (more than 5 items).

If bullet/ordered list text is a complete sentence, use proper punctuation and end with period.

If bullet/ordered list text is an incomplete sentence, do not end with period.

If bullet/ordered list belongs to a sentence, use commas to seprate each item with the second last time including `and` at end.

Pronouns
~~~~~~~~

Use the pronouns "you" and "we" whenever possible.

Avoid the pronoun "I".

Avoid gendered terms (NOT police man BUT police officer).

Instead of "guys" or "girls" use inclusive language such as everyone, all, members, folks.

"They" is an acceptable singular third person pronoun (See `here <www.merriam-webster.com/dictionary/they>`_).

Abbreviations and acronyms
~~~~~~~~~~~~~~~~~~~~~~~~~~

Spell out acronyms on first appearance on each page or article it appears.
For example: Random number generator (rng)

If the abbreviation/acronym is well known (e.g., HTML) you do not need to spell it out.


Commas
~~~~~~~

Use the oxford comma (apples, bananas, and grapes) for lists. But use a bullet list if your list is more than 5 items.

Use the comma as separator for thousands (37,000).

To join two sentences into one, you must use a conjuction (and, or , but) along with the comma, or use the semicolon.

Conjunctions (For, And, Nor, But, Or, Yet, So)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

You can use these to start a sentence if clarity is not impeded.

Avoid using `So`, `However`, at the beginning of sentences.

Ampersand
~~~~~~~~~~~~

Avoid the ampersand '`&`' and use '`and`' instead unless the ampersand is part of a proper name (e.g. Ben \& Jerry's).


Capitalization
~~~~~~~~~~~~~~~

Capitalize first word of heading, but use lower case for the rest.

Capitalize first word in bullet.

Capitalize proper nouns and follow company policy in naming conventions (e.g., macOS, LaTeX).