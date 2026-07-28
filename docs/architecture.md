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
to propose the best remaining backlog item on request. Any task in the full
To-do queue can also be planned directly by its stable ID. New task capture
leaves the planning date empty unless the user explicitly adds the task to Today.
`CategoryListModel` owns reusable category metadata and notes and derives a
deterministic work-area proposal from open task counts, while `GoalListModel`
owns goal and milestone presentation state and deterministically proposes the
earliest unfinished milestone. `DataTransferService` coordinates validated
portable exports and imports. `TaskListModel` also exposes a completed-today
summary for review.
`FocusSessionModel` owns the deterministic countdown state used by the Focus
screen; it does not persist or mutate tasks. `MentalMapModel` validates and
persists user-authored brainstorming groups, short notes, metadata, checklists,
positions, and directed connections. It also creates task links without coupling
map-note lifetime to task lifetime: deleting a task unlinks its note, while
deleting a note never deletes its task.
`DailyNoteModel` debounces local note saves and exposes only today and yesterday
to QML. These objects translate user actions into repository operations without
exposing SQL to QML.

`MeetingListModel` validates and presents locally stored meetings.
`MeetingReminderService` is the single reminder scheduler: while Daymark is
running, it wakes at the next ten-minute boundary, atomically records a meeting
as notified, and emits one delivery request. `DesktopNotificationService`
delivers that request through the freedesktop notification service on Linux and
the native system-tray notification facility on Windows and macOS. Recording
the reminder before delivery prevents duplicate notifications across timer
wakeups and application restarts.

`UpdateService` is the single network and installer boundary. It performs a
non-blocking startup request for a platform-specific manifest attached to the
latest public GitHub Release. This avoids unauthenticated REST API quotas while
still accepting only a strictly newer semantic version and the exact package
name for the running platform. It streams the package into the application cache
and checks the manifest's SHA-256 digest before any operating-system installer
handoff. Downloads and installation are always explicit user actions. Update
failures do not affect the database or offline task workflows.

`LanguageManager` installs the bundled French Qt translation catalog before
models are constructed and retranslates the QML engine immediately when the
persistent language setting changes. Model reloads regenerate translated dates,
priority reasons, map details, and validation feedback without changing stored
task content.

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
Meetings store their start, creation, and optional notification timestamps as
UTC ISO-8601 values and convert them back to local time for display and
scheduling. Reminder state is durable so each meeting is claimed at most once.

The mental map uses separate group, note, and directed-connection tables. Group
and note positions use map-space coordinates so presentation pan and zoom do not
alter stored layout. Deleting a group cascades through its contained notes and
their connections. A note's optional task foreign key uses `ON DELETE SET NULL`,
preserving the brainstorm record when its task is removed.

Daily notes use a separate date-keyed table. On startup and across day
boundaries, records older than yesterday are deleted. The current and previous
day are the complete retention window; daily notes are deliberately excluded
from portable exports because they are short-lived dashboard context rather
than durable task or goal data.

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
- A future graph service may extend user-authored map connections into task
  dependencies and blocked-work rules.
- Additional platform services wrap autostart and credential storage.
- Export and backup operate on documented, versioned formats.
