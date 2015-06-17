---
layout: index
---

[« Back to the index](index)

<hr>

# Continuous Integration

[Continuous Integration](http://en.wikipedia.org/wiki/Continuous_integration) (CI)
is a software development practice where quality control is
continuously applied to the product as opposed to the traditional
procedure of applying quality control after completing all development
(during the so called integration phase). In essence, it is a way of
decreasing the risks associated with the integration by spreading
required efforts over time, which helps to improve on the quality of
software, and to reduce the time taken to deliver it.

Stringent quality control is particularly important in the context of
NEST, a neuronal network simulator with the emphasis on correctness,
reproducibility and performance. However, given the limited amount of
the available resources, it is wasteful to transfer the responsibility
to re-run the test suite for all target platforms on every single code
base change to the shoulders of the developers.

In order to address this problem, a
[Travis](http://travis-ci.org/)-Continuous Integration (CI) service has been setup,
which would allow for regular testing of changes that are
getting into the tree and timely reporting of identified
problems. This way, issues will be discovered earlier and the amount
of efforts to fix them will be significantly decreased (hopefully).
Continuous Integration at NEST Simulator

The current CI implementation is now available at the following URL:

    https://travis-ci.org/nest/nest-simulator

Travis CI service has been integrated with the Github repository to automatically run the tests when code is pushed. Github integration is done by adding a simple YAML file to the project root.Travis results will appear in the Github pull requests and the primary log is visible in the Travis interface. 
Build jobs

The CI system is closely integrated to Github repository. Whenever some changes in the code
are detected, the latest source code is downloaded to a worker machine which is running on Ubuntu 14.04 LTS(Trusty) and the following actions are performed:

    Bootstrap the build system
    Build and install NEST
    Run the test suite
    Uploading the build artifacts to Amazon S3 (Simple Storage Service)

If any of the first three steps fails (returns a non-zero exit code), the build
is marked as failed and a notification is sent to the person who initiated the build.

Due to security reasons, Travis doesn't allow uploading of artifacts to S3 for the pull requests.

In the case of some changes in the documentation where you don't want to trigger a Travis build, add 
[ci skip] or [skip ci] anywhere in the commit message.



## Further reading

* [Martin Fowler's paper on Continuous Integration](http://martinfowler.com/articles/continuousIntegration.html)
* [Continuous Integration vs Continuous Delivery vs Continuous Deployment: what is the difference?](http://www.itwriting.com/blog/4797-continuous-integration-vs-continuous-delivery-vs-continuous-deployment-what-is-the-difference.html)
* [8 Principles of Continuous Delivery](http://java.dzone.com/articles/8-principles-continuous)
