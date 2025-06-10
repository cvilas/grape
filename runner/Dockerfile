# Dockerfile for GitHub Actions self-hosted runner

FROM ubuntu:24.04

RUN apt-get update && \
    apt-get install -y curl sudo git libicu74 && \
    useradd -m runner && \
    echo "runner ALL=(ALL) NOPASSWD:ALL" >> /etc/sudoers

USER root
WORKDIR /home/runner

COPY entrypoint.sh /entrypoint.sh
RUN chmod +x /entrypoint.sh

USER runner

# Install GitHub Actions runner (update version as needed)
RUN curl -o actions-runner-linux-x64-2.325.0.tar.gz -L https://github.com/actions/runner/releases/download/v2.325.0/actions-runner-linux-x64-2.325.0.tar.gz && \
    tar xzf ./actions-runner-linux-x64-2.325.0.tar.gz && \
    rm actions-runner-linux-x64-2.325.0.tar.gz

ENTRYPOINT ["/entrypoint.sh"]
CMD ["./run.sh"]