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

`TaskListModel` converts persisted tasks into either the full sorted queue or a
Today-scoped queue. The Today scope includes tasks planned for the current date
and tasks whose deadlines are due or overdue, then uses the same priority engine
to propose the best remaining backlog item on request. New task capture leaves
the planning date empty unless the user explicitly adds the task to Today.
`CategoryListModel` owns reusable category metadata and notes and derives a
deterministic work-area proposal from open task counts, while `GoalListModel`
owns goal and milestone presentation state and deterministically proposes the
earliest unfinished milestone. `DataTransferService` coordinates validated
portable exports and imports. `TaskListModel` also exposes filtered task
snapshots for the native category map and a completed-today summary for review.
`FocusSessionModel` owns the deterministic countdown state used by the Focus
screen; it does not persist or mutate tasks. These objects translate user
actions into repository operations without exposing SQL to QML.

## Persistence

SQLite is the source of truth. Schema changes use SQLite's `user_version` value
and transactional migrations. Timestamps are stored as UTC ISO-8601 strings and
converted to local time at the presentation boundary. Tasks refer to categories
and optional child subcategories through nullable stable IDs; names and notes
are stored once, and assignments are cleared safely if their category is removed
in a future version. A task's nullable planning date is a local calendar date
separate from its deadline; moving work into Today never changes when it is due.
The former free-text project field is migrated into
categories and is no longer part of the product interface. Goal and milestone
target dates are calendar dates stored in ISO-8601 form without a timezone.

Portable data uses the versioned `org.daymark.backup` JSON format. Import fully
validates a file before starting a database transaction, upserts matching record
IDs, and retains unrelated destination records. Preferences are applied only
after the database merge succeeds. See [data-format.md](data-format.md) for the
portable schema and merge contract.

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
- A graph service owns goals, category relationships, dependencies, and mind-map
  layout.
- Platform services wrap notifications, autostart, tray, and credential storage.
- Export and backup operate on documented, versioned formats.
