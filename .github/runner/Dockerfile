# Dockerfile for GitHub Actions self-hosted runner

FROM ubuntu:24.04

ARG ARCH=x64 # CPU architecture (x64, arm64)

RUN apt-get update && \
    apt-get install -y curl sudo gosu git git-lfs libicu74 && \
    useradd -m runner && \
    echo "runner ALL=(ALL) NOPASSWD:ALL" >> /etc/sudoers

WORKDIR /home/runner

# Install GitHub Actions runner (update version as needed)
ENV RUNNER_VERSION=2.327.1
ENV RUNNER_ARCH=${ARCH}
ENV RUNNER_TARBALL=actions-runner-linux-${RUNNER_ARCH}-${RUNNER_VERSION}.tar.gz

RUN curl -o ${RUNNER_TARBALL} -L https://github.com/actions/runner/releases/download/v${RUNNER_VERSION}/${RUNNER_TARBALL} && \
    tar xzf ./${RUNNER_TARBALL} && \
    rm ${RUNNER_TARBALL} && \
    ./bin/installdependencies.sh

COPY entrypoint.sh /entrypoint.sh
RUN chmod +x /entrypoint.sh

ENTRYPOINT ["/entrypoint.sh"]

# run the github runner script
CMD ["./run.sh"]
