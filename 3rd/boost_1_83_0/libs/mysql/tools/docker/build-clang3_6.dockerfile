#
# Copyright (c) 2019-2023 Ruben Perez Hidalgo (rubenperez038 at gmail dot com)
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
#

FROM ubuntu:16.04

RUN \
    apt-get update && \
    apt-get --no-install-recommends -y install \
        software-properties-common \
        ca-certificates \
        libssl-dev \
        git \
        python3 \
        mysql-client && \
    add-apt-repository -y ppa:ubuntu-toolchain-r/test && \
    apt-get --no-install-recommends -y install clang-3.6 && \
    ln -s /usr/bin/clang++-3.6 /usr/bin/clang++ && \
    ln -s /usr/bin/clang-3.6 /usr/bin/clang && \
    ln -s /usr/bin/python3 /usr/bin/python
