# this Dockerfile must be built using the following command:
#     docker build -f ../ci/Dockerfile .
# this command must be run while in the `debian` directory in the root of the repo.
FROM ubuntu:jammy
LABEL name=darling-build-image version=0.1.0
ARG DEBIAN_FRONTEND="noninteractive"
RUN cp /etc/apt/sources.list /etc/apt/sources.list.d/sources-src.list && sed -i 's|deb http|deb-src http|g' /etc/apt/sources.list.d/sources-src.list
RUN apt-get -y update
RUN apt-get -y install cmake clang bison flex libfuse-dev libudev-dev pkg-config libc6-dev-i386 gcc-multilib libcairo2-dev libgl1-mesa-dev libglu1-mesa-dev libtiff5-dev libfreetype6-dev git git-lfs libelf-dev libxml2-dev libegl1-mesa-dev libfontconfig1-dev libbsd-dev libxrandr-dev libxcursor-dev libgif-dev libavutil-dev libpulse-dev libavformat-dev libavcodec-dev libswresample-dev libdbus-1-dev libxkbfile-dev libssl-dev python2 llvm-dev libvulkan-dev && apt clean -y
RUN apt-get -y install lsb-release lsb-core && apt clean -y
RUN groupadd -g 1001 ci
RUN useradd -u 1001 -g 1001 -m ci
RUN apt-get -y install devscripts equivs debhelper && apt clean -y
COPY control /control
RUN mk-build-deps -i -r -t "apt-get --no-install-recommends -y" /control && apt clean -y
RUN rm /control
USER ci
