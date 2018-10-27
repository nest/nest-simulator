
  This workflow builds the *NEST Topology User Manual*.

  You need to have [Snakemake] and [pandoc] installed.

  The process runs from the `.tex` file to all different output formats automatically by just running 

    snakemake

  In case a conversion can not be fixed in another way than changing the
  intermediate `Topology_UserManual.md`, be sure to recreate the patch file
  and add it to the repository:

    snakemake -f patch
    git add *patch


  [Snakemake]: https://snakemake.readthedocs.io/en/stable/index.html
  [pandoc]: https://pandoc.org/

