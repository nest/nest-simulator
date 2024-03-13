# -*- coding: utf-8 -*-
#
# check_conda_package_build.py
#
# This file is part of NEST.
#
# Copyright (C) 2004 The NEST Initiative
#
# NEST is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 2 of the License, or
# (at your option) any later version.
#
# NEST is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with NEST.  If not, see <http://www.gnu.org/licenses/>.


"""Script to check if NEST conda forge package builds with master.

This script is supposed to be run from CI.
"""

import json
import os
import time

import requests

try:
    pat = os.environ.get("CI_PAT")
except NameError:
    print("Token not available!")

CI_NAME = os.environ.get("CI_NAME")
CI_MAIN_NAME = os.environ.get("CI_MAIN_NAME")
CI_REPO = os.environ.get("CI_REPO")
CI_PR_NUM = os.environ.get("CI_PR_NUM")
GITHUB_SHA = os.environ.get("GITHUB_SHA")

url = "https://api.github.com/graphql"
headers = {"Content-Type": "application/json", "Authorization": f"Bearer {pat}"}


def findOID():
    payload = (
        '{"query":"query findOID {  repository(owner: \\"'
        + str(CI_NAME)
        + '\\", name: \\"'
        + str(CI_REPO)
        + '\\") { refs(first: 1, refPrefix: \\"refs/heads/\\", query: \\"dev\\") {nodes {name \\n target \
            {\\n ... on Commit {history(first: 1) {nodes {oid}}}}}}}}","variables":{}}'
    )
    response = requests.request("POST", url, headers=headers, data=payload)
    oid = json.loads(response.text)["data"]["repository"]["refs"]["nodes"][0]["target"]["history"]["nodes"][0]["oid"]
    return oid


def listWorkflowStatus():
    payload = (
        '{"query":"query listWorkflowsRun {\\n  repository(owner: \\"'
        + CI_MAIN_NAME
        + '\\", name: \\"'
        + CI_REPO
        + '\\") {\\n    pullRequest(number: '
        + CI_PR_NUM
        + ') {\\n      commits(last: 1) {\\n edges {\\n node {\\n commit {\\n checkSuites(first: 1) {\\n nodes \
            {\\n status\\n checkRuns(last: 16) {\\n nodes {\\n conclusion\\n completedAt\\n detailsUrl\\n status\\n }\
            \\n }\\n }\\n }\\n }\\n }\\n }\\n }\\n }\\n }\\n}","variables":{}}'
    )
    status = requests.request("POST", url, headers=headers, data=payload)
    return status


def createCommitOnBranch():
    oid = findOID()
    payload = (
        '{"query":"mutation createCommit($input: CreateCommitOnBranchInput!) {  createCommitOnBranch(input: $input) \
            {    commit {      url    }  }}","variables":{"input":{"branch":{"repositoryNameWithOwner":"'
        + CI_NAME
        + "/"
        + CI_REPO
        + '","branchName":"dev"},"message":{"headline":"Build from '
        + GITHUB_SHA
        + '!"},"fileChanges":{"additions":[{"path":"recipe/conda_build_log.txt","contents":"'
        + GITHUB_SHA
        + '"}]},"expectedHeadOid":"'
        + oid
        + '"}}}'
    )
    response = requests.request("POST", url, headers=headers, data=payload)
    status = ""
    while status != "COMPLETED":
        fullstatus = listWorkflowStatus()
        status = json.loads(fullstatus.text)["data"]["repository"]["pullRequest"]["commits"]["edges"][0]["node"][
            "commit"
        ]["checkSuites"]["nodes"][0]["status"]
        checknodes = json.loads(fullstatus.text)["data"]["repository"]["pullRequest"]["commits"]["edges"][0]["node"][
            "commit"
        ]["checkSuites"]["nodes"][0]["checkRuns"]
        # First give time to start the CI and then poll
        time.sleep(90)  # time to start could be long
        for stat in checknodes["nodes"]:
            if (
                stat["conclusion"] == "FAILURE"
                or stat["conclusion"] == "CANCELLED"
                or stat["conclusion"] == "TIMED_OUT"
                or stat["conclusion"] == "STARTUP_FAILURE"
                or stat["conclusion"] == "ACTION_REQUIRED"
            ):
                print("SOMETHING WENT WRONG!\n")
                print(f"{stat['conclusion']} - {(stat['status'])}")
    else:
        print("\n CHECK COMPLETED! \n")
    print(f"\n COMMIT RESPONSE \n -------------- \n{response.text}")


createCommitOnBranch()
