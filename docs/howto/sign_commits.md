# How to sign GitHub commits

- Enable ['vigilant mode'](https://docs.github.com/en/authentication/managing-commit-signature-verification/displaying-verification-statuses-for-all-of-your-commits) in GitHub

- [Add your ssh 'public' key](https://docs.github.com/en/authentication/connecting-to-github-with-ssh/adding-a-new-ssh-key-to-your-github-account) to GitHub. You need to do it twice. Once as 'authentication key' and once as 'signing key'

- Configure git to use your SSH key for signing

  ```
  # Note: To set this only on a specific git repository and not all of them, 
  # remove '--global' and run the commands within that repository
  
  # use ssh key to sign
  git config --global gpg.format ssh
  
  # specify path to ssh key
  git config --global user.signingkey /path/to/your/ssh/key.pub
  
  # Always sign every commit by default
  git config --global commit.gpgsign true
  
  # Always sign every tag by default
  git config --global tag.gpgsign true
  
  # Set file that specifies who are allowed to sign
  git config --global gpg.ssh.allowedSignersFile ~/.ssh/allowed_signers
  ```

- Create `~/.ssh/allowed signers` file, and add a line for your SSH key. The format is:
  ```
  <GitHub username> <public key type> <public key data>
  ```
  For instance, my file looks like this:
  ```
  cvilas ssh-ed25519 AXyB..rest of the key..
  ```

- To verify whether commits are signed properly, 
  ```
  git log --show-signature
  ```
