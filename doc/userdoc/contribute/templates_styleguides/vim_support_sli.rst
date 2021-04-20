:orphan:

Vim support for SLI files
=========================

A simple syntax file for Vim users is provided below. Copy it to your Vim configuration folder to make it available to Vim:


.. code-block::

   $ cp ${prefix}/share/nest/extras/EditorSupport/vim/syntax/sli.vim ~/.vim/syntax/sli.vim

Then add the following lines to your ``~/.vimrc`` file to use it:

.. code-block::

    " sli
    au BufRead,BufNewFile *.sli set filetype=sli
    au FileType sli setl foldenable foldmethod=syntax
