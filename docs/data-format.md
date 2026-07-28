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
| `categories` | array | Reusable task categories with nested subcategories and notes. |
| `goals` | array | Active and achieved goals with nested milestones. |
| `meetings` | array | Dated meetings and their durable reminder state. |
| `mentalMap` | object | Persistent cloud and constellation groups, notes, and directed connections. |
| `settings` | object | Portable appearance, planning, and behavior preferences. |

Task records contain their stable ID, title, notes, optional category and
subcategory IDs, optional `plannedDate`, due and creation timestamps, completion
state and timestamp, importance, and estimate. Category and nested subcategory
records contain a stable ID, name, notes, and creation timestamp. The `project`
task field remains as an empty compatibility field for older Daymark builds; old
imports with a project value convert it into a category. Goal and milestone
records contain stable IDs, titles, descriptions where applicable, target dates,
creation timestamps, and completion state.
Meeting records contain a stable ID, title, notes, required start and creation
timestamps, and an optional notification timestamp.
The mental-map object contains `groups`, `notes`, and `connections` arrays.
Groups retain their view kind, title, color, map-space rectangle, and creation
time. Notes retain their group, position, body, color, tags, external link,
priority, checklist, central-idea state, and optional linked task ID. Connections
retain their source, destination, optional label, and direction.

Settings include the selected interface, language, and palette, planning and capture
defaults, and behavior preferences such as timeline visibility. The
`showTimeline` and `language` settings are optional for compatibility with
earlier version 1 exports and default to `false` and English respectively when
absent.

Rolling dashboard day notes are intentionally absent from this format. They are
retained only for today and yesterday in the local database, then pruned, so an
export cannot turn them into a permanent journal.

Timestamps are written in UTC with ISO-8601 millisecond precision. Task planning
dates and goal and milestone targets are calendar dates in `YYYY-MM-DD` form and
do not carry a timezone. An empty date or timestamp string means that value was
not set.

## Import behavior

Daymark validates the complete document and its supported ranges before writing
anything. Import then performs one SQLite transaction:

- matching stable IDs are updated from the export;
- unrelated records already on the destination computer are retained;
- category and subcategory records are merged before tasks so assignments remain
  intact;
- milestones remain attached to their exported goal;
- meetings retain their start time and whether their reminder was already sent;
- map groups are merged before their notes and directed connections, while linked
  task IDs continue to refer to the imported task records;
- preferences are applied only after the database transaction succeeds.

Unknown format versions are rejected so a newer Daymark export cannot be
silently misread by an older application. Keep the original export as a backup
until the destination computer has been checked.

The `categories` list and each task's `categoryId` and `subcategoryId` were added
as backward-compatible optional fields in format version 1. Imports created by
earlier Daymark builds therefore remain valid; legacy project labels become
categories automatically. A task's `plannedDate` is also optional in version 1,
so backups from builds that predate Today planning remain valid and import those
tasks into To-do without assigning them to a day.
The top-level `meetings` list is optional for the same compatibility reason;
older version 1 backups import with an empty meeting list.
The top-level `mentalMap` object is also optional in version 1, so older backups
continue to import with an empty brainstorming map.
