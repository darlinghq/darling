ARG OS=fedora:37
FROM ${OS}

SHELL ["/bin/sh", "-euxvc"]

RUN dnf install -y rpm-build dnf-utils rpmdevtools git; \
    . /etc/os-release; \
    dnf install -y https://download1.rpmfusion.org/free/fedora/rpmfusion-free-release-${VERSION_ID}.noarch.rpm; \
    dnf clean all

COPY SPECS/darling.spec /darling.spec

# Bootstrap
RUN yum-builddep -y /darling.spec; \
    dnf clean all

RUN rm /darling.spec

RUN mkdir -p /root/rpmbuild/SOURCES

CMD /src/rpm/build.sh
