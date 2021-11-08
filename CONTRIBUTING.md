# Contributing to NEST

The NEST simulator is a scientific tool and as such it is never finished and constantly changing to meet the needs of novel neuroscientific endeavors. Here you find the most important information on how you can contribute to NEST. This document is an excerpt from our [Contributing to NEST documentation](https://nest-simulator.readthedocs.io/en/latest/contribute/index.html), which provides more detailed information.

## Getting started

* Make sure you have a [GitHub account](https://github.com/signup/free)
* The development workflow is based purely on pull requests. This [article](https://nest-simulator.readthedocs.io/en/latest/contribute/development_workflow.html) gives information on how to work with git and GitHub if you are new to it.

## Reporting bugs and issues

If you come across a bug in NEST, please [file an issue on the GitHub issue tracker](https://github.com/nest/nest-simulator/issues/new). Please use the template below to provide information:

```
Description of problem:


Version-Release or Git commit:


How reproducible: (Always/Sometimes/Unsure)


Steps to Reproduce:
1.
2.
3.

Actual results:


Expected results:


Additional info:
```

You can also attach files to issues. Data dumps or example code snippets that help to reproduce the issue is most welcome. This information enables developers to debug and fix issues quicker.

## Making Changes

* Create a topic branch from NEST master in your fork. Please avoid working directly on the `master` branch. See [issue 31](https://github.com/nest/nest-simulator/issues/31) for more information.
* Make commits of logical units.
* Make sure NEST compiles and has no new warnings.
* Make sure all tests pass (`make installcheck`).
* Make sure your code conforms to our [coding guidelines](https://nest-simulator.readthedocs.io/en/latest/contribute/coding_guidelines_cpp.html) (`./extras/check_code_style.sh`)

## Code Review

We review each pull request according to our [code review guidelines](https://nest-simulator.readthedocs.io/en/latest/contribute/code_review_guidelines.html):

* In general, the rule is that each pull request needs an OK from the CI platform and at least two reviewers to be merged.
* For changes labeled "not code" or "minor" (e.g. changes in documentation, fixes for typos, etc.), the release manager can waive the need for code review and just accept the OK from Travis in order to merge the request.
* New features like SLI or PyNEST functions, neuron or synapse models need to be accompanied by one or more tests written either in SLI or Python. New features for the NEST kernel need a test written in SLI.
* Each change to the code has to be reflected also in the corresponding examples and documentation.
* All source code has to conform to the Coding Guidelines for [C++](https://nest-simulator.readthedocs.io/en/latest/contribute/coding_guidelines_cpp.html) and [PEP8](https://www.python.org/dev/peps/pep-0008/) for Python in order to pass the continuous integration system checks.
* All commits should be coherent and contain only changes that belong together.

## Submitting Changes

* Sign the [Contributor License Agreement](https://raw.githubusercontent.com/nest/nest-simulator/master/doc/userdoc/contribute/NEST_Contributor_Agreement.pdf).
* Push your changes to a topic branch in your fork of the repository.
* Submit a pull request to the [NEST repository](https://github.com/nest/nest-simulator).
* If your pull request affects documented issues, [mention](https://github.com/blog/957-introducing-issue-mentions) them in the description. If it is solving an issue, you can [state this explicitly](https://help.github.com/articles/closing-issues-via-commit-messages/).
* The core team looks at pull requests on a regular basis and posts feedback.
* After feedback has been given we expect responses within two weeks. After two weeks we may close the pull request if it isn't showing any activity.

# Additional Resources

* Documentation on [Contributing to NEST](https://nest-simulator.readthedocs.io/en/latest/contribute/index.html).
* The [NEST Simulator homepage](https://www.nest-simulator.org/).
* The [NEST Initiative homepage](https://www.nest-initiative.org/).
