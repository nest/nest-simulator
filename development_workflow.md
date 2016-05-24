---
layout: index
---
[GitHub]: <http://github.com/> "GitHub"
[NEST]: <http://www.nest-simulator.org/> "NEST"
[NEST GitHub Organization]: <https://github.com/nest>
[NEST GitHub]: <https://github.com/nest/nest-simulator> "NEST GitHub"
[NEST Issue Tracker]: <https://github.com/nest/nest-simulator/issues> "NEST Issue Tracker"
[NEST private]: <https://github.com/nest/nest-private>

# Getting started with Git development

This section and the next describe in detail how to set up git for working with
the NEST source code. If you have git already set up, skip to
[Development workflow](#development-workflow)
This document is heavily based on the
[NumPy Developer Documentation](https://github.com/numpy/numpy/tree/master/doc/source/dev/gitwash) :)

## Basic Git setup

* [Install git](http://git-scm.com/book/en/v2/Getting-Started-Installing-Git)
* Introduce yourself to Git:

``` sh
git config --global user.email you@yourdomain.example.com
git config --global user.name "Your Name Comes Here"
```
----------------------------------------------------------------------------

# Making your own copy (fork) of NEST

You need to do this only once. The instructions here are very similar to the
instructions at [GitHub](http://help.github.com/forking/) - please see that page
for more detail. We’re repeating some of it here just to give the specifics for the
NEST project, and to suggest some default names.

## Set up and configure a GitHub account

If you don’t have a GitHub account, go to the [GitHub page][GitHub],
and make one.

You then need to configure your account to allow write access - see the
[Generating SSH keys help](http://help.github.com/articles/generating-ssh-keys/)
on the GitHub help system.

----------------------------------------------------------------------------

# Create your own forked copy of NEST

* [Login](https://github.com/login) to your GitHub account
* Go to the [NEST] GitHub home at [NEST GitHub]
* Click on the *Fork* button
    ![fork button](images/fork-button.png)

After a short pause, you should find yourself at the home page for your own
forked copy of [NEST].

----------------------------------------------------------------------------

# Set up your fork

## Overview

```sh
git clone git@github.com:your-user-name/nest-simulator.git
cd nest-simulator
git remote add upstream git://github.com/nest/nest-simulator.git
```

## In detail

### Clone your fork

Clone your fork to the local computer with

```sh
git clone git@github.com:your-user-name/nest-simulator.git
```

Investigate. Change directory to your new repo: `cd nest-simulator`.
Then `git branch -a` to show you all branches. You’ll get something like:

    * master
    remotes/origin/master

This tells you that you are currently on the `master` branch, and that you
also have a `remote` connection to `origin/master`. What remote repository
is `remote/origin`? Try `git remote -v` to see the URLs for the remote.
They will point to your GitHub fork.

Now you want to connect to the upstream [NEST GitHub repository][NEST GitHub],
so you can merge in changes from `master`.

### Linking your repository to the upstream repo

```sh
cd nest-simulator
git remote add upstream git://github.com/nest/nest-simulator.git
```

`upstream` here is just the arbitrary name we’re using to refer to the main [NEST] repository  at [NEST GitHub].

Note that we’ve used `git://` for the URL rather than `git@`.
The `git://` URL is read only. This means that we can’t accidentally (or
deliberately) write to the `upstream` repo, and we are only going to use it
to merge into our own code.

Just for your own satisfaction, show yourself that you now have a new `remote`,
with `git remote -v show`, giving you something like:

    upstream     git://github.com/nest/nest-simulator.git (fetch)
    upstream     git://github.com/nest/nest-simulator.git (push)
    origin       git@github.com:your-user-name/nest-simulator.git (fetch)
    origin       git@github.com:your-user-name/nest-simulator.git (push)

----------------------------------------------------------------------------

# Development Workflow

You already have your own forked copy of the [NEST repository][NEST GitHub], by
following the previous sections. Below is the recommended workflow with Git for [NEST].

## Basic workflow

In short:

1. Start a new *feature branch* for each set of edits that you do.
    See [making a new feature branch](#making-a-new-feature-branch).
2. Hack away! See [editing workflow](#editing-workflow).
3. When finished, push your feature branch to your own GitHub repo, and
    [create a pull request](#create-a-pull-request).

This way of working helps to keep work well organized and the history
as clear as possible.

### Making a new feature branch

First, update your master branch with changes that have been made in the main
[NEST repository][NEST GitHub]. In this case, the `--ff-only` flag ensures that a new
commit is not created when you merge the `upstream` and `master` branches. It is
very important to avoid merging adding new commits to `master`.

```bash
# go to the master branch
git checkout master
# download changes from github
git fetch upstream
# update the master branch
git merge upstream/master --ff-only
# Push new commits to your Github repo
git push origin master

```
You could also use `pull`, which combines `fetch` and `merge`, as follows:

    git pull --ff-only upstream master

However, never use `git pull` without explicity indicating the source
branch (as above); the inherent ambiguity can cause problems.
This avoids a common mistake if you are new to Git.

Finally create a new branch for your work and check it out:

    git checkout -b my-new-feature master

### Editing workflow overview

```bash
# hack hack
git status # Optional
git diff # Optional
git add modified_file
git commit -m 'very verbose commit message'
# push the branch to your own Github repo
git push origin my-new-feature
```

### Editing workflow in more detail

* Make some changes. When you feel that you've made a complete, working set of related changes, move on to the next steps.
* Optional: Check which files have changed with `git status`. You'll see a listing like this one

        # On branch my-new-feature
        # Changed but not updated:
        #   (use "git add <file>..." to update what will be committed)
        #   (use "git checkout -- <file>..." to discard changes in working directory)
        #
        #   modified:   README
        #
        # Untracked files:
        #   (use "git add <file>..." to include in what will be committed)
        #
        #   INSTALL
        no changes added to commit (use "git add" and/or "git commit -a")

* Optional: Compare the changes with the previous version using with `git diff`.
    This brings up a simple text browser interface that highlights the difference
    between your files and the previous version.
* Add any relevant modified or new files using `git add modified_file`. This
    puts the files into a staging area, which is a queue of files that will
    be added to your next commit. Only add files that have related, complete
    changes. Leave files with unfinished changes for later commits.
* To commit the staged files into the local copy of your repo, do
    `git commit` or `git commit -m 'very verbose commit message'`. At this point,
    if you did not specify a comitt message in the command line, a text editor
    will open up to allow you to write a commit message. Be sure that you are
    writing a properly formatted and sufficiently detailed commit message.
    After saving your message and closing the editor, your commit will be saved.

    In some cases, you will see this form of the commit command: `git commit -a`.
    The extra `-a` flag automatically commits all modified files and removes all
    deleted files. This can save you some typing of numerous `git add` commands;
    however, it can add unwanted changes to a commit if you're not careful.
* Push the changes to your forked repo on [GitHub]

        git push origin my-new-feature

Assuming you have followed the instructions in these pages, git will create
a default link to your [GitHub] repo called `origin`. In git >= 1.7 you can
ensure that the link to origin is permanently set by using the `--set-upstream`
option:

    git push --set-upstream origin my-new-feature

From now on git will know that `my-new-feature` is related to the
`my-new-feature` branch in your own [GitHub] repo. Subsequent push calls
are then simplified to the following

    git push

It may be the case that while you were working on your edits, new commits have
been added to `upstream` that affect your work. In this case, follow the
[Rebasing on Master](#rebasing-on-master) section of this document to apply
those changes to your branch.

### Create a pull request
When you feel your work is finished, you can create a pull request (PR). GitHub
has a nice help page that outlines the process for
[submitting pull requests](https://help.github.com/articles/using-pull-requests/#initiating-the-pull-request).
Your pull request will usually be reviewed by other NEST developers.
More details about naming feature branches, formatting of pull requests and
code reviews can be found in the detailed use cases below.

### Pushing changes to the main repo
*This is only relevant if you have commit rights to the main NEST repo*

When you have a set of "ready" changes in a feature branch ready for
NEST's `master`, you can push them to `upstream` as follows:

1. First, merge or rebase on the target branch.

    a) Only a few, unrelated commits then prefer rebasing:

        git fetch upstream
        git rebase upstream/master

     See [Rebasing on master](#rebasing-on-master).

    b) If all of the commits are related, create a merge commit:

        git fetch upstream
        git merge --no-ff upstream/master

2. Check that what you are going to push looks sensible:

        git log -p upstream/master..
        git log --oneline --graph

3. Push to upstream::

        git push upstream my-feature-branch:master

It is usually a good idea to use the `-n` flag to `git push` to check
first that you're about to push the changes you want to the place you
want.

### Rebasing on master
This updates your feature branch with changes from the upstream [NEST GitHub]
repo. If you do not absolutely need to do this, try to avoid doing
it, except perhaps when you are finished. The first step will be to update
your `master` branch with new commits from `upstream`. This is done in the same
manner as described at the beginning of
[Making a new feature branch](#making-a-new-feature-branch). Next, you need to
update the feature branch:

```bash
# go to the feature branch
git checkout my-new-feature
# make a backup in case you mess up
git branch tmp my-new-feature
# rebase on master
git rebase master
```

If you have made changes to files that have changed also upstream,
this may generate merge conflicts that you need to resolve. See
[below](#recovering-from-mess-up) for help in this case.

Finally, remove the backup branch upon a successful rebase:

    git branch -D tmp

### Recovering from mess-ups

Sometimes, you mess up merges or rebases. Luckily, in Git it is
relatively straightforward to recover from such mistakes.

If you mess up during a rebase:

    git rebase --abort

If you notice you messed up after the rebase:

```bash
# reset branch back to the saved point
git reset --hard tmp
```

If you forgot to make a backup branch:

```bash
# look at the reflog of the branch
git reflog show my-feature-branch

8630830 my-feature-branch@{0}: commit: BUG: io: close file handles immediately
278dd2a my-feature-branch@{1}: rebase finished: refs/heads/my-feature-branch onto 11ee694744f2552d
26aa21a my-feature-branch@{2}: commit: BUG: lib: make seek_gzip_factory not leak gzip obj
...

# reset the branch to where it was before the botched rebase
git reset --hard my-feature-branch@{2}
```

If you didn't actually mess up but there are merge conflicts, you need to
resolve those.  This can be one of the trickier things to get right.  For a
good description of how to do this, see
[this article on merge conflicts](http://git-scm.com/book/en/Git-Branching-Basic-Branching-and-Merging#Basic-Merge-Conflicts).

### Additional things you might want to do

#### Rewriting commit history

*Do this only for your own feature branches!*
*Do not use this if you are sharing your work with other people!*
There's an embarrassing typo in a commit you made? Or perhaps you
made several false starts you would like the posterity not to see.

This can be done via *interactive rebasing*.

Suppose that the commit history looks like this:

    git log --oneline
    eadc391 Fix some remaining bugs
    a815645 Modify it so that it works
    2dec1ac Fix a few bugs + disable
    13d7934 First implementation
    6ad92e5 * masked is now an instance of a new object, MaskedConstant
    29001ed Add pre-nep for a copule of structured_array_extensions.
    ...


and `6ad92e5` is the last commit in the `master` branch. Suppose we
want to make the following changes:

* Rewrite the commit message for `13d7934` to something more sensible.
* Combine the commits `2dec1ac`, `a815645`, `eadc391` into a single one.

We do as follows:

```bash
# make a backup of the current state
git branch tmp HEAD
# interactive rebase
git rebase -i 6ad92e5
```

This will open an editor with the following text in it:

```bash
pick 13d7934 First implementation
pick 2dec1ac Fix a few bugs + disable
pick a815645 Modify it so that it works
pick eadc391 Fix some remaining bugs

# Rebase 6ad92e5..eadc391 onto 6ad92e5
#
# Commands:
#  p, pick = use commit
#  r, reword = use commit, but edit the commit message
#  e, edit = use commit, but stop for amending
#  s, squash = use commit, but meld into previous commit
#  f, fixup = like "squash", but discard this commit's log message
#
# If you remove a line here THAT COMMIT WILL BE LOST.
# However, if you remove everything, the rebase will be aborted.
#
```

To achieve what we want, we will make the following changes to it:

    r 13d7934 First implementation
    pick 2dec1ac Fix a few bugs + disable
    f a815645 Modify it so that it works
    f eadc391 Fix some remaining bugs

This means that (i) we want to edit the commit message for
`13d7934`, and (ii) collapse the last three commits into one. Now we
save and quit the editor.

Git will then immediately bring up an editor for editing the commit
message. After revising it, we get the output:

    [detached HEAD 721fc64] FOO: First implementation
     2 files changed, 199 insertions(+), 66 deletions(-)
    [detached HEAD 0f22701] Fix a few bugs + disable
     1 files changed, 79 insertions(+), 61 deletions(-)
    Successfully rebased and updated refs/heads/my-feature-branch.

and the history looks now like this:

     0f22701 Fix a few bugs + disable
     721fc64 ENH: Sophisticated feature
     6ad92e5 * masked is now an instance of a new object, MaskedConstant

If it went wrong, recovery is again possible as explained
[above](#recovering-from-mess-up).

#### Deleting a branch on github

```bash
git checkout master
# delete branch locally
git branch -D my-unwanted-branch
# delete branch on github
git push origin :my-unwanted-branch
```

(Note the colon `:` before `test-branch`.  See also
[here](http://github.com/guides/remove-a-remote-branch).

#### Several people sharing a single repository

If you want to work on some stuff with other people, where you are all
committing into the same repository, or even the same branch, then just
share it via [GitHub]

First fork [NEST] into your account, as from
[above](#making-your-own-copy-(fork)-of-nest)


Then, go to your forked repository GitHub page, say
`http://github.com/your-user-name/nest-simulator`

Click on the 'Admin' button, and add anyone else to the repo as a
collaborator.

Now all those people can do:

```bash
git clone git@github.com:your-user-name/nest-simulator.git
```

Remember that links starting with `git@` use the ssh protocol and are
read-write; links starting with `git://` are read-only.

Your collaborators can then commit directly into that repo with the
usual:

```bash
git commit -am 'ENH - much better code'
git push origin master # pushes directly into your repo
```

----------------------------------------------------------------------------

# Use cases for the new platform



## Karolina wants to fix a minor bug in a neuron model


Karolina has identified a minor issue with a neuron model she is using. To
inform the other developers about the problem, she creates a ticket in the
public issue tracker on GitHub, which gives other people the chance to discuss
the issue and possible solutions. To create a ticket on GitHub, Karolina goes
to [NEST Issue Tracker] and clicks on the green button “New Issue”.

Then, she will have to specify a title for the ticket which mentions
in a few words what the problem is, and will have to write a short description of
the problem in the text field. The text can be formatted using markdown syntax and
can be previewed by switching to the “Preview” tab. Karolina can reference other
tickets by indicating them with their issue number, for example `#123`. Existing
Pull Requests can be also referenced by their number (Issues and Pull Requests share
the same number space). In the text Karolina can also reference commits in the repo
by specifying their SHA-1 hash: [GitHub] will add a link to the correspoding commit
automatically.

Finally, Karolina can notify other NEST developers by mentioning
them with their GitHub account name prefixed by the symbol `@`, as in
`@a-user-name`. In this case developer `a-user-name` will get an email
notification that she should come and view the Issue. Karoline submits the issue by
clicking on the corresponding green button “Submit new issue”. The issue will get
assigned an issue number, for example `#123`.

Karolina is happy with the fact that other people can watch her fixing the bug.
When she knows how to fix the problem, after having forked the official NEST repo
as explained in the [Making your own copy (fork) of NEST](#making-your-own-copy-(fork)-of-nest)
section, she creates a feature branch on her local clone, as in
[making a new feature branch](#making-a-new-feature-branch)

```bash
git checkout -b fix-123
```

She explicitly mentions the issue number in the branch name, so that even to
a casual read it will be clear what the branch is about. She writes a regression
test and fixes the problem. In the process, she commits often and runs the test suite
to verify that none of her commits break anything. This is described in the
[basic workflow](#basic-workflow).

When the regression test stops failing and she is happy with the result, she is
ready to submit a Pull Request. If her git-foo is high enough, she may decide to
clean up her local git history by using some combination of `git rebase -i` as decribed
in [Rewriting commit history](#rewriting-commit-history) but this is not strictly
necessary and can mess up things very badly if she doesn't know what she is doing,
so she can skip this step. After that, she pushes the branch to her [GitHub] fork by doing

```bash
git push origin fix-123
```

To generate the Pull Request, she now goes to her fork on [GitHub], for example
https://github.com/a-user-name/nest-simulator , to see that [GitHub] highlights
her branch in a yellow box with a green button “Compare & pull request”.

When she clicks on the button, GitHub will allow her to give a title to the Pull
Request. The title should be something on the line of *Bugfix and test for issue
123*. In the text field, Karolina will start with a sentence like *This Pull
Request fixes #123*. By using the words `fixes #123` GitHub will automatically
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
flag their agreement in the form of a comment like `merge approved` to the Pull
Request. The release manager can now merge the Pull Request.

## Jeyashree wants to add a new neuron model to NEST

Jeyashree wants to develop a new neuron or synapse model for NEST as part of
her PhD project. As this is an internal project, she is using a private
repository in the NEST organization on GitHub to benefit from the CI, but still
allow her supervisor and other members of the NEST developer team to have
a look (similar to the workflow using the *developer module*, which was used in
the past).

Jeyashree will start by asking the Release Manager to add her [GitHub]
account to the [NEST GitHub Organization]. The Release Manager will invite her to
join the organization and she will receive a notification of the invitation. She
will accept the invitation by clicking on the link in the email from GitHub. Now
she can create a private fork by going to the
[private NEST repository on GitHub][NEST private] and
click the fork button there. As a target she selects her GitHub account. Note that, because
she is forking a private repository, her fork will also be private: only members
of the NEST organization will be able to see her code in
https://github.com/a-user-name/nest-private .

From here on, the workflow is the same as it was in the
[Karolina wants to fix a minor bug in a neuron model](#karolina-wants-to-fix-a-minor-bug-in-a-neuron-model)
use case, with the noticeable difference that the branch name now will be something like
`neuron-model-XYZ`. She will not submit any Pull Request, but will just push
to her private fork. The collaborators will still be able to open issues or submit
Pull Requests to her, if they propose changes to which Jeyashree agrees with. To do
this the reviewers will have to fork https://github.com/a-user-name/nest-private to
their [GitHub] accounts.

After a first development phase, she submits a paper about the first results of
her research using the new neuron model. She wants the corresponding
code to be available in the [NEST public repository][NEST GitHub] to the readers.
For this, she first [forks the official (and public) NEST repository](#making-your-own-copy-(fork)-of-nest).

This fork will now be public and everyone will see it linked from the official
NEST repository. She now adds this fork to her local clone of her private repo
as a new remote:

```bash
git remote add public git://github.com/a-user-name/nest-simulator
```

now she can push her branch to her public fork:

```bash
git push public neuron-model-XYZ
```

From here on the workflow is the same as for
[Karolina wants to fix a minor bug in a neuron model](#karolina-wants-to-fix-a-minor-bug-in-a-neuron-model).

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

## Jakob and Tammo want to implement a new communication infrastructure

Because Tammo and Jakob are advanced users, they set up their own Git
repository on their own server for maximumg.

They are using a private repo. They will take care of adding the official NEST
repository as a remote in their clone:

```bash
git add remote upstream git://github.com/nest/nest-simulator
```

They will usually work on feature branches. To keep their fork up to date
with the public repo, they will pull the changes to the master branch in the
[official NEST repository][NEST GitHub] as explained [here](#reasing-on-master)

Now their changes have been applied on top of the last commit in the
[official NEST repository][NEST GitHub] master branch!

This workflow is more complicated and many things can go wrong in case some merge
conflict do appear after all. Much easier would have been for Tammo and Jacob to
use a private repository in the NEST organization and do the same as Jeyashree in
[Jeyashree wants to add a new neuron model to NEST](#jeyashree-wants-to-add-a-new-neuron-model-to-nest)

## Abigail wants to fix a misleading piece of documentation

Abigail wants to fix the documentation of a single file and figures that changing the file directly through the GitHub web interface is the easiest way to go.

On the GitHub page she navigates to her personal fork of NEST and browses to the file she wants to edit. She clicks the button on the top right corner that allows her to edit this file and then fixes the documentation. Once she is done, she writes a meaningful commit message under "Commit changes" at the bottom of the page and selects the option "Create a new branch for this commit and start a pull request". She types in an appropriate branch name and finally clicks "Commit changes".

As Abigail has made only minor changes to the documentation, she did not create a ticket in the bug tracker and she includes the flag `[ci skip]` in the title of the Pull Request to keep Travis from running any tests.

If Abigail is an owner of the NEST repository, she can also add the label “documentation” to the Pull Request. Otherwise, she can ask one of the owners to do so.

If no one objects, the release manager merges the Pull Request into the public version of NEST.

## Alex wants to fix a bug in the NEST kernel

Old SVN, old bug tracker, controversy about the way the problem was fixed in
review

## Hannah wants to extend a neuron model by additional receptor types

travis is happy first, but the code rots for some time due to vacation and
reporting season. Then Travis is unhappy.


## Susanne wants to change the API of a function in PyNEST

The code review is fine, because the code quality is superb. However, the
release manager is concerned that the review was not thorough enough and did
not consider all apsects of the API change.

Start a wider discussion on the developer list
