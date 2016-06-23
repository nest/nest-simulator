# Continuous Integration

## Introduction

[**Continuous Integration**](http://en.wikipedia.org/wiki/Continuous_integration) (CI)
is a software development practice where quality control is continuously
applied to the product as opposed to the traditional procedure of
applying quality control *after* completing all development (during the
so called *integration phase*). In essence, it is a way of decreasing
the risks associated with the integration by spreading required efforts
over time, which helps to improve on the quality of software, and to
reduce the time taken to deliver it.

Stringent quality control is particularly important in the context of
NEST, a neuronal network simulator with the emphasis on correctness,
reproducibility and performance. However, given the limited amount of
the available resources, it is wasteful to transfer the responsibility
to re-run the test suite for all target platforms on every single code
base change to the shoulders of the developers.

In order to address this problem, a
[Jenkins](http://jenkins-ci.org/)-based CI infrastructure was created
during the [NEST Google Summer of Code
2011](http://www.google-melange.com/gsoc/project/google/gsoc2011/zaytsev/17001)
project, which would allow for regular testing of changes that are
getting into the tree and timely reporting of identified problems. This
way, issues will be discovered earlier and the amount of efforts to fix
them will be significantly decreased (hopefully).

## Continuous Integration at NEST Initiative

The current CI implementation is now available at the following URL:

<https://qa.nest-initiative.org/>
The site is secured with a Class 1 SSL certificate issued by StartCom
Ltd.

### Build jobs

The CI system is set up to regularly poll the version control repository
hosting the main official NEST source tree. Whenever changes are
detected, the latest source code is downloaded to a build executor
machine and the following actions are performed:

-   Bootstrap the build system
-   Build and install NEST
-   Run the test suite
-   Bootstrap MyModule
-   Build and install MyModule

If any of these steps fails (returns a non-zero exit code), the build is
marked as failed and a notification is sent to the developers mailing
list.

Additionally, a number of metrics are collected from the build log to
obtain a cumulative project "health" indicators:

-   Build status:

    -   *Success* (all steps completed successfully)

    -   *Unstable* (some publishers reported non-fatal errors, e.g.
        there were failed tests)

    -   *Failed* (a fatal error happened during the build, e.g. non-zero
        exit code was returned)

-   Weather report (how many failures there were out of 5 latest builds)

-   Test suite report (all JUnit-compatible reports are aggregated)

-   Compiler warnings report (all GCC4 / LD warnings are aggregated)

The values of these indicators are archived along with the build logs
and trends can be graphed for arbitrary periods of time.

### User registration

NEST developers, using the system as registered users enjoy a number of
benefits as compared to the anonymous users. Most notably they may:

-   Access projects workspace on the build executors
-   Schedule / remove / edit builds manually
-   Be granted personal build jobs for their branches
-   Be allowed to create jobs (to be discussed)

Project workspace access allows one to view the source code and build
directories / logs as they are on the build executor machines, which can
be helpful in diagnosing build problems.

If you'd like to register with the system, please contact the
administrators!

### Technical details

The infrastructure resides on a Dell PowerEdge R710 server hosted in a
private BCF rack at Uni Freiburg running KVM hypervisor which controls
virtual machines that host Jenkins and build executors.

The repositories hosting the up to date versions of the configuration
([*anubis-puppet*](http://git.zaytsev.net/?p=anubis-puppet.git;a=summary))
and documentation
([*anubis-docs*](http://git.zaytsev.net/?p=anubis-docs.git;a=summary))
are available online. Please refer to these repositories for more
detailed documentation regarding the setup, which is out of the scope of
this brief user manual.

### Administrators

At the moment, the following NEST developers have administrative access
to the build system:

-   Yury V. Zaytsev

## Further reading

-   [Martin Fowler's paper on Continuous
    Integration](http://martinfowler.com/articles/continuousIntegration.html)

-   [Continuous Integration vs Continuous Delivery vs Continuous
    Deployment: what is the
    difference?](http://www.itwriting.com/blog/4797-continuous-integration-vs-continuous-delivery-vs-continuous-deployment-what-is-the-difference.html)

-   [8 Principles of Continuous
    Delivery](http://java.dzone.com/articles/8-principles-continuous)
