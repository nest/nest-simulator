NEST Style Guide
==================

.. contents::
  :local:

General guidelines about writing for NEST
---------------------------------------------

How to *write good* : a few tips
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

- Be concise - Use short words and sentences; avoid unnecessary modifiers

- Be consistent - Follow this style guide and relevant templates

- Be specific - Avoid vague language; clarity is key

- Focus your message - Start with what's most important, and keep text relevant within given section

- Write for your reader - Try to understand what your reader needs to know; include context to what you're saying

- Provide examples - Show don't tell; if you can use code snippets, screenshots, or figures to illustrate a point, do so!

- Avoid terms like simply, just, or easy - your knowledge does not always equal your readers.  Don't make your reader feel bad because they -simply-
  didn't understand something.

- Write positively -  Use positive language rather than negative language

Voice
~~~~~~

Voice is *how* we talk to people through writing. The voice encompasses substance, style, and tone.

Our voice is

- clear - we write for clarity and simplicity,
- empathetic - we write thinking of who we are writing for and what their needs are
- relaxed - we write in natural, informal way rather than an authoritative way

Use 'you' to indicate the reader and 'we' to indicate yourself (the writer and possibly the NEST team).

The language we use
~~~~~~~~~~~~~~~~~~~~~~

We follow spelling and grammar rules of American English, using Merriam Webster as our reference.

For additional style questions or concepts, follow the Microsoft Writing Styling Guide.

The markup language we use is reStructuredText. This includes documentation blocks in files written in Python and cpp.

We write in plain (global) English, and avoid slang or jargon that is unnecessary.

We do, however, use terminology appropriate to NEST and neuroscience when specific and exact terms are needed. But explanations or references
are provided for clarity.


Headings and subheadings
~~~~~~~~~~~~~~~~~~~~~~~~~

Headings and subheadings should describe the purpose of the section.

Begin with a descriptive verb (avoid vague verbs like get or use) or begin with `How to ...`

Heading should explain the section in a short phrase.

Use the imperative not the gerund ('ing') form of verbs.

Avoid section names like `Introduction` or `Part 1`.

One word subheadings are fine if section is short and the meaning is clear.

Headings and subheadings should always begin with a capital but all other words
should be lower case (except proper nouns).


::

 Good examples: Create your first neural network
                Set up and configure MUSIC to work with NEST
                Add a device to your network

 Bad examples:  Start here.
                MUSIC and NEST
                Device setup


Numbers
~~~~~~~~~

Numbers should be spelled out if they begin a sentence. In most cases, however, the numeral/ordinal is preferred.

For additinal mathematical notation, use the math role or directive (see below).

We use the period for the decimal point. (`57.45`)

The thousand seperator is the comma except when showing a code example

Example:

   We have over 5,000 connections.

   The number of connections is ``x = 5000``

Make sure you use the correct unit (e.g., millivolts for voltage) and the unit's syntax (`V_m`)  (see glossary).

Lists
~~~~~~~

User ordered lists for step-by-step instructions only. Do not have more that 2 related actions in one step.

Use bullet lists to improve clarity of long lists (more than 5 items).

If bullet/ordered list text is a complete sentence, use proper punctuation and end with period.

If bullet/ordere list text is an incomplete sentence, do not end with period.

Active voice
~~~~~~~~~~~~~~

Prefer the active voice, where the subject acts on object.

If the object needs emphasis rather than subject, use passive voice. But ensure that you cannot reorganize sentence
to use active voice.

Mix it up! Use both styles.

Writing people
~~~~~~~~~~~~~~~

Avoid gendered terms (NOT police man BUT police officer).

Instead of "guys" or "girls" use inclusive language such as everyone, all, members, folks.

"They" is an acceptable singular 3rd person pronoun (See www.merriam-webster.com/dictionary/they)

Abbreviation and acronyms
~~~~~~~~~~~~~~~~~~~~~~~~~~

Spell out acronyms on first appearance on each page or article it appears
For example: Random number generator (rng)

If the abbreviation/acronym is well known (e.g., HTML) you do not need to spell it out


Commas
~~~~~~~

Use the oxford comma (apples, bananas, and grapes) for lists. But use a bullet list if your list is more than 5 items.

Use the comma as separator for thousands (37,000).

To join two sentences into one, you must use a conjuction (and, or , but) along with the comma. Or use the semicolon.

Conjunctions (For, And, Nor, But, Or, Yet, So)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

You can use these to start a sentence if clarity is not impeded.
(There is **no rule** against doing this)

Ampersand
~~~~~~~~~~~~

Avoid the ampersand '`&`' and use '`and`' instead unless the ampersand is part of a proper name (e.g. Ben \& Jerry's)


Capitalization
~~~~~~~~~~~~~~~

Capitalize first word of heading, but use lower case for the rest.

Capitalize first word in bullet.

Capitalize proper nouns and follow company policy in naming conventions (e.g., macOS, LaTeX).



Types of formatting to use in writing
--------------------------------------

We use specific formatting marks to help readers scan through and find what they need, as well
as provide references to important terms.


Monospace \``double back tick\``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Use the double back tick for:

- Any term that should become a link to a another page
- Any term that you want to demonstrate the correct syntax

- Terms to be linked:

  - objects/functions
  - model names
  - important vocabulary


.. note::

   A script will search through the files to replace the above terms with the correct syntax that will provide a link to the appropriate
   source.

- Terms to demonstrate syntax

  - **complete** function call (e.g., ``nest.Create("iaf_psc_alpha")`` NOT (``("iaf_psc_alpha")`` )
  - Paths (e.g, You can find the models in ``nest-simulator/pynest/examples``)
  - Key value pairs (``{key: value}``)
  - Variables with assigned values ``x = 10``

\`Single back tick\`
~~~~~~~~~~~~~~~~~~~~~

- Keys
- Parameters
- Variable names
- Values

but use double back ticks when showing a complete example of variable with assigned value (e.g., \``volt = 37.0``

\**Boldface\**
~~~~~~~~~~~~~~~

If you want to emphasize a word or phrase in text, you can use **boldface**.

Do not use underline to emphasize words.

An example:

::

   Here we use the ``Create`` function to instantiate our model, in this case ``iaf_psc_alpha``. We can modify
   the parameter `V_m` and set the value to `50.0`.

Rendered as


 Here we use the ``Create`` function to instantiate our model, in this case ``iaf_psc_alpha``. We can modify
 the parameters `V_m` and set the value to `50.0`.


A brief look into reStructuredText markup
--------------------------------------------------------

.. note::

   reStructuredText is sensitive to indentation. A single space should be always be between between text and headings,
   tables, reference labels, and directives  Some characters have assigned special meanings. Use the '\' to 'escape' and use the literal
   character.

How to write code or code snippets in reStructuredText
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


For writing Python code, you can use the double colon  `::`


 \::

    python code here

    x = 1

    for i in line:
        x = i
        print(thing.x)

Rendered as

   ::

       python code here

       x = 1

       for i in line:
           x = i
           print(thing.x)

For other code styles use the code-block directive (you can also use this one for python)



\   .. code-block:: cpp


       int main()
       {
         cout << "Hello, World!";
         return 0;
       }

Rendered as

    .. code-block:: cpp

       int main()
       {
         cout << "Hello, World!";
         return 0;
       }


For input/output examples use the following

::

   >>> input begins with 3, right-angled brackets
   output is directly below input without any brackets


Directives
~~~~~~~~~~~


This is not a complete reference to the directives of reStructuredText. For more options see `Sphinx directives <https://www.sphinx-doc.org/en/master/usage/restructuredtext/directives.html>`_
or `Docutils section on directives <http://docutils.sourceforge.net/docs/ref/rst/directives.html>`_.

Include images in text
^^^^^^^^^^^^^^^^^^^^^^^

Include images with the image or, preferably, the figure directive. The figure directive allows captions.

::

    .. image:: triceratops.png
         :height: 200px
         :width: 200 px
         :alt: alternate text
         :align: center

    .. figure:: triceratops.png
         :scale: 30 %
         :align: center

         Here is the descriptive caption. This is a triceratops, a member of the Marginocephalia clade.

         Figures take the same options as images. Note the indentation is the same for the options and text. This text
         also belongs with the figure, but is not considered part of the caption.

Image rendered as

    .. image:: triceratops.png
         :height: 200px
         :width: 200 px
         :alt: alternate text
         :align: center
----

Figure rendered as

.. figure:: triceratops.png
      :scale: 30 %
      :align: center

      Here is the descriptive caption. This is a triceratops, a member of the Marginocephalia clade.

      Figures take the same options as images. Note the indentation is the same for the options and text. This text
      also belongs with the figure, but is not considered part of the caption.

For equations, use LaTeX markup:
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

::

    .. math::

            f(x) = \int_{-\infty}^{\infty} \hat f(\xi)\ e^{2 \pi i x \xi}\,d\xi,

Rendered as


    .. math::

            f(x) = \int_{-\infty}^{\infty} \hat f(\xi)\ e^{2 \pi i x \xi}\,d\xi,


Use admonitions to draw readers attention to a particular point
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Possible admonitions types include

"attention", "caution", "danger", "error", "hint", "important", "note", "tip", "warning", "admonition"

If you want a custom admonition use

::

   .. admonition:: custom name

         Here is some text

Rendered as


   .. admonition:: custom name

         Here is some text

You can also use the see also note

::

    .. seealso::

       The style can be modifed using custom CSS

Rendered as

    .. seealso::

       The style can be modifed using custom CSS


Roles
~~~~~~~

See `the roles section in the Sphinx docs <https://www.sphinx-doc.org/en/master/usage/restructuredtext/roles.html>`_ for details

For in-text math
^^^^^^^^^^^^^^^^^^^^^

::

   Now we can see :math:`x=1` for this example.

Rendered as


   Now we can see :math:`x=1` for this example.

For referencing rst files in repository
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

::

   :doc:`file`

::

   :doc:`custom name <path/file>`

Rendered as

   :doc:`file`

   :doc:`custom name <file>`


For referencing rst files, sections, or figures
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

You can also reference arbitrary places in a file, but you must insert a custom name (second option) to create
a link.

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

For terms in glossary
^^^^^^^^^^^^^^^^^^^^^^

::

   :term:`word`

Rendered as

   :term:`word`

.. note::

   Glossaries are built with the glossary directive
   ::

    .. glossary::

       word
        here is definition

For models to link to cpp model info

::

   :cpp:class:`Model_name <nest::Model_name>`

Rendered as

   :cpp:class:`Model_name <nest::Model_name>`

For functions to link to api

::

   :py:func:`.Connect`


Rendered as

   :py:func:`.Connect`


Reference style for NEST
--------------------------

The reST reference style is used throughout documentation so links are autogenerated and a consistent format is used.

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


