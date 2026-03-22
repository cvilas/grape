# Copilot Instructions for `grape`

- Follow the [README](https://github.com/cvilas/grape/blob/main/README.md) for high-level context for this repo.
- Canonical coding guidelines live in [`cvilas/guidance`](https://github.com/cvilas/guidance).

This file is a **distilled, actionable summary** for day-to-day use.
When deeper context is needed, reference the linked documents directly (e.g. via `#file` in Copilot Chat).

---

## C++ coding rules

- Follow [CppCoreGuidelines](https://github.com/isocpp/CppCoreGuidelines); enforce via `.clang-tidy`.
- Follow coding style defined by [`.clang-format`](https://github.com/cvilas/grape/blob/main/.clang-format) file
- Prefer the latest C++ standard for language features and performance
- Prefer functional programming techniques; minimise coupling and dependencies
- Follow DRY and SOLID principles
- Follow UNIX philosophy of programs that (a) do one thing and do it well and (b) work well together by composition
- Use SI units in public interfaces
- Propose code that is **correct by construction** and therefore, hard to misuse ([full guide](https://github.com/cvilas/guidance/blob/main/process/correct_by_construction.md)):
  - Prefer `const`, `consteval`, `constexpr`; use `enum class` over bare enums or `bool` flags
  - Enforce `const` correctness
  - Use strong/named types to clarify semantics; mark single-argument constructors `explicit`
  - Use `static_assert` for compile-time checks; prefer `switch` with no `default` over `if/else` chains
  - Use RAII everywhere (Constructor Acquires, Destructor Releases)
  - Use contracts to protect invariants in code (`contract_assert`, `pre(condition)`, `post(r: condition)`)

---

## Project structure

Organise code into self-contained **modules** under `modules/`. Each module contains:
`src/` · `include/` · `tests/` · `examples/` (minimum); optionally `docs/` · `scripts/` · `apps/`

---

## Documentation

- Colocate design docs with code (in `docs/` inside the relevant module)
- Document *why*, not just *what*; include assumptions, inputs/outputs, failure modes
- Document all non-obvious APIs with Doxygen; include valid parameter ranges
- Prefer `md`/`txt` formats
- Cite sources
- Be succinct. Verbosity hurts readability and maintainability

---

## Git workflow

Follow the branch-and-merge model ([developer guide](https://github.com/cvilas/guidance/blob/main/process/developer_guide.md)):
- Branch from `main`; tidy history with `git rebase` before raising a PR
- Follow [commit message guidelines](https://github.com/cvilas/guidance/blob/main/process/commit_messages.md)
- Follow [code review guidelines](https://github.com/cvilas/guidance/blob/main/process/code_reviews.md)

---
