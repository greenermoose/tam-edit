# P2: Views, configuration, and syntax highlighting

- **Status:** done (0.2.0–0.3.0, 2026-11-13)
- **Versions:** 0.2.0 to 0.3.0

## Goal

Add multi-split panes, tabs, softwrap, native INI configuration, micro
config import, and native UTF-8 syntax highlighting for all 158 language
definitions.

## Versions

- **0.2.0 (done 2026-11-12):** Views and configuration (Milestone 2):
  reference-counted buffer/view split, recursive vertical/horizontal
  split panes, multi-tab management, softwrap/wordwrap, truecolor
  colorscheme engine (`color-link`), native `tam_ini` settings/bindings,
  and `--import-micro-config`.
- **0.3.0 (done 2026-11-13):** Syntax highlighting (Milestone 3):
  linear-time UTF-8 Thompson-NFA/Pike-VM `tam_regex` engine, committed C
  tables for 158 syntax definitions and 25 colorschemes, incremental
  line-state highlighter, user YAML overrides, and differential parity
  suite against Go `pkg/highlight`.

## Acceptance

Unit tests, differential syntax parity suite (0 mismatches), ASan/UBSan,
PTY smoke tests, and `make check-deps` pass.

## Dependencies

`libtam` (static) and POSIX libc only (maintainer converter is build-time
independent).

## Risks

None (completed; recorded retrospectively).

## Open points

None.
