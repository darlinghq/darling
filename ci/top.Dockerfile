FROM alpine:latest
LABEL name=darling-build-image-top version=0.1.0
ARG DOCKER_GID=975
RUN addgroup -g ${DOCKER_GID} -S docker
RUN apk add docker git git-lfs sed
RUN addgroup -g 1001 ci
RUN adduser -u 1001 -G ci --disabled-password ci
RUN addgroup ci docker
USER ci
