# Daymark

[![CI](https://github.com/eyial667/daymark/actions/workflows/ci.yml/badge.svg)](https://github.com/eyial667/daymark/actions/workflows/ci.yml)

Daymark is a local-first, cross-platform daily command center. It turns tasks into
an explainable priority queue and will grow into a connected view of goals,
projects, dependencies, calendar commitments, focus sessions, and reviews.

Daymark is a native Qt application. It does not use HTML, a browser engine, an
embedded webview, Node.js, or a local web server.

## Current vertical slice

- Create tasks with a due date, importance, estimate, and project.
- Store all data locally in SQLite.
- Rank open tasks with a deterministic priority engine.
- Explain every priority score.
- Complete tasks from the Today queue.
- Switch between four interfaces, each with coordinated light and dark palettes,
  without changing task data: Midnight Command, Spatial Map, Quiet Focus, and
  Daymark Hybrid.
- Keep the selected interface across application launches.
- Configure light or dark mode, accent color, daily capacity, task-capture
  defaults, clock format, priority explanations, and completion confirmation
  from a native Settings page.
- Open the local application-data folder directly from Settings.
- Navigate the initial desktop application shell.

The working product name is **Daymark** and can be changed before the first
public release.

## Build requirements

- A C++20 compiler
- CMake 3.21 or newer
- Ninja
- Qt 6.5 or newer with Core, GUI, Quick, Quick Controls, SQL, and Test modules
- The Qt SQLite driver

### Fedora

```bash
sudo dnf install gcc-c++ cmake ninja-build \
  qt6-qtbase-devel qt6-qtdeclarative-devel qt6-qttools-devel sqlite-devel
```

### Ubuntu and Debian

```bash
sudo apt install build-essential cmake ninja-build \
  qt6-base-dev qt6-declarative-dev qt6-tools-dev libgl1-mesa-dev
```

On macOS and Windows, install Qt 6 through the open-source Qt installer and use
the platform compiler supplied by Xcode or Visual Studio respectively.

## Build and run

```bash
cmake --preset dev
cmake --build --preset dev
ctest --preset dev
./build/dev/daymark
```

On macOS, run `build/dev/daymark.app`. On Windows, run
`build\\dev\\daymark.exe`.

The SQLite database is created in the operating system's standard local
application-data directory. It is never committed to the repository.

Use the **Interface** selector at the bottom of the sidebar to change the Today
layout. `Ctrl+1`, `Ctrl+2`, `Ctrl+3`, and `Ctrl+4` switch directly between the
four interfaces; `Ctrl+N` opens task capture, `Ctrl+,` opens Settings, and
`Ctrl+Shift+L` toggles light or dark mode.

## Create a package

```bash
cmake --preset release
cmake --build --preset release
cpack --config build/release/CPackConfig.cmake -B build/packages
```

On Linux this currently creates a host-compatible `.tar.gz` with the executable,
installer, desktop assets, and license texts. It deliberately uses the host
distribution's complete Qt runtime, including its Wayland, GLX, EGL, and SQLite
plugins. The distribution-independent Linux release will be an AppImage produced
in an older Linux CI container so its glibc baseline works across supported
distributions.

To install the generated archive for the current Linux user:

```bash
mkdir daymark-package
tar --strip-components=1 -xzf build/packages/Daymark-0.1.0-Linux.tar.gz \
  -C daymark-package
./daymark-package/install.sh
```

The installer requires no administrator privileges. It creates:

- `~/.local/opt/daymark` for the application files
- `~/.local/bin/daymark` for terminal launches
- an application-menu entry and scalable application icon
- a `Daymark.desktop` shortcut in the user's configured desktop directory

## Continuous integration

GitHub Actions builds, lints, tests, packages, and uploads short-lived build
artifacts for Linux, Windows, and macOS on every push to `main` and every pull
request. The workflow uses read-only repository permissions and immutable action
revisions.

## Architecture

The code is split into four boundaries:

- `domain`: durable application concepts such as tasks.
- `core`: deterministic business rules, including priority scoring.
- `data`: SQLite persistence and migrations.
- `presentation`: Qt models exposed to the QML desktop interface.

See [docs/architecture.md](docs/architecture.md) for the decisions and
constraints behind this structure.

## Contributing

Contributions are welcome. Start with [CONTRIBUTING.md](CONTRIBUTING.md) and the
repository instructions in [AGENTS.md](AGENTS.md). Security-sensitive findings
should follow [SECURITY.md](SECURITY.md).

Notable user-facing changes are tracked in [CHANGELOG.md](CHANGELOG.md).

## License

Daymark is free software licensed under
[GPL-3.0-or-later](LICENSE). Contributions must be compatible with that license.
