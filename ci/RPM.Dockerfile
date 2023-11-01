# this Dockerfile must be built using the following command:
#     docker build -f ../ci/RPM.Dockerfile .
# this command must be run while in the `rpm` directory in the root of the repo.
ARG BASE_IMAGE=fedora:37
FROM $BASE_IMAGE
LABEL name=darling-build-image-fedora version=0.1.0
ARG CI_UID=1111
ARG CI_GID=1111
RUN dnf install -y rpm-build dnf-utils rpmdevtools git; \
    source /etc/os-release; \
    dnf install -y https://download1.rpmfusion.org/free/fedora/rpmfusion-free-release-${VERSION_ID}.noarch.rpm; \
    dnf clean all
RUN groupadd -g "${CI_GID}" ci
RUN useradd -u "${CI_UID}" -g "${CI_GID}" -m ci
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
