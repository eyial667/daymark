# Architecture

## Non-negotiable constraints

- Daymark is an installed desktop application, not a web application.
- The production application must run without a browser or local server.
- Core task management works fully offline.
- User data is stored locally and remains exportable.
- Priority recommendations are explainable and user-overridable.
- macOS, Windows, and Linux share one product codebase.

## Runtime boundaries

The QML layer owns presentation and interaction only. C++ owns validation,
persistence, priority decisions, integration coordination, and operating-system
services. QML must not contain business rules or direct SQL access.

`TaskListModel` is the current application boundary. It converts persisted tasks
into a sorted, read-only queue for the interface and translates user actions
into repository operations.

## Persistence

SQLite is the source of truth. Schema changes use SQLite's `user_version` value
and transactional migrations. Dates are stored as UTC ISO-8601 strings and
converted to local time at the presentation boundary.

Future sync integrations must operate through an outbox rather than mutating UI
state directly. Local edits remain usable while integrations are offline.

## Priority scoring

The first engine intentionally uses a transparent weighted score:

- Importance: 10–50 points
- Overdue: 55 points
- Due today: 45 points
- Due tomorrow: 28 points
- Due within three days: 18 points
- Due within seven days: 8 points
- Waiting age: up to 14 points
- Short focus fit: up to 6 points

The exact weights will evolve, but every added rule must emit a human-readable
reason and have deterministic tests. Optional AI may propose plans later; it
will not silently replace the deterministic engine.

## Planned boundaries

- Calendar adapters import events into a separate calendar domain.
- A graph service owns goals, projects, dependencies, and mind-map layout.
- Platform services wrap notifications, autostart, tray, and credential storage.
- Export and backup operate on documented, versioned formats.
