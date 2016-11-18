---
layout: index
---

[« Back to the index](index)

[GitHub]: <http://github.com/> "GitHub"
[NEST]: <http://www.nest-simulator.org/> "NEST"
[NEST GitHub Organization]: <https://github.com/nest>
[NEST GitHub]: <https://github.com/nest/nest-simulator> "NEST GitHub"
[NEST Issue Tracker]: <https://github.com/nest/nest-simulator/issues> "NEST Issue Tracker"
[NEST private]: <https://github.com/nest/nest-private>


* TOC
{:toc}

# Reporting bugs and issues

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

# Basic Git Setup

The [NEST] development team uses [Git](https://git-scm.com/) for version control.
The following sections document the general steps required to set up a working
installation of Git. If you have Git set up already, skip to 
[the section describing the development workflow](#development-workflow).
This document is heavily based on the [NumPy Developer
Documentation](https://github.com/numpy/numpy/tree/master/doc/source/dev/gitwash)
:)

For more information on using Git, please refer to the [Git book](http://git-scm.com/book/en/v2).

## Installation and global setup

* [Install Git](http://git-scm.com/book/en/v2/Getting-Started-Installing-Git)
* Introduce yourself to Git:

``` sh
git config --global user.email you@yourdomain.example.com
git config --global user.name "Your Name Comes Here"
```

## Setting up your GitHub account

The [NEST] source code is hosted in a public repository on
[GitHub](https://en.wikipedia.org/wiki/GitHub). If you don’t have a GitHub
account already, please [create one][GitHub].

You then need to configure your account to allow write access - please see the
[article on generating SSH keys](http://help.github.com/articles/generating-ssh-keys/)
on the [GitHub help website](https://help.github.com/).

----------------------------------------------------------------------------

# Making your own copy (fork) of NEST

This needs to be done only once. The instructions here are very similar to the
[instructions at GitHub](http://help.github.com/forking/) - which you can refer
to for more details. This documentation includes a version specific to [NEST].

## Create your own forked copy of NEST

* [Login](https://github.com/login) to your GitHub account.
* Go to the [NEST source code repository on GitHub](https://github.com/nest/nest-simulator).
* Click on the *Fork* button:

![fork button](images/fork-button.png)

After a short pause, you should find yourself at the home page for your own
forked copy of [NEST].


## Downloading your fork

After forking the repository, you need to download it to your local computer to
work with the code.

### Command list

The following commands should do it. The next section explains the commands.

```sh
git clone git@github.com:your-user-name/nest-simulator.git
cd nest-simulator
git remote add upstream git://github.com/nest/nest-simulator.git
```

### Commands explained

**Clone your fork**

```sh
git clone git@github.com:your-user-name/nest-simulator.git
```

This downloads your fork to your local system.  Investigate. Change directory
to your new repository: `cd nest-simulator`.
Then `git branch -a` to show you all branches. You’ll get something like:

    * master
    remotes/origin/master

This tells you that you are currently on the `master` branch, and that you
also have a `remote` connection to `origin/master`. The `master` branch is the
default branch and this is where code that has been reviewed and tested resides. 
`origin/master` is just a copy of the `master` branch on your system on the `remote`.

What remote repository is `remote/origin`? Try `git remote -v` to see the web
address for the remote. It should point to your GitHub fork.

Next, you connect your local copy to the central [NEST GitHub
repository][NEST GitHub], so that you can keep your local copy and remote fork
up to date in the future. Conventionally, the main source code repository is
called `upstream`.

**Link your repository to the upstream repository**

```sh
cd nest-simulator
git remote add upstream git://github.com/nest/nest-simulator.git
```

Note that we’ve used `git://` in the web address instead of `git@`.
The `git://` web address is read only and ensures that you don't make any
accidental changes to the `upstream` repository (if you have permissions to
write to it, of course).

Check that you have a new `remote` set up with `git remote -v show`. You should
see something like this:

```
upstream     git://github.com/nest/nest-simulator.git (fetch)
upstream     git://github.com/nest/nest-simulator.git (push)
origin       git@github.com:your-user-name/nest-simulator.git (fetch)
origin       git@github.com:your-user-name/nest-simulator.git (push)
```

----------------------------------------------------------------------------

# Suggested Development Workflow

Once you've already set up your forked copy of the [NEST source code
repository][NEST GitHub], you can now start making changes to it. The following
sections document the suggested Git workflow.

## Basic workflow

In short:

1. Start a *new branch* for each set of changes that you intend to make.
    See the section on [making a new feature branch](#making-a-new-feature-branch) below.
2. Hack away! See the section that documents the [editing workflow](#editing-workflow).
3. When you are satisfied with your edits, push these changes to your own GitHub
    fork, and open a pull request to notify the development team that you'd like
    to make these changes available at the `upstream` repository.
    The steps for this are documented in the section on [creating a pull
    request](#create-a-pull-request).

This suggested workflow helps to keep the source code repository properly
organized. It also ensures that the history of changes that have been made to
the source code (called `commit history`) remains tidy - making it easier to follow.

### Making a new feature branch

Before you make any changes, ensure that your local copy is up to date with the
`upstream` repository. 


```
# go to (checkout) the default master branch
git checkout master
# download (fetch) changes from upstream
git fetch upstream
# update your master branch - merge any changes that have been made upstream
git merge upstream/master --ff-only
# update the remote for your fork
git push origin master
```

We suggest using the `--ff-only` flag since it ensures that a new
commit is not created when you merge the changes from `upstream` into your
`master` branch. Using this minimises the occurrence of superfluous merge
commits in the commit history.

Now that you have the latest version of the source code, create a new branch
for your work and check it out:

    git checkout -b my-new-feature master

This starts a new branch called `my-new-feature` from `master`.


It is extremely important to work on the latest available source code. If you
work on old code, it is possible that in the meantime, someone else has
already made more changes to the same files that you have also edited. This
will result in [merge
conflicts](https://git-scm.com/book/en/v2/Git-Branching-Basic-Branching-and-Merging#Basic-Merge-Conflicts)
and resolving these is extra work for both the development team and you. It
also muddles up the `commit history` of the source code.

### Editing workflow - command list

```
# improve 'modified_file' with your text editor/IDE
# confirm what files have changed in the repository
git status
# review the changes you've made
git diff # Optional
# inform git that you want to save these changes
git add modified_file
# save these changes
git commit
# push these changes to the remote for your fork
git push origin my-new-feature
```

### Editing workflow - commands explained

* Make some changes. When you feel that you've made a complete, working set of
  related changes, move on to the next steps.
* Please ensure that you have followed the coding guidelines for
  [C++](coding_guidelines_c++) and [SLI](coding_guidelines_sli).
* Then test your changes by building the source code and running the tests.
  (Usually `cmake ...; make; make install; make installcheck`. Please see the
  [INSTALL](https://github.com/nest/nest-simulator/blob/master/INSTALL) file for
  details.)
* Check which files have changed with `git status`. You'll see a listing like this one

```
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
```

* Compare the changes with the previous version using `git diff`.
    This brings up a simple text browser interface that highlights the difference
    between your files and the previous version like this:

```
diff --git a/development_workflow.md b/development_workflow.md
index f05f0cd..e581f00 100644
--- a/development_workflow.md
+++ b/development_workflow.md
@@ -8,17 +8,22 @@ layout: index
 [NEST Issue Tracker]: <https://github.com/nest/nest-simulator/issues> "NEST Issue Tracker"
 [NEST private]: <https://github.com/nest/nest-private>
 
-# Getting started with Git development
 
-This section and the next describe in detail how to set up git for working with
-the NEST source code. If you have git already set up, skip to
-[Development workflow](#development-workflow)
-This document is heavily based on the
-[NumPy Developer Documentation](https://github.com/numpy/numpy/tree/master/doc/source/dev/gitwash) :)
+# Basic Git Setup
 
-## Basic Git setup
+The [NEST] development team uses [Git](https://git-scm.com/) for version control.
+The following sections document the general steps required to set up . If you
+have Git set up already, skip to 
+[the section describing the development workflow](#development-workflow).
+This document is heavily based on the [NumPy Developer
+Documentation](https://github.com/numpy/numpy/tree/master/doc/source/dev/gitwash)
+:)
```

* Inform Git of what modified or new files you want to save (stage) using `git
  add modified_file`. This puts the files into a `staging area`, which is a
  list of files that will be added to your next commit. Only add files that have
  related, complete changes. Leave files with unfinished changes for later
  commits.

* To commit the staged files into the local copy of your repo, do
  `git commit`. Write a clear Git commit message that describes the changes
  that you have made. (Please read [this article on writing commit
  messages](http://chris.beams.io/posts/git-commit/)).
  If a commit fixes an open issue on the [GitHub issue
  tracker](https://github.com/nest/nest-simulator/issues), include 
  `Fixes #issue_number` in the commit message. GitHub finds such keywords and
  closes the issue automatically when the pull request is merged. For a list of
  all keywords you can use, refer to [this GitHub help
  page](https://help.github.com/articles/closing-issues-via-commit-messages/).
  After saving your message and closing the editor, your commit will be saved.

* Push the changes to your forked repo on [GitHub]

```
git push origin my-new-feature
```

  Assuming you have followed the instructions in these pages, git will create
  a default link to your [GitHub] repo called `origin`. In git >= 1.7 you can
  ensure that the link to origin is permanently set by using the `--set-upstream`
  option:

```
git push --set-upstream origin my-new-feature
```

From now on git will know that `my-new-feature` is related to the
`my-new-feature` branch in your own [GitHub] repo. Subsequent push calls
are then simplified to the following

```
git push
```

It often happens that while you were working on your edits, new commits have
been added to `upstream` that affect your work. In this case, you will need to
reposition your commits on the new master. Please follow the instructions on
[rebasing your commits on master](#rebasing-on-master) section of this document
to see how this is handled.

Next, we see how to create a pull request.

### Create a pull request
When you feel your work is finished, you can create a pull request (PR). GitHub
has a nice help page that outlines the process for
[submitting pull requests](https://help.github.com/articles/using-pull-requests/#initiating-the-pull-request).
Your pull request will usually be reviewed by other [NEST] developers using the
[code review guidelines](code_review_guidelines).

----------------------------------------------------------------------------

# Advanced cases

The following sections document some advanced scenarios. Most of it applies to
members of the [NEST] developer team.

## Pushing changes to the main repo

*This is only relevant if you have commit rights to the main [NEST] repo*

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

## Rebasing on master
This updates your feature branch with changes from the upstream [NEST GitHub]
repo. If you do not absolutely need to do this, try to avoid doing
it, except perhaps when you are finished. The first step will be to update
your `master` branch with new commits from `upstream`. This is done in the same
manner as described at the beginning of
[Making a new feature branch](#making-a-new-feature-branch). Next, you need to
update the feature branch:

```
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

## Recovering from mess-ups

Sometimes, you mess up merges or rebases. Luckily, in Git it is
relatively straightforward to recover from such mistakes.

If you mess up during a rebase:

    git rebase --abort

If you notice you messed up after the rebase:

```
# reset branch back to the saved point
git reset --hard tmp
```

If you forgot to make a backup branch:

```
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

## Rewriting commit history

*Do this only for your own feature branches!*
*Do not use this if you are sharing your work with other people!*

There's an embarrassing typo in a commit you made? Or perhaps you
made several false starts you would like the posterity not to see.
This can be done via *interactive rebasing*.

Suppose that the commit history looks like this:

```
git log --oneline
eadc391 Fix some remaining bugs
a815645 Modify it so that it works
2dec1ac Fix a few bugs + disable
13d7934 First implementation
6ad92e5 * masked is now an instance of a new object, MaskedConstant
29001ed Add pre-nep for a copule of structured_array_extensions.
...
```


and `6ad92e5` is the last commit in the `master` branch. Suppose we
want to make the following changes:

* Rewrite the commit message for `13d7934` to something more sensible.
* Combine the commits `2dec1ac`, `a815645`, `eadc391` into a single one.

We do as follows:

```
# make a backup of the current state
git branch tmp HEAD
# interactive rebase
git rebase -i 6ad92e5
```

This will open an editor with the following text in it:

```
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

```
r 13d7934 First implementation
pick 2dec1ac Fix a few bugs + disable
f a815645 Modify it so that it works
f eadc391 Fix some remaining bugs
```

This means that (i) we want to edit the commit message for
`13d7934`, and (ii) collapse the last three commits into one. Now we
save and quit the editor.

Git will then immediately bring up an editor for editing the commit
message. After revising it, we get the output:

```
[detached HEAD 721fc64] FOO: First implementation
 2 files changed, 199 insertions(+), 66 deletions(-)
[detached HEAD 0f22701] Fix a few bugs + disable
 1 files changed, 79 insertions(+), 61 deletions(-)
Successfully rebased and updated refs/heads/my-feature-branch.
```

and the history looks now like this:

```
 0f22701 Fix a few bugs + disable
 721fc64 ENH: Sophisticated feature
 6ad92e5 * masked is now an instance of a new object, MaskedConstant
```

If it went wrong, recovery is again possible as explained
[above](#recovering-from-mess-up).

## Deleting a branch on GitHub

```
git checkout master
# delete branch locally
git branch -D my-unwanted-branch
# delete branch on GitHub
git push origin :my-unwanted-branch
```

(Note the colon `:` before `my-unwanted-branch`.  See also
[here](http://github.com/guides/remove-a-remote-branch).

## Several people sharing a single repository

If you want to work on some stuff with other people, where you are all
committing into the same repository, or even the same branch, then just
share it via [GitHub].

* First fork [NEST] into your account, as explained
  [above](#making-your-own-copy-(fork)-of-nest)
* Then, go to your forked repository GitHub page, say
  `http://github.com/your-user-name/nest-simulator`
* Click on the 'Admin' button, and add anyone else to the repo as a
  collaborator.

Now all those people can do:

```
git clone git@github.com:your-user-name/nest-simulator.git
```

Remember that links starting with `git@` use the ssh protocol and are
read-write; links starting with `git://` are read-only.

Your collaborators can then commit directly into that repo with the
usual:

```
git commit -am 'ENH - much better code'
git push origin master # pushes directly into your repo
```

## Finding buggy commits using git bisect

[This post](http://webchick.net/node/99) explains how you can find buggy/bad Git commits using `git bisect`.


## GPG signing your commits

It is suggested that you [sign your commits with your unique GPG
key](https://git-scm.com/book/en/v2/Git-Tools-Signing-Your-Work) to prevent
[Git horror stories](https://mikegerwitz.com/papers/git-horror-story).

----------------------------------------------------------------------------
