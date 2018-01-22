ARG OS=fedora:27
FROM ${OS}

SHELL ["bash", "-euxvc"]

RUN dnf install -y rpm-build dnf-utils rpmdevtools

CMD bash -xv /src/rpm/build.bsh