.. _code_guidelines:

Code review guidelines
======================

Using our purely :ref:`pull-request-based workflow <git_workflow>`,
code can be developed very flexibly by internal and external
contributors. At the same time this workflow allows us to enforce
stricter rules on code that ends up in the public repository.

Following are rough guidelines for the code reviewers. These are not
meant to prevent progress, but to keep up the code quality as the
number of developers is growing. All of this is not set in stone and
can be discussed on the :ref:`NEST mailing lists <contact_us>`.

* In most cases, each pull request needs an OK from the
  CI platform and at least two reviewers to be merged.

* The two reviews have to cover the technical side (i.e., if the code
  does the right thing and is architecturally sound) and the content
  side (i.e., if the code is scientifically correct and fixes an
  actual issue).

* For changes labeled "not code" or "minor" (e.g., changes in
  documentation, fixes for typos, etc.), the need for a second code
  review can be waived, and a single review plus the OK from the CI
  system is sufficient to merge the request.

* New features like SLI or PyNEST functions, neuron or synapse models
  need to be accompanied by one or more tests written in either SLI or
  Python.

* Each change to the code has to be reflected in the
  corresponding examples and documentation.

* A pull request should be coherent and contain only changes that
  belong together.

* Please also check that the typesetting of the documentation looks
  correct. To learn how to test the documentation locally offline,
  please check out our :ref:`User documentation workflow
  <userdoc_workflow>`.


Before merging, reviewers have to make sure that:

1. pull request titles match the actual content of the PR and
   be adequate for the release notes

1. pull request titles are complete sentences that start with an
  upper-case, present-tense verb and end without punctuation

1. pull requests are assigned to projects and properly and completely
   labeled

1. all discussions are settled and all conversations are marked as
   resolved

1. there are no blocking issues mentioned in the comments
