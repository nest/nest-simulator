# Continuous Integration

[**Continuous
Integration**](http://en.wikipedia.org/wiki/Continuous_integration)
(CI) is a software development practice where quality control is
continuously applied to the product as opposed to the traditional
procedure of applying quality control *after* completing all
development (during the so called *integration phase*). In essence, it
is a way of decreasing the risks associated with the integration by
spreading required efforts over time, which helps to improve on the
quality of software, and to reduce the time taken to deliver it.

Given the limited amount of available resources, it is impossible for
a single developer to test all combinations of configuration options
and different compiler and library versions on all target platforms
upon a commit. In order to address this problem, the [TravisCI system]
(https://travis-ci.org/nest/nest-simulator) is triggered for every
[pull
request](http://nest.github.io/nest-simulator/development_workflow) to
the [central source code
repository](https://github.com/nest/nest-simulator).

This allows for regular and automated testing of changes that are
getting into the tree and timely reporting of identified
problems. This way, issues will be discovered earlier and the amount
of efforts to fix them will be significantly decreased.

## Build jobs

The CI system is set up to run upon commits to branches that are
related to a pull request, or for commits that are in a fork for
which CI is enabled. Whenever changes are detected, the latest source
code is downloaded to an executor machine and the following actions
are performed:

- Install optional and mandatory packages that NEST may use
- Perform static source code analysis using
  [Vera++](https://bitbucket.org/verateam/vera/wiki/Home)
- Perform static source code analysis using
  [Cppcheck](http://cppcheck.sourceforge.net/)
- Check [source code formatting](https://nest.github.io/nest-simulator/coding_guidelines_c++)
  using [ClangFormat](http://clang.llvm.org/docs/ClangFormat.html)
- Bootstrap the build system
- Build and install NEST
- Run the test suite

If any of these steps fails (returns a non-zero exit code), the build is
marked as failed and a notification is added to the commit or pull request on GitHub.

## Further reading

-   [Martin Fowler's paper on Continuous
    Integration](http://martinfowler.com/articles/continuousIntegration.html)

-   [Continuous Integration vs Continuous Delivery vs Continuous
    Deployment: what is the
    difference?](http://www.itwriting.com/blog/4797-continuous-integration-vs-continuous-delivery-vs-continuous-deployment-what-is-the-difference.html)

-   [8 Principles of Continuous
    Delivery](http://java.dzone.com/articles/8-principles-continuous)
