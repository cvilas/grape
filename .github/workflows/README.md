# CI workflow settings

## Protect 'main' branch

- Go to repository on GitHub.
- Click on *Settings* (top menu) -> *Branches* (sidebar)
- Under "Branch protection rules," click Add rule.
  - Enter the branch name pattern (e.g., main).
  - Select the protection options you want, such as:
    - Require a pull request before merging
    - Require approvals
    - Require status checks to pass
- Click Create or Save changes.

## Enable "Require approval for all external contributors "

- Go to repository on GitHub. 
- Click on *Settings* (top menu) → *Actions* (left sidebar) → *General*.
- Scroll down to the "Approval for running fork pull request workflows from contributors" section.
- Find the option: "Require approval for all outside collaborators" (or similar wording).
- Check the box to enable it.
- Click Save if prompted.

## Semantic versioning (release-please)

Versioning follows [Semantic Versioning](https://semver.org) driven by 
[Conventional Commits](https://www.conventionalcommits.org) and automated via
[release-please](https://github.com/googleapis/release-please). How it works:

- Follow the Conventional Commits format for every commit in the PR (enforced by `commit-lint.yml`):
  - `fix: …` → patch bump (0.1.x → 0.1.x+1)
  - `feat: …` → minor bump (0.x.0 → 0.x+1.0)
  - `feat!: …` or footer `BREAKING CHANGE: …` → major bump (x.0.0 → x+1.0.0)
  - `chore:`, `docs:`, `ci:`, etc. → no version bump
- On each merge to `main`, `release-please.yml` opens or updates a Release PR that contains the computed next version and a generated `CHANGELOG.md` entry.
- Merging the Release PR creates the `vX.Y.Z` git tag and a GitHub Release.

The `CHANGELOG.md` and the release-please files under [.github/release-please](../release-please) are used by the release automation. The manifest is maintained automatically by release-please — do not edit it by hand.

## TODO

- Add cross-builds (presets: `llvm-cross`, `gcc-cross`)
- Add a few sanitizer builds
- Separate [RTSan](https://clang.llvm.org/docs/RealtimeSanitizer.html) build on realtime module
