# Self-hosted GitHub Actions Runner (Docker, Ubuntu)

Instructions to run a persistent, self-hosted GitHub Actions runner using Docker and Docker Compose.

## Install Docker

- Install [Docker](https://docs.docker.com/get-docker/) 
  ```sh
  sudo apt update
  sudo apt install -y ca-certificates curl gnupg
  sudo install -m 0755 -d /etc/apt/keyrings
  curl -fsSL https://download.docker.com/linux/ubuntu/gpg | sudo gpg --dearmor -o /etc/apt/keyrings/docker.gpg
  echo \
    "deb [arch=$(dpkg --print-architecture) signed-by=/etc/apt/keyrings/docker.gpg] https://download.docker.com/linux/ubuntu \
    $(lsb_release -cs) stable" | \
    sudo tee /etc/apt/sources.list.d/docker.list > /dev/null
  sudo apt update
  sudo apt install -y docker-ce docker-ce-cli containerd.io
  ```
- Enable and start Docker
  ```sh
  sudo systemctl enable docker
  sudo systemctl start docker
  ```
- Run Docker as non-root user
  ```sh
  sudo usermod -aG docker $USER
  # Log out and log back in for this to take effect
  ```
- Install [Docker Compose](https://docs.docker.com/compose/install/)
  ```sh
  sudo apt install -y docker-compose-plugin
  ```

## Setup instructions

- Build the Docker Image. From this `runner/` directory, run:
  ```sh
  # For X86_64 Linux
  docker compose build --build-arg ARCH=x64

  # For Arm64 Linux
  docker compose build --build-arg ARCH=arm64
  ```

- Register the Runner
  - Go to this project's GitHub repository: **Settings → Actions → Runners → New self-hosted runner**  
   Select Linux, x64, and copy the registration command (with repo URL and token).
  - Run the following to open a shell in the container:
    ```sh
    docker compose run --rm gha-runner /bin/bash
    ```
  - Inside the container, register the runner (replace with your actual values):
    ```sh
    ./config.sh --url https://github.com/cvilas/grape --token YOUR_TOKEN
    ```
    Follow the prompts to finish registration, then exit the container shell.

- Start the runner in the background (detached mode):
  ```sh
  docker compose up -d
  ```

- Enable Auto-Start on Boot: The `restart: unless-stopped` policy in `docker-compose.yml` ensures the runner container will automatically start on system boot. If you ever reboot your machine, Docker will restart the runner container automatically.

- To stop and remove the Runner
  ```sh
  docker compose down
  ```

## Notes

- Runner configuration and job data are persisted in Docker volumes (`gha-runner-config`, `gha-runner-work`).
- To unregister the runner, use `./config.sh remove` inside the container.
- You can update the runner version by editing the `Dockerfile`.

## Troubleshooting

- Follow logs:  
  ```sh
  docker compose logs -f
  ```
- If you change the repository or token, re-run the registration step.
- To remove all docker images, containers, volumes and configs by brute force
  ```sh
  sudo systemctl stop docker
  sudo rm -rf /var/lib/docker
  sudo systemctl start docker
  ```

## References

- [GitHub Actions: Adding self-hosted runners](https://docs.github.com/en/actions/hosting-your-own-runners/adding-self-hosted-runners)
- [Docker Compose documentation](https://docs.docker.com/compose/)
