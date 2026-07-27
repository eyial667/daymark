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
- Create long-term goals, break them into dated milestones, track their progress,
  and keep achieved goals in local history.
- Export tasks, completed history, goals, milestones, and preferences to a
  portable `.daymark.json` file, then merge it safely on another computer.
- Open the local application-data folder directly from Settings.
- Navigate the initial desktop application shell.

The working product name is **Daymark** and can be changed before the first
public release.

## Install a packaged build

Daymark does not publish signed releases yet. The current packages are unsigned
development snapshots attached to successful runs of the
[CI workflow](https://github.com/eyial667/daymark/actions/workflows/ci.yml).
Open the latest successful run on `main`, scroll to **Artifacts**, and download
the package for your operating system. GitHub wraps each package in an artifact
ZIP, so extract that outer ZIP first.

### Linux

The Linux artifact contains `Daymark-0.1.0-Linux.tar.gz`. It is currently a
host-compatible package rather than a distribution-independent AppImage, so the
system must provide a compatible Qt 6 runtime, including Qt Quick, Qt Quick
Controls, the SQLite driver, and the Wayland or XCB platform plugin used by the
desktop session.

Install it for the current user without administrator privileges:

```bash
unzip daymark-linux.zip
mkdir -p daymark-package
tar --strip-components=1 -xzf Daymark-0.1.0-Linux.tar.gz \
  -C daymark-package
./daymark-package/install.sh
```

This installs Daymark under `~/.local/opt/daymark`, creates the terminal command
`~/.local/bin/daymark`, and adds application-menu and desktop entries. Launch it
from the application menu or a terminal:

```bash
daymark
```

If `~/.local/bin` is not on `PATH`, launch it with
`~/.local/bin/daymark` instead.

### macOS

Extract `daymark-macos.zip`, then open `Daymark-0.1.0-Darwin.dmg`. Drag
**Daymark** into the **Applications** folder and eject the disk image.

Launch Daymark from Applications, Spotlight, or Terminal:

```bash
open -a Daymark
```

The direct executable is
`/Applications/Daymark.app/Contents/MacOS/daymark`. Because current CI snapshots
are not signed or notarized, macOS may ask for confirmation before the first
launch.

### Windows

Extract `daymark-windows.zip`, then extract the contained
`Daymark-0.1.0-win64.zip`. Move or rename the resulting
`Daymark-0.1.0-win64` directory to a permanent location such as
`%LOCALAPPDATA%\Programs\Daymark`. Keep the complete directory together because
it contains the Qt runtime and plugins required by Daymark.

Launch `bin\daymark.exe` from that directory. From PowerShell, the suggested
location can be launched with:

```powershell
& "$env:LOCALAPPDATA\Programs\Daymark\bin\daymark.exe"
```

The Windows package is currently portable: it does not modify the registry or
create Start-menu and terminal shortcuts. Windows may display a SmartScreen
warning because the development snapshot is not code-signed.

Package filenames above use the current `0.1.0` version; substitute the version
shown in a newer artifact when it changes.

## Build from source

### Requirements

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

### Configure, build, test, and launch

```bash
cmake --preset dev
cmake --build --preset dev
cmake --build --preset dev --target daymark_qmllint
ctest --preset dev --output-on-failure
./build/dev/daymark
```

The last command launches the development build on Linux. On macOS, use
`open build/dev/daymark.app`. On Windows, run `build\dev\daymark.exe` from
PowerShell or File Explorer.

The SQLite database is created in the operating system's standard local
application-data directory. It is never committed to the repository.

To move Daymark data to another computer, open **Settings → Local data &
privacy**, select **Export**, and copy the resulting `.daymark.json` file to the
other computer. Install Daymark there, choose **Import**, select the file, and
confirm **Import and merge**. Existing records on the destination computer are
kept. The versioned file structure is documented in
[docs/data-format.md](docs/data-format.md).

Use **Goals** in the sidebar to create long-term outcomes and add their smaller
milestones. Use the **Interface** selector at the bottom of the sidebar to change
the Today layout. `Ctrl+1`, `Ctrl+2`, `Ctrl+3`, and `Ctrl+4` switch directly
between the four interfaces; `Ctrl+N` opens task capture, `Ctrl+,` opens
Settings, and `Ctrl+Shift+L` toggles light or dark mode.

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

Generated packages are written to `build/packages`. Follow the matching Linux,
macOS, or Windows installation instructions above; locally generated packages
do not have GitHub's additional artifact ZIP wrapper.

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
