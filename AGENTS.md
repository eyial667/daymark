# Daymark repository instructions

## Project

Daymark is a local-first native desktop application built with C++20, Qt 6,
Qt Quick/QML, and SQLite. It must remain a real desktop application: do not add
HTML, an embedded browser, Node.js, or a local web server.

The project is free software under GPL-3.0-or-later. New source files should use:

```text
SPDX-License-Identifier: GPL-3.0-or-later
```

## Build and verification

From the repository root:

```bash
cmake --preset dev
cmake --build --preset dev
cmake --build --preset dev --target daymark_qmllint
ctest --preset dev --output-on-failure
```

For a release archive:

```bash
cmake --preset release
cmake --build --preset release
ctest --test-dir build/release --output-on-failure
cpack --config build/release/CPackConfig.cmake -B build/packages
```

Run the development executable with `./build/dev/daymark`.

## Architecture

- `src/domain`: durable application concepts with no UI dependencies.
- `src/core`: deterministic business rules such as priority scoring.
- `src/data`: SQLite persistence and forward-safe migrations.
- `src/presentation`: QObject models and settings exposed to QML.
- `qml`: native Qt Quick presentation.
- `tests`: focused Qt Test coverage for business rules and persistence.
- `packaging`: platform launchers and installers.

Keep business logic out of QML when it can be deterministic C++. Keep database
details out of presentation components.

## UI conventions

- Every screen must work in both light and dark color modes.
- Use `Theme.qml` palette properties instead of hardcoded surface or text
  colors. White is acceptable for text or marks rendered on a known accent fill.
- Keep all four Today interfaces functional: Midnight Command, Spatial Map,
  Quiet Focus, and Daymark Hybrid.
- Reuse `AppButton.qml`, `TaskRow.qml`, and other shared components.
- Use `pragma ComponentBehavior: Bound` for components with delegates that
  access an outer id.
- Preserve keyboard access and visible focus behavior.
- Do not add a setting unless it affects real current behavior.

## Data and privacy

- Tasks and preferences remain on the user's machine.
- Do not add telemetry, accounts, network synchronization, or external API calls
  without an explicit product decision.
- Never delete or reset a user's SQLite database during an upgrade.
- Schema changes require a migration and repository tests.
- Settings keys are persistent API: retain or migrate existing keys when names
  change.

## Change discipline

- Preserve unrelated user changes.
- Keep commits focused and use imperative commit subjects.
- Update README or architecture documentation when behavior or boundaries change.
- A change is complete when it builds, QML lint is clean, relevant tests pass,
  and the native application starts successfully.
