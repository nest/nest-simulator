VIM support for SLI files
=========================

A simple syntax file for VIM users is provided below. Copy it to your vim configuration folder to make it available to VIM:


.. code-block::

   $ cp ${prefix}/share/nest/extras/EditorSupport/vim/syntax/sli.vim ~/.vim/syntax/sli.vim

Then add the following lines to your ``~/.vimrc`` file to use it:

.. code-block::

    " sli
    au BufRead,BufNewFile *.sli set filetype=sli
    au FileType sli setl foldenable foldmethod=syntax