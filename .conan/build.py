#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os
import re
from cpt.packager import ConanMultiPackager


def get_version():
    with open("CMakeLists.txt") as cmake:
        content = cmake.read()
        match = re.search(r'project\(uvw VERSION (.*)\)', content)
        if match:
            return match.group(1)
        return os.getenv("TRAVIS_BRANCH", "master")

def get_username():
    return os.getenv("CONAN_USERNAME", "skypjack")


def get_reference():
    version = get_version()
    username = get_username()
    return "uvw/{}@{}/stable".format(version, username)


def get_upload():
    username = get_username()
    url = "https://api.bintray.com/conan/{}/conan".format(username)
    default_upload = url if os.getenv("TRAVIS_TAG") else False
    return os.getenv("CONAN_UPLOAD", default_upload)


if __name__ == "__main__":
    test_folder = os.path.join(".conan", "test_package")
    builder = ConanMultiPackager(reference=get_reference(),
                                 username=get_username(),
                                 upload=get_upload(),
                                 remote="https://api.bintray.com/conan/bincrafters/public-conan",
                                 test_folder=test_folder)
    builder.add(settings={"compiler": "gcc", "compiler.version": "7",
                          "arch": "x86_64", "build_type": "Release"},
                options={}, env_vars={}, build_requires={})
    builder.run()
