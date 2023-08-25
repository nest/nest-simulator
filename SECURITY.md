
# Security Policy

The NEST Simulator is research oriented software and as such is expected to
run mainly in well protected environments. In case it is found that this
software can be used to violate security mechanisms, developers will try to
provide patches that mitigate the risk of this misuse.

The current policy is explained in SECURITY.md in the nest-simulator
master branch.  The policy can also be found at
<https://github.com/nest/nest-simulator/security/policy>.

## Supported Versions

Generally only the latest version will receive security related
updates. In severe cases developers may create a backport of the
corresponding patch for the previous version, if it is also affected.
Currently the following versions are being supported with security
updates:

| Version  | Supported          |
| -------- | ------------------ |
| <=2.18.0 | :x:                |
| 2.20.2   | :white_check_mark: |
| 3.x      | :heavy_check_mark: |

## Reporting a Vulnerability

For reporting a vulnerability please create a security advisory on the
nest/nest-simulator [Security
Advisories](https://github.com/nest/nest-simulator/security/advisories)
page.  You need a GitHub account to create an advisory.

Developers will then contact the reporter in a timely manner to assess
severity and further handling via [Security
Advisories](https://github.com/nest/nest-simulator/security/advisories)
or as normal [Issue](https://github.com/nest/nest-simulator/issues) in
non-critical cases.

