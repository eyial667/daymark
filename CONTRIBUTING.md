# Contributing to Daymark

Thanks for helping build Daymark. Contributions should preserve its core
qualities: native, local-first, explainable, cross-platform, and accessible.

## Before you start

Read [AGENTS.md](AGENTS.md) for repository-specific engineering rules and
[docs/architecture.md](docs/architecture.md) for the current boundaries.

You need a C++20 compiler, CMake 3.21+, Ninja, Qt 6.5+ with Core, GUI, Quick,
Quick Controls, SQL, and Test modules, plus the Qt SQLite driver.

## Development workflow

1. Create a focused branch from `main`.
2. Make the smallest coherent change that solves the problem.
3. Add or update tests for business logic, persistence, and settings behavior.
4. Verify the native UI in both light and dark modes when presentation changes.
5. Run:

   ```bash
   cmake --preset dev
   cmake --build --preset dev
   cmake --build --preset dev --target daymark_qmllint
   ctest --preset dev --output-on-failure
   ```

6. Update documentation when commands, behavior, or architecture changes.

## Code and commits

- Add `SPDX-License-Identifier: GPL-3.0-or-later` to new source files.
- Follow the surrounding C++ and QML style.
- Use short, imperative commit subjects.
- Avoid committing build output, local databases, editor state, or generated
  packages.

By contributing, you agree that your contribution is licensed under
GPL-3.0-or-later.
