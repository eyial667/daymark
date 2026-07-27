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
| `settings` | object | Portable appearance, planning, and behavior preferences. |

Task records contain their stable ID, title, notes, optional category and
subcategory IDs, optional `plannedDate`, due and creation timestamps, completion
state and timestamp, importance, and estimate. Category and nested subcategory
records contain a stable ID, name, notes, and creation timestamp. The `project`
task field remains as an empty compatibility field for older Daymark builds; old
imports with a project value convert it into a category. Goal and milestone
records contain stable IDs, titles, descriptions where applicable, target dates,
creation timestamps, and completion state.

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
