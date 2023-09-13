FROM alpine:latest
LABEL name=darling-build-image-top version=0.1.0
ARG DOCKER_GID=109
ARG CI_UID=1111
ARG CI_GID=1111
RUN addgroup -g ${DOCKER_GID} -S docker
RUN apk add docker git git-lfs sed
RUN addgroup -g "${CI_GID}" ci
RUN adduser -u "${CI_UID}" -G ci --disabled-password ci
RUN addgroup ci docker
USER ci
