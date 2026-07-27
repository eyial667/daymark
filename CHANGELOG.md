# Changelog

All notable changes to Daymark will be documented here.

The format follows [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and releases will use semantic versioning.

## [Unreleased]

### Added

- Native Qt 6 application shell for Linux, macOS, and Windows.
- Local SQLite task storage and explainable priority ordering.
- Four switchable Today interfaces.
- Persistent light and dark color modes with configurable accents.
- Application settings for planning defaults and behavior.
- Per-user Linux installer, terminal launcher, menu entry, and desktop shortcut.
- Cross-platform GitHub Actions build, lint, test, package, and artifact pipeline.
- Long-term goals with descriptions, target dates, progress, and smaller dated
  milestones.
- Versioned portable data export and merge import for tasks, completed history,
  goals, milestones, and preferences.
- Reusable task categories with notes, task-row reassignment, and portable
  category assignments.
- One-level category nesting with subcategory notes and hierarchical task
  assignment.
- A dedicated To-do page for the complete open-task queue.
- Per-user Windows installation wizard with Start-menu integration, an optional
  desktop shortcut, and uninstall support.
- Empty-day planning that can pull the highest-ranked To-do task into Today or
  create today's work from the earliest unfinished goal milestone.
- A separate local planning date for Today, independent of task deadlines.
- An optional, persistent Today timeline setting with a list-first default.
- An always-available “Give me something to do” chooser for To-do tasks, work
  areas, and goal milestones.
- Deterministic category and subcategory work-area proposals based on open tasks.

### Changed

- Retired the incomplete Projects surface. Existing project labels migrate to
  categories so previously entered organization is preserved.
- Enlarged task-estimate input and duration labels for better readability across
  capture, queues, and compact timelines.
- Added a themed calendar picker alongside manual `YYYY-MM-DD` entry for task,
  goal, and milestone dates.
- New task capture now defaults to To-do; tasks still enter Today automatically
  when their deadline arrives or becomes overdue.
- Replaced inert Today controls with working navigation and added click, focus,
  success, and validation feedback throughout task interactions.
