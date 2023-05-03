# this Dockerfile must be built using the following command:
#     docker build -f ../ci/RPM.Dockerfile .
# this command must be run while in the `rpm` directory in the root of the repo.
FROM fedora:37
LABEL name=darling-build-image-fedora version=0.1.0
RUN dnf install -y rpm-build dnf-utils rpmdevtools git; \
    source /etc/os-release; \
    dnf install -y https://download1.rpmfusion.org/free/fedora/rpmfusion-free-release-${VERSION_ID}.noarch.rpm; \
    dnf clean all
RUN groupadd -g 1001 ci
RUN useradd -u 1001 -g 1001 -m ci
COPY SPECS/darling.spec /darling.spec
RUN yum-builddep -y /darling.spec; \
    dnf clean all
RUN rm /darling.spec
RUN dnf install -y sudo; \
    dnf clean all
RUN echo 'ci ALL=(ALL) NOPASSWD:ALL' >> /etc/sudoers
RUN mkdir -p /src
RUN dnf install -y ccache; \
    dnf clean all
RUN mkdir -p /ccache
USER ci
