# P1: C17 foundation and daily-usable editor

- **Status:** done (0.0.1–0.1.0, 2026-11-12)
- **Versions:** 0.0.1 to 0.1.0

## Goal

Scaffold the C17 rewrite of `tam-edit` linking only libc and `libtam`,
and deliver a daily-usable terminal editor with undo, selections,
prompts, mouse, and clipboard.

## Versions

- **0.0.1 (done 2026-11-12):** First runnable C17 editor on `libtam`
  0.7.0: open, edit UTF-8, LF/CRLF preservation, save (`Ctrl+S`), dirty
  quit (`Ctrl+Q`), status line, and PTY smoke tests.
- **0.1.0 (done 2026-11-12):** Daily-usable editor (Milestone 1):
  double-buffered cell-grid renderer with damage tracking, wide/combining
  chars, grouped undo/redo, visual selections, word motions, action
  registry, interactive prompts, OSC 52 / external clipboard, SGR mouse,
  bracketed paste, and terminal signals.

## Acceptance

Unit tests, ASan/UBSan checks, PTY smoke tests, and `make check-deps`
(libc only) pass.

## Dependencies

`libtam` (static) and POSIX libc only.

## Risks

None (completed; recorded retrospectively from the C17 migration roadmap).

## Open points

None.
