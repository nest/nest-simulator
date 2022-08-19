TEST examples
=============


1. Use ipynb as source file
   > Copied the generated notebooks from sphinx-gallery and put them into pynest/examples 
   !! Some notebook files are broken! 
   
2. store file in python/examples?
  >  made symlink copy for CMAKE in local build, but used a direct copy for RTD in conf.py 

3. Run at build a converter to python
   > using nbconvert
  ! Cannot glob all files (because of broken notebooks seemingly)
4. Add button to download python / ebrains button
   > ebrains button should indicate jupyter notebook (note button image still on nest-simulator)

* Remove sphinx gallery

TODO


* Fix up python empty cells (preprocessor?) and any other issues with python or jupyterfiles.
- Have code to remove emtpy In[] and empty coment lines in the python file

* Get proper link to  nbgitpuller
> not working >:(

* figure out how to generate / store links for puller

* See about not copying files into docs on RTD (how to symlink)

* Check subdirectories that they are included  etc (spatial)

* Convert all jupyter notebooks (is this what's taking time with just the one file?)
 > adjusted script to only act on notebooks rather than every file, now faster. may need further tweaking

* Replace auto_examples with links to notebooks


