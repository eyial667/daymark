# Graph Report - .  (2026-07-31)

## Corpus Check
- Corpus is ~47,230 words - fits in a single context window. You may not need a graph.

## Summary
- 970 nodes · 2065 edges · 43 communities (42 shown, 1 thin omitted)
- Extraction: 91% EXTRACTED · 9% INFERRED · 0% AMBIGUOUS · INFERRED: 189 edges (avg confidence: 0.83)
- Token cost: 105,112 input · 0 output

## Community Hubs (Navigation)
- Task Repository SQLite Layer
- JSON Export and Import
- Mental Map Model
- Task List Model
- Category List Model
- Goal List Model
- App Settings Keys
- Priority Engine and Domain Headers
- Focus Session Timer
- Meeting List Model
- Language and Meeting Reminders
- Daily Note Model
- Update Service State
- Desktop Notification Service
- Update Release Verification
- Settings QML Properties
- Update Download and Install
- Update Service Tests
- Repository Instructions and Licensing
- CI Build and Packaging
- Changelog and Release History
- Model Test Fixtures
- Four Today Interfaces
- Settings Storage Helpers
- App Settings Tests
- Task List Model Tests
- QML Module and UI Conventions
- Mental Map Persistence Design
- Portable Backup Format
- QML C++ Runtime Boundary
- Data Transfer Tests
- Goal List Model Tests
- Mental Map Model Tests
- Release Workflow Jobs
- Application Icon Design
- Category List Model Tests
- Daily Note Model Tests
- Meeting List Model Tests
- Meeting Reminder Service Tests
- Accent Preset Setting
- Interface Style Setting
- Linux Install Script

## God Nodes (most connected - your core abstractions)
1. `AppSettings` - 75 edges
2. `UpdateService` - 64 edges
3. `TaskRepository` - 54 edges
4. `CategoryListModel` - 51 edges
5. `MentalMapModel` - 50 edges
6. `TaskListModel` - 46 edges
7. `GoalListModel` - 41 edges
8. `assignError()` - 39 edges
9. `FocusSessionModel` - 33 edges
10. `DailyNoteModel` - 31 edges

## Surprising Connections (you probably didn't know these)
- `Daymark Project Instructions (CLAUDE.md)` --semantically_similar_to--> `Daymark Repository Instructions (AGENTS.md)`  [INFERRED] [semantically similar]
  CLAUDE.md → AGENTS.md
- `Slice 5 — Integrations and Distribution` --conceptually_related_to--> `Tag-Driven Release Workflow`  [INFERRED]
  docs/roadmap.md → .github/workflows/release.yml
- `README Architecture Boundary Summary` --semantically_similar_to--> `Four-Layer Source Layout (domain/core/data/presentation)`  [INFERRED] [semantically similar]
  README.md → AGENTS.md
- `Private Vulnerability Reporting Policy` --semantically_similar_to--> `Data and Privacy Rules`  [INFERRED] [semantically similar]
  SECURITY.md → AGENTS.md
- `Contributor Development Workflow` --semantically_similar_to--> `Build and Verification Command Sequence`  [INFERRED] [semantically similar]
  CONTRIBUTING.md → AGENTS.md

## Import Cycles
- None detected.

## Hyperedges (group relationships)
- **QObject Models Exposed to QML Without SQL Leakage** — docs_architecture_tasklistmodel, docs_architecture_categorylistmodel, docs_architecture_goallistmodel, docs_architecture_mentalmapmodel, docs_architecture_dailynotemodel, docs_architecture_meetinglistmodel, docs_architecture_focussessionmodel, docs_architecture_runtime_boundaries [EXTRACTED 1.00]
- **Tag-to-Verified-Update Release Pipeline** — _github_workflows_release_prepare_release, _github_workflows_release_build_release, _github_workflows_release_publish_release, _github_workflows_release_verify_release_version, _github_workflows_release_create_update_manifest, docs_architecture_updateservice, readme_update_checking, cmakelists_update_manifest_tests [INFERRED 0.95]
- **Local-First Privacy and Data-Durability Guarantee** — agents_data_and_privacy_rules, docs_architecture_non_negotiable_constraints, security_vulnerability_reporting, docs_data_format_day_notes_excluded, docs_architecture_sync_outbox, contributing_core_qualities [INFERRED 0.85]
- **Daymark Icon Visual Composition** — assets_icons_org_daymark_dashboard_dark_rounded_tile, assets_icons_org_daymark_dashboard_background_gradient, assets_icons_org_daymark_dashboard_d_monogram, assets_icons_org_daymark_dashboard_checkmark_glyph [EXTRACTED 1.00]

## Communities (43 total, 1 thin omitted)

### Community 0 - "Task Repository SQLite Layer"
Cohesion: 0.10
Nodes (86): DailyNote, MentalMapChecklistItem, QSqlQuery, assignError(), bindCategory(), bindGoal(), bindMeeting(), bindMilestone() (+78 more)

### Community 1 - "JSON Export and Import"
Cohesion: 0.10
Nodes (67): QJsonObject, QJsonValue, QSet, applySettings(), categoryFromJson(), categoryToJson(), AppSettings, Category (+59 more)

### Community 2 - "Mental Map Model"
Cohesion: 0.07
Nodes (63): QVariantMap, QVariantList, CategoryListModel::subcategoriesAt(), cleanColor(), connectionVariant(), MentalMapConnection, MentalMapGroup, MentalMapNote (+55 more)

### Community 3 - "Task List Model"
Cohesion: 0.06
Nodes (59): Item, QByteArray, QDate, QDateTime, QHash, QModelIndex, QObject, QString (+51 more)

### Community 4 - "Category List Model"
Cohesion: 0.07
Nodes (55): Q_INVOKABLE, qsizetype, CategoryListModel, addCategory, addSubcategory, advanceWorkSuggestion, assignmentNames, categoriesChanged (+47 more)

### Community 5 - "Goal List Model"
Cohesion: 0.07
Nodes (50): Goal, QByteArray, QDate, QHash, QModelIndex, QObject, QString, QVariant (+42 more)

### Community 6 - "App Settings Keys"
Cohesion: 0.04
Nodes (48): AppSettings, accentPresetChanged, AccentPresetKey, colorModeChanged, ColorModeKey, ConfirmCompletionKey, confirmTaskCompletionChanged, DailyCapacityKey (+40 more)

### Community 7 - "Priority Engine and Domain Headers"
Cohesion: 0.06
Nodes (24): optional, QDateTime, Task, QStringList, PriorityEngine, evaluate, PriorityResult, reasons (+16 more)

### Community 8 - "Focus Session Timer"
Cohesion: 0.09
Nodes (39): QObject, QString, FocusSessionModel, clear, clearStatus, finished, FocusSessionModel::FocusSessionModel(), formattedDuration (+31 more)

### Community 9 - "Meeting List Model"
Cohesion: 0.08
Nodes (38): AppSettings, AppSettings, Meeting, QByteArray, QDateTime, QHash, QModelIndex, QObject (+30 more)

### Community 10 - "Language and Meeting Reminders"
Cohesion: 0.08
Nodes (30): QLocale, QTranslator, Language, LanguageManager, apply, m_installed, m_originalLocale, m_translator (+22 more)

### Community 11 - "Daily Note Model"
Cohesion: 0.11
Nodes (33): QObject, QString, TaskRepository, DailyNoteModel, clearStatus, DailyNoteModel::DailyNoteModel(), deleteToday, dirty (+25 more)

### Community 12 - "Update Service State"
Cohesion: 0.06
Nodes (32): qint64, Q_ENUM, qint64, QObject, QString, Release, State, QNetworkAccessManager (+24 more)

### Community 13 - "Desktop Notification Service"
Cohesion: 0.09
Nodes (26): QSystemTrayIcon, QObject, QString, DesktopNotificationService, DesktopNotificationService::DesktopNotificationService(), m_trayIcon, public, showNotification (+18 more)

### Community 14 - "Update Release Verification"
Cohesion: 0.12
Nodes (24): QVersionNumber, QObject, QString, QUrl, expectedAssetName(), isExpectedReleaseAssetUrl(), parseVersion(), busy (+16 more)

### Community 15 - "Settings QML Properties"
Cohesion: 0.13
Nodes (22): colorMode, confirmTaskCompletion, dailyCapacityMinutes, defaultEstimatedMinutes, defaultImportance, language, openDataDirectory, resetDefaults (+14 more)

### Community 16 - "Update Download and Install"
Cohesion: 0.20
Nodes (19): main(), cacheRoot(), State, QNetworkReply, QProcess, canDownload, checkForUpdates, clearDownload (+11 more)

### Community 17 - "Update Service Tests"
Cohesion: 0.24
Nodes (18): QByteArray, Release, parseRelease, Q_OBJECT, QByteArray, QObject, QString, releasePayload() (+10 more)

### Community 18 - "Repository Instructions and Licensing"
Cohesion: 0.19
Nodes (15): Draft-First Publication Gate, Tag-Driven Release Workflow, Build and Verification Command Sequence, Change Discipline / Definition of Done, Data and Privacy Rules, Daymark Repository Instructions (AGENTS.md), Four-Layer Source Layout (domain/core/data/presentation), Daymark Project Instructions (CLAUDE.md) (+7 more)

### Community 19 - "CI Build and Packaging"
Cohesion: 0.15
Nodes (14): Dependabot GitHub Actions Monthly Updates, build-test-package Matrix Job, CI Workflow, Pinned Immutable Action Revisions and Read-Only Permissions, Three-Platform Build Matrix (Linux/Windows/macOS), CPack Per-Platform Packaging Configuration, daymark_core Static Library, Inno Setup Per-User Windows Installer (+6 more)

### Community 20 - "Changelog and Release History"
Cohesion: 0.16
Nodes (14): create-update-manifest.cmake Per-Platform Manifest, Native Desktop Only Constraint, Daymark Changelog, Release 0.2.1 — Native Application Shell Baseline, Release 0.2.3 — Per-Platform Update Manifests, Release 0.2.4 — Local Meeting Scheduling, Release 0.2.6 — Stable-ID Mental Map Tests, Core Qualities: Native, Local-First, Explainable, Cross-Platform, Accessible (+6 more)

### Community 21 - "Model Test Fixtures"
Cohesion: 0.21
Nodes (8): CategoryListModel, GoalListModel, MeetingListModel, QSettings, QObject, TaskRepository, DataTransferService::DataTransferService(), TaskListModel

### Community 22 - "Four Today Interfaces"
Cohesion: 0.22
Nodes (9): Four Today Interfaces, Two-Day Daily Note Retention Window, Day Notes Deliberately Excluded From Exports, Slice 2 — Planning Essentials, Current Vertical Slice Feature Set, Midnight Command, Spatial Map, Quiet Focus, Daymark Hybrid, Meetings with Ten-Minute Local Notification, Mental Map Brainstorming Space (Clouds and Constellations) (+1 more)

### Community 23 - "Settings Storage Helpers"
Cohesion: 0.25
Nodes (9): Enum, AppSettings::AppSettings(), dataDirectory, localeUses24HourClock, QObject, QSettings, QString, storedBoundedInt() (+1 more)

### Community 24 - "App Settings Tests"
Cohesion: 0.31
Nodes (7): AppSettingsTest, ignoresInvalidStoredValues, persistsSelectedSettings, private, resetsPreferencesWithoutTouchingData, Q_OBJECT, QObject

### Community 25 - "Task List Model Tests"
Cohesion: 0.22
Nodes (3): Q_OBJECT, QObject, TaskListModelTest

### Community 26 - "QML Module and UI Conventions"
Cohesion: 0.25
Nodes (8): Daymark UI Conventions, daymark Executable Target, Daymark QML Module (URI Daymark 1.0), Qt Translation Pipeline (daymark_fr.ts), DesktopNotificationService, LanguageManager, Compact Layout Down to 640 Pixels, Global Keyboard Shortcuts

### Community 27 - "Mental Map Persistence Design"
Cohesion: 0.32
Nodes (8): Release 0.2.5 — Persistent Brainstorming Canvas, Mental Map Group/Note/Connection Tables, MentalMapModel, Planned Future Boundaries (calendar, graph service, platform services), mentalMap Export Object (groups, notes, connections), Backup Top-Level Object Schema, Slice 3 — Connected Mind Map, Slice 5 — Integrations and Distribution

### Community 28 - "Portable Backup Format"
Cohesion: 0.43
Nodes (7): Retired Projects Surface Migrated Into Categories, DataTransferService, SQLite Persistence and Migration Model, Backward-Compatible Optional Fields in Version 1, Validate-Then-Transaction Merge Import Contract, org.daymark.backup Portable Format, Portable .daymark.json Export and Merge Import

### Community 29 - "QML C++ Runtime Boundary"
Cohesion: 0.48
Nodes (7): CategoryListModel, DailyNoteModel, FocusSessionModel, GoalListModel, QML/C++ Runtime Boundary, TaskListModel, Slice 4 — Focus and Review

### Community 30 - "Data Transfer Tests"
Cohesion: 0.29
Nodes (3): Q_OBJECT, QObject, DataTransferTest

### Community 31 - "Goal List Model Tests"
Cohesion: 0.29
Nodes (3): Q_OBJECT, QObject, GoalListModelTest

### Community 32 - "Mental Map Model Tests"
Cohesion: 0.29
Nodes (3): Q_OBJECT, QObject, MentalMapModelTest

### Community 33 - "Release Workflow Jobs"
Cohesion: 0.33
Nodes (6): build-release Job, prepare-release Job, publish-release Job, verify-release-version.cmake Tag/Version Gate, Daymark CMake Project (version 0.2.6), Linux Host-Compatible Qt Runtime Install Decision

### Community 34 - "Application Icon Design"
Cohesion: 0.47
Nodes (6): Violet Background Gradient, Task Completion Checkmark Glyph, D Monogram Letterform, Dark Rounded Squircle Tile, Daymark Desktop Application Identity, Daymark Application Icon

### Community 35 - "Category List Model Tests"
Cohesion: 0.33
Nodes (3): CategoryListModelTest, Q_OBJECT, QObject

### Community 36 - "Daily Note Model Tests"
Cohesion: 0.40
Nodes (3): Q_OBJECT, QObject, DailyNoteModelTest

### Community 37 - "Meeting List Model Tests"
Cohesion: 0.40
Nodes (3): Q_OBJECT, QObject, MeetingListModelTest

### Community 38 - "Meeting Reminder Service Tests"
Cohesion: 0.40
Nodes (3): Q_OBJECT, QObject, MeetingReminderServiceTest

### Community 39 - "Accent Preset Setting"
Cohesion: 0.67
Nodes (3): accentPreset, setAccentPreset, AccentPreset

### Community 40 - "Interface Style Setting"
Cohesion: 0.67
Nodes (3): interfaceStyle, setInterfaceStyle, InterfaceStyle

## Knowledge Gaps
- **158 isolated node(s):** `install.sh script`, `public`, `m_trayIcon`, `score`, `reasons` (+153 more)
  These have ≤1 connection - possible missing edges or undocumented components.
- **1 thin communities (<3 nodes) omitted from report** — run `graphify query` to explore isolated nodes.

## Suggested Questions
_Questions this graph is uniquely positioned to answer:_

- **Why does `AppSettings` connect `App Settings Keys` to `Accent Preset Setting`, `Interface Style Setting`, `Settings QML Properties`, `Model Test Fixtures`, `Settings Storage Helpers`?**
  _High betweenness centrality (0.114) - this node is a cross-community bridge._
- **Why does `FocusSessionModel` connect `Focus Session Timer` to `Desktop Notification Service`?**
  _High betweenness centrality (0.073) - this node is a cross-community bridge._
- **What connects `install.sh script`, `public`, `m_trayIcon` to the rest of the system?**
  _158 weakly-connected nodes found - possible documentation gaps or missing edges._
- **Should `Task Repository SQLite Layer` be split into smaller, more focused modules?**
  _Cohesion score 0.09665621734587251 - nodes in this community are weakly interconnected._
- **Should `JSON Export and Import` be split into smaller, more focused modules?**
  _Cohesion score 0.0996488147497805 - nodes in this community are weakly interconnected._
- **Should `Mental Map Model` be split into smaller, more focused modules?**
  _Cohesion score 0.07307692307692308 - nodes in this community are weakly interconnected._
- **Should `Task List Model` be split into smaller, more focused modules?**
  _Cohesion score 0.0576271186440678 - nodes in this community are weakly interconnected._