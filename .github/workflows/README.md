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

## TODO
- Track regression from baseline for benchmarks. Fail CI if regression > x%
- Setup configuration presets for applications, developer and CI builds
- After PR is merged and newly tagged, build and generate deployment artifacts (tarball)
- Add a few sanitizer builds 
- Separate [RTSan](https://clang.llvm.org/docs/RealtimeSanitizer.html) build on realtime module 