# Daymark portable data format

Daymark exports a UTF-8 JSON document with the `.daymark.json` suffix. The
format is intentionally readable, versioned, and independent of the operating
system's SQLite and settings locations.

## Top-level object

| Field | Type | Meaning |
| --- | --- | --- |
| `format` | string | Always `org.daymark.backup`. |
| `version` | integer | Currently `1`. |
| `exportedAt` | ISO-8601 timestamp | Time the export was created. |
| `applicationVersion` | string | Daymark version that created the export. |
| `tasks` | array | Open and completed task records. |
| `goals` | array | Active and achieved goals with nested milestones. |
| `settings` | object | Portable appearance, planning, and behavior preferences. |

Task records contain their stable ID, title, notes, project, due and creation
timestamps, completion state and timestamp, importance, and estimate. Goal and
milestone records contain stable IDs, titles, descriptions where applicable,
target dates, creation timestamps, and completion state.

Timestamps are written in UTC with ISO-8601 millisecond precision. Goal and
milestone targets are calendar dates in `YYYY-MM-DD` form and do not carry a
timezone. An empty date or timestamp string means that value was not set.

## Import behavior

Daymark validates the complete document and its supported ranges before writing
anything. Import then performs one SQLite transaction:

- matching stable IDs are updated from the export;
- unrelated records already on the destination computer are retained;
- milestones remain attached to their exported goal;
- preferences are applied only after the database transaction succeeds.

Unknown format versions are rejected so a newer Daymark export cannot be
silently misread by an older application. Keep the original export as a backup
until the destination computer has been checked.
