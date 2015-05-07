Introduction
============

Currently, the NEST development infrastructure consists of four main services:

-   A in form of a Subversion server.

-   A and an on a trac server.

-   for developers and users runing on Mailman.

-   based on Jenkins.

All services are self-hosted on physical hardware in either Freiburg
or Jülich, or on a virtualized server at the Webtropia data center in
Düsseldorf. All systems are using different operating systems and
access control mechanisms, which makes maintainance and backup
non-trivial tasks.

As the number of developers is growing constantly and we thus need
better control over the quality of code changes, we would like to
extend the current infrastructure by a code review platform system,
which would further increase the complexity of the whole system.

On the 5th of July 2012, the NEST Steering Committee decided that
NEST’s source code shall be releases under the GPL instead of the
proprietary NEST License and that there will be a public Git
repository.

A first plan for the Subversion-Git migration was announced in the
[100k project meeting on the 24th of September, 2012](https://www.csn.fz-juelich.de/blog/100kproject/?p=1166).
Since then, however, there was basically a standstill on the project.

The basic plan back then was to clean up the repository, export some
version of NEST from the SVN, import that into a new Git, and re-base
all active branches onto that branch.

The reason for using a SVN export for starting with Git is that the
history is “poisoned” with a lot of “private” development that was
never meant to become public. Given that at the moment the whole
history *is* private, by starting afresh with an empty history you
don’t take anything away from the (non-developer) users and allow for
faster and smaller clones, which is always a plus.

This way nothing “private” gets leaked by mistake, which is not easy
to ensure otherwise in a history \(>\) 10 years. The history wouldn’t
be lost, though. It will be either in the frozen SVN or in an ad-hoc
private Git repository somewhere. So, those who care about the history
and have the privileges to read it now, will also be able to do so in
the future.

Action plan
===========

The following is the result of some discussions between 
[Alex Peyser](mailto:a.peyser@fz-juelich.de),
[Jochen M. Eppler](mailto:j.eppler@fz-juelich.de),
[Abigail Morrison](mailto:morrison@fz-juelich.de),
[Rajalekshmi Deepu](mailto:r.deepu@fz-juelich.de),
[Tiziano Zito](mailto:t.zito@fz-juelich.de),
[Tammo Ippen](mailto:t.ippen@fz-juelich.de), and
[Markus Diesmann](mailto:diesmann@fz-juelich.de)
to finally get the work on consolidating the NEST development
infrastructure going again.

-   We will switch from SVN to Git, once the 4g release is out. This is
    expected for the end of 2014.

-   Prerequisites for the switch are the removal of the developer module and
    adaptation of pertaining code in release and build scripts (see
    [#707](https://trac.nest-initiative.org/trac/ticket/707) ).

-   We want to have a public Git without history starting with the 4g code
    released as NEST 2.6.0 and a public issue tracker.

-   In addition to the public Git we want the old SVN+trac frozen for
    reference.

-   All existing branches need to be re-based on the public Git.

-   According to the research by Rajalekshmi Deepu and Alex Peyser, open source
    solutions for code review and continuous integration (CI) exist (e.g.
    gerrit and Jenkins), but require a lot of know-how and man power for
    installation and maintainance on our side.

-   Integrated solutions like GitHub and JetBrains are impossible to beat
    quality-wise.

-   Possible contributors already know how to use GitHub and there is no
    interaction required from us to allow them to do so (e.g. creating
    accounts, explaining policies, discussing about permissions, ...).

-   It is possible to do a local installation of GitHub on own hardware, which,
    however, burdens us with maintainance and backup again.

-   The SimLab could provide a migration guide for other neuroscience tools and
    projects.

-   For the migration of old tickets from trac to GitHub, we could use the
    scripts that were used by NumPy and SciPy, who did this move already some
    years ago (TZ knows more).

-   In the CI, we require matrix builds for NEST and MUSIC and want dependent
    builds to test NEST with MUSIC support and PyNN with NEST and possibly the
    compatibility of a whole mesh of different neuroscience tools.

-   We need to find out if Travis CI in the commercial GitHub variant is enough
    for us, or if we need additional CI infrastructure here.

-   The group above agrees that paid GitHub would be the best solution for code
    hosting, code review platform and ticket and repository hosting atm

Git based workflow
==================

The idea would be to have a pure pull request (PR) based workflow, which works as following:

-   On the central GitHub public repository only PRs are merged by the release
    manager(s). No direct commits are allowed.

-   Developers and users alike should fork the central public repo if they want
    to contribute, and work on their own forks and branches until they are
    ready for a PR.

-   If the devs need private repos, they can have them on GitHub, or everywhere
    else actually. When their work is ready for the public, it would have to
    land on a public fork and a PR will be issued.

-   Internal people and PhD-students can use private repositories on GitHub in
    order to benefit from the code review platform already before the final
    commit to the public repository.

With this workflow the question of private vs. public development becomes moot:
each developer chooses what she wants, hosts her private clones wherever she
wants; if she wants code to be merged, it needs to land on a public fork and
a PR needs to be issued. No exceptions.

The release manager(s) does not need to care anymore to keep track of zillions
of private branches/repositories: s/he becomes active only when a PR is issued.
Developers don’t need to ask for permission to create a new private fork: they
just do it on the platform of their choice. That gives us maximum of
flexibility and maximum of control over what is public and what is private,
without burdening the release manager(s) with all the work.

If the PR-based workflow is used, and a GitHub-hooked CI solution is
implemented, every PR issues automatically a rebuild and the quality of
contributions gets checked automatically without having to mess with
configuring CI for different repos and different levels of “privateness”.

Code Review Guidelines
======================

Using the PR-based workflow explained above, code can be developed flexibly as
required for the use case (see ). At the same time, we can now enforce stricter
rules on code that ends up in the public repository, shifting the burden of
bringing the code into a suitable shape from the release manager to the
original developer of the code. Following are rough guidelines for the code
reviewers and the use of the platform.

In general, every Pull Request needs to get an OK from the CI platform and two
reviewers.

Each PR needs to be documented by a ticket in the issue tracker explaining the
reason for the changes an the solution. New features like SLI or PyNEST
functions, neuron or synapse models need to be accompanied by one or more tests
written either in SLI or Python. New features for the NEST kernel need a test
written in SLI.

For changes labelled “not code” or “minor” (e.g. changes in documentation,
fixes for typos, etc.), the release manager can waive the need for code review
and just accept the OK from Travis in order to merge the request.

Each change to the code has to be reflected in the corresponding examples and
documentation.

All source code has to be adhering to the new coding styles outlined in . There
is a test for the CI to check this adherence.

All Commits should be coherent and contain only changes that belong together.

Coding style
============

Use cases for the new platform
==============================

Prerequisite
------------

Everyone who wants to contribute to NEST needs a GitHub account. Just using the
public version does not require this, as anonymous clones are possible.

To create an anonymous clone:

```bash
git clone https://github.com/nest/nest-simulator.git
```

If you already have an account on GitHub, you should fork the NEST repository.
To do that, [login to GitHub](https://github.com/login), go to the 
[official NEST repository](https://github.com/nest/nest-simulator), and click
the “Fork” button on that page. If GitHub asks for a target for the fork, choose
your personal account. The “Fork” button as of now is located in the
at the top of the page, below the GitHub header, on the right side near the 
“Watch”, and "Star" buttons. This will create a copy of the official repo in
your own account, for example if your username is *otizonaizit*, the fork will
be located at https://github.com/otizonaizit/nest-simulator . To clone your
personal repository to your laptop, you can use

```bash
git clone git@github.com:otizonaizit/nest-simulator.git
```


Karolina wants to fix a minor bug in a neuron model
---------------------------------------------------

Karolina has identified a minor issue with a neuron model she is using. To
inform the other developers about the problem, she creates a ticket in the
public issue tracker on GitHub, which gives other people the chance to discuss
the issue and possible solutions. To create a ticket on GitHub, Karolonia goes 
to https://github.com/nest/nest-simulator/issues and clicks on the green button
“New Issue”. Then, she will have to specify a title for the ticket which mentions
in a few words what the problem is, and will have to write a short description of
the problem in the text field. The text can be formatted using markdown syntax and
can be previewed by switching to the “Preview” tab. Karolina can reference other
tickets by indicating them with their issue number, for example *#123*. Existing 
Pull Requests can be also referenced by their number (Issues and Pull Requests share
the same number space). In the text Karolina can also reference commits in the repo
by specifying their SHA-1 hash: GitHub will add a link to the correspoding commit
automatically. Finally, Karolina can notify other NEST developers by mentioning
them with their GitHub account name prefixed by the symbol *@*, as in
*@otizonaizit*. In this case developer *otizonaizit* will get an email
notification that she should come and view the Issue. Karoline submits the issue by
clicking on the corresponding green button “Submit new issue”. The issue will get
assigned an issue number, for example #123. 

Karolina is happy with the fact that other people can watch her fixing the bug.
When she knows how to fix the problem, after having forked the official NEST repo
as explained in the “Prerequisite” section, she creates a branch on her local clone

```bash
git checkout -b fix-123
```
She explicitly mentions the issue number in the branch name, so that even to
a casual read it will be clear what the branch is about. She writes a regression
test and fixes the problem. In the process, she commits often and runs the test suite
to verify that none of her commits break anything.

When the regression test stops failing and she is happy with the result, she is
ready to submit a Pull Request. If her git-foo is high enough, she may decide to
clean up her local git history by using some combination of 

```bash
git rebase -i
```

but this is not strictly necessary and can mess up things very badly if she doesn't
know what she is doing, so she can skip this step. After that, she pushes the
branch to her GitHub fork by doing

```bash
git push origin fix-123
```
To generate the Pull Request, she now goes to her fork on GitHub, for example
https://github.com/otizonaizit/nest-simulator , to see that GitHub highlights
her branch in a yellow box with a green button “Compare & pull request”.
When she clicks on the button, GitHub will allow her to give a title to the Pull
Request. The title should be something on the line of *Bugfix and test for issue
123*. In the text field, Karolina will start with a sentence like *This Pull
Request fixes #123*. By using the words *fixes #123* GitHub will automatically
close the corresponding issue when the Pull Request is merged. The rest of the text
should explain what the fix is about, of course. When she is happy with the text,
she clicks the green button “Create pull request”.  

Travis tries to merge the Pull Request, build it and run all the tests. Because
Karolina already took care that everything works, the build succeeds and the
NEST Core mailing list is informed about the new Pull Request and invited to
have a look at the change. The reviewers may ask Karolina to change something.
Karolina will receive a notification everytime a review posts a comment on her Pull
Request. She can add commits to her local clone to address the reviewers comments,
and everytime she pushes to her fork with

```bash
git push origin fix-123
```

the reviewer will get a notification that Karolina has addressed their comments.

When Karolina and the reviewers are happy with the Pull Request, they will flag 
flag their agreement in the form of a comment like *merge approved* to the Pull
Request. The release manager can now merge the Pull Request. 

Jeyashree wants to add a new neuron model to NEST
-------------------------------------------------

Jeyashree wants to develop a new neuron or synapse model for NEST as part of
her PhD project. As this is an internal prohject, she is using a private
repository in the NEST organization on GitHub to benefit from the CI, but still
allow her supervisor and other members of the NEST developer team to have
a look (similar to the workflow using the *developer module*, which was used in
the past). Jeyashree will start by asking the Release Manager to add her GitHub
account to the NEST Organization on GitHub. The Release Manager will invite her to
join the organization and she will receive a notification of the invitation. She
will accept the invitation by clicking on the link in the email from GitHub. Now
she can create a private fork by going to the private NEST repository on GitHub
???ADD LINK HERE! FAKE LINK FOLLOWS??? https://github.com/nest/nest-private and
click the fork button there. As a target she selects her GitHub account. Note that, because 
she is forking a private repository, her fork will also be private: only members
of the NEST organization will be able to see her code in
https://github.com/otizonaizit/nest-private. From here on, the workflow
is the same as it was in the "Karolina wants to fix a minor bug in a neuron model"
usecase, we noticeable difference that the branch name now will be something like
*neuron-model-XYZ*. She will not submit any Pull Request, but will just push
to her private fork. The collaborators will still be able to open issues or submit
Pull Requests to her, if they propose changes to which Jeyashree agrees with. To do
this the reviewers will have to fork https://github.com/otizonaizit/nest-private to
their GitHub accounts. 

After a first development phase, she submits a paper about the first results of
her research using the new neuron model. She wants the corresponding
code to be available in the public repository to the readers. For this, she first
forks the official (and public) NEST repository. This fork will now be public and
everyone will see it linked from the official NEST repository. She now adds this
fork to her local clone of her private repo as a new remote:

```bash
git remote add public https://github.com/otizonaizit/nest-simulator
```
now she can push her branch to her public fork:

```bash
git push public neuron-model-XYZ
```
From here on the workflow is the same as for "Karolina wants to fix a minor bug in
a neuron model".

???IS THE FOLLOWING NEEDED? ???
However, the code is very much tailored to her specific use in
the paper. One of the reviewers is not happy with this and points out possible
ways to improve its generality and make it usable to a wider audience.

The release manager agrees with the skeptical reviewer and requests more
discussions about the implementation and suggests the creation of a ticket with
details about the new model.

Jeyashree creates a ticket and agrees on a workable solution with the reviewer
after a short round of discussion in the code review platform and fixes the
remaining issues. After Travis and both reviewers signal their agreement with
the code, the release manager merges the Pull Request.

Abigail wants to fix a misleading piece of documentation
--------------------------------------------------------

???No EXAMPLE NEEDED HERE? ???
Abigail is happy with the fact that other people can watch her fixing the
documentation. She clicks the button on the GitHub page of NEST and chooses her
personal profile as the target.

She then works on her own public version of the NEST code on GitHub and creates
a once she is done by clicking on the corresponding button.

She adds a comment to the Pull Request explaining the reason for the change.

Because it is only a minor change to the documentation, she did not create
a ticket in the bug tracker, but flags the Pull Request by adding the label
“not code”.

Travis tries to merge the Pull Request, build it and run all the tests. If this
succeeds, the NEST Core mailing list is informed about the new Pull Request and
invited to have a look at the change.

If no one objects, the release manager can merge the Pull Request into the
public version of NEST.

Alex wants to fix a bug in the NEST kernel
------------------------------------------

Old SVN, old bug tracker, controversy about the way the problem was fixed in
review

Hannah wants to extend a neuron model by additional receptor types
------------------------------------------------------------------

travis is happy first, but the code rots for some time due to vacation and
reporting season. Then Travis is unhappy.

Jakob and Tammo want to implement a new communication infrastructure
--------------------------------------------------------------------

Because Tammo and Jakob are advanced users, they set up their own Git
repository on their own server for maximumg.

They are using a private repo. They will take care of adding the official NEST
repository as a remote in their clone:

```bash
git add remote upstream https://github.com/nest/nest-simulator
```
They will usually work on so-called feature branches. To keep their fork up to date
with the public repo, they will pull the changes to the master branch in the
official NEST repository with:

```bash
git checkout master
git pull upstream master
```

As they always took care of working on branches, this pull will always succeed
without merge conflicts. They will then rebase the changes in the feature branches
with something like:

```bash
git checkout feature-branch-X
git rebase master
```

Now their changes have been applied on top of the last commit in the official NEST
repository master branch!

???NO TRAVIS HERE!!! ???
Travis is unhappy in certain configurations of the matrix, because they did not
test the full matrix
??? ???

This workflow is more complicated and many things can go wrong in case some merge
conflict do appear after all. Much easier would have been for Tammo and Jacob to
use a private repository in the NEST organization and do the same as Jeyashree in 
"Jeyashree wants to add a new neuron model to NEST"

Susanne wants to change the API of a function in PyNEST
-------------------------------------------------------

The code review is fine, because the code quality is superb. However, the
release manager is concerned that the review was not thorough enough and did
not consider all apsects of the API change.

start a wider discussion on the developer list

