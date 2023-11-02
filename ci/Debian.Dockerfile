# this Dockerfile must be built using the following command:
#     docker build -f ../ci/Debian.Dockerfile .
# this command must be run while in the `debian` directory in the root of the repo.
ARG BASE_IMAGE=ubuntu:jammy
FROM $BASE_IMAGE
LABEL name=darling-build-image version=0.1.0
ARG DEBIAN_FRONTEND="noninteractive"
RUN cp /etc/apt/sources.list /etc/apt/sources.list.d/sources-src.list && sed -i 's|deb http|deb-src http|g' /etc/apt/sources.list.d/sources-src.list
RUN apt-get -y update
RUN apt-get -y install cmake clang bison flex libfuse-dev libudev-dev pkg-config libc6-dev-i386 gcc-multilib libcairo2-dev libgl1-mesa-dev libglu1-mesa-dev libtiff5-dev libfreetype6-dev git git-lfs libelf-dev libxml2-dev libegl1-mesa-dev libfontconfig1-dev libbsd-dev libxrandr-dev libxcursor-dev libgif-dev libavutil-dev libpulse-dev libavformat-dev libavcodec-dev libswresample-dev libdbus-1-dev libxkbfile-dev libssl-dev llvm-dev libvulkan-dev && apt clean -y
RUN apt-get -y install lsb-release && apt clean -y
RUN apt-get -y install devscripts equivs debhelper && apt clean -y
COPY control /control
RUN mk-build-deps -i -r -t "apt-get --no-install-recommends -y" /control && apt clean -y
RUN rm /control
RUN apt-get -y install ccache && apt clean -y
RUN apt-get -y install wget gpg ca-certificates && apt clean -y

RUN if [ "$(clang --version | head -n 1 | cut -d ' ' -f 3 | cut -d '.' -f 1)" -lt 11 ]; then \
      wget -O - https://apt.llvm.org/llvm-snapshot.gpg.key | apt-key add - && \
      echo "deb http://apt.llvm.org/$(lsb_release -cs)/ llvm-toolchain-$(lsb_release -cs) main" >> /etc/apt/sources.list.d/llvm.list && \
      echo "deb-src http://apt.llvm.org/$(lsb_release -cs)/ llvm-toolchain-$(lsb_release -cs) main" >> /etc/apt/sources.list.d/llvm.list && \
      apt-get update; \
    fi

RUN if [ "$(cmake --version | head -n 1 | cut -d ' ' -f 3 | awk '{print $1"\n3.20"}' | sort -Vr | head -n 1)" = "3.20" ]; then \
      wget -O - https://apt.kitware.com/keys/kitware-archive-latest.asc 2>/dev/null | gpg --dearmor - | tee /usr/share/keyrings/kitware-archive-keyring.gpg >/dev/null && \
      echo "deb [signed-by=/usr/share/keyrings/kitware-archive-keyring.gpg] https://apt.kitware.com/ubuntu/ $(lsb_release -cs) main" | tee /etc/apt/sources.list.d/kitware.list >/dev/null && \
      apt-get update; \
    fi

RUN apt-get install -y --only-upgrade clang cmake && apt clean -y

ARG CI_UID=1111
ARG CI_GID=1111
RUN groupadd -g "${CI_GID}" ci || true
RUN useradd -u "${CI_UID}" -g "${CI_GID}" -m ci || true
RUN mkdir -p /ccache
RUN mkdir -p /src/mnt
RUN chown -R ${CI_UID}:${CI_GID} /src
USER ${CI_UID}:${CI_GID}
