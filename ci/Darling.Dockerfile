# Darling - macOS runtime environment for Linux
# https://github.com/darlinghq/darling
#
# This image contains a pre-built Darling installation.
#
# IMPORTANT: Darling requires the darling-mach kernel module loaded on the host.
# Without it, `darling shell` will fail with "Failed to open /dev/mach" =[.
#
# Usage:
#   docker run --rm --privileged --cap-add=SYS_ADMIN \
#     -v /dev/dxg:/dev/dxg \
#     darlinghq/darling darling shell echo "Hello from macOS runtime"

FROM ubuntu:22.04
LABEL maintainer="Darling Project <https://darlinghq.org>" \
      name="darlinghq/darling" \
      description="Darling - macOS runtime environment for Linux" \
      url="https://github.com/darlinghq/darling"

ARG DEBIAN_FRONTEND=noninteractive

# Install build dependencies
RUN apt-get update && apt-get install -y \
    cmake \
    clang \
    bison \
    flex \
    libfuse-dev \
    libudev-dev \
    pkg-config \
    libc6-dev-i386 \
    linux-headers-generic \
    gcc-multilib \
    libcairo2-dev \
    libgl1-mesa-dev \
    libglu1-mesa-dev \
    libtiff5-dev \
    libfreetype6-dev \
    git \
    git-lfs \
    libelf-dev \
    libxml2-dev \
    libegl1-mesa-dev \
    libfontconfig1-dev \
    libbsd-dev \
    libxrandr-dev \
    libxcursor-dev \
    libgif-dev \
    libavutil-dev \
    libpulse-dev \
    libavformat-dev \
    libavcodec-dev \
    libswresample-dev \
    libdbus-1-dev \
    libxkbfile-dev \
    libssl-dev \
    llvm-dev \
    libvulkan-dev \
    ninja-build \
    libc++-dev \
    libc++abi-dev \
    libcap2-bin \
    python3 \
    && apt-get clean && rm -rf /var/lib/apt/lists/*

WORKDIR /src/darling
COPY . .

# Initialize submodules from the already-copied .git data (no network fetch needed)
RUN git submodule update --init --recursive --no-fetch 2>/dev/null || \
    git submodule update --init --recursive

# Build and install
#
# Key cmake flags:
# - CMAKE_C_FLAGS_INIT: Appended to internal flags. Using CMAKE_C_FLAGS would 
#   overwrite required warning suppressions like -Wno-nullability-completeness.
# - TARGET_i386=OFF: Skip 32-bit slices to avoid noisy linker failures and ranlib warnings.
RUN mkdir -p build && cd build && \
    cmake .. \
        -DCMAKE_BUILD_TYPE=RelWithDebInfo \
        -DCMAKE_C_COMPILER=clang \
        -DCMAKE_CXX_COMPILER=clang++ \
        -DCMAKE_C_FLAGS_INIT="-Wno-incompatible-pointer-types" \
        -DCMAKE_CXX_FLAGS_INIT="-Wno-incompatible-pointer-types" \
        -DTARGET_i386=OFF \
        -DDARLING_NO_DSYM=ON \
        -GNinja && \
    ninja -j$(nproc) && \
    ninja install && \

    cd / && rm -rf /src/darling/build

WORKDIR /root

CMD ["darling", "shell"]
