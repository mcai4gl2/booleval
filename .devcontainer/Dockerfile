FROM mcr.microsoft.com/devcontainers/base:ubuntu-22.04

# Install build dependencies
RUN apt-get update && export DEBIAN_FRONTEND=noninteractive \
    && apt-get -y install --no-install-recommends \
    build-essential \
    cmake \
    git \
    wget \
    clang \
    && apt-get clean -y \
    && rm -rf /var/lib/apt/lists/*

# Set default compiler to GCC (Ubuntu 22.04 provides GCC 11 by default)
ENV CC=gcc
ENV CXX=g++