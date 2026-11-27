# P5: Post-v1 extensions (deferred)

- **Status:** not started (deferred until Fred chooses a direction)
- **Versions:** 1.3.0+ / 2.0.0 (unassigned)

## Goal

Track optional post-v1 extension ideas from the original migration
roadmap until Fred decides whether any should be built.

## Versions

- **Optional extensions (deferred):**
  - Plugin path (embedded Lua or a C plugin ABI, or no scripting).
  - Embedded terminal pane / VT parser.
  - Diff view and linter integration.
  - Terminfo reader for unusual terminals if ANSI/xterm proves
    insufficient.
  - Full grapheme-cluster segmentation and emoji sequences.

## Acceptance

Defined per extension if and when Fred authorizes it.

## Dependencies

Must preserve libc-only runtime linkage (`make check-deps`).

## Risks

Scope creep into areas already served by dedicated Tamlinux tools
(`tam-knox` for diffs, `tam-last` for terminals).

## Open points

- Plugin path after v1, terminfo necessity, and whether `tam_regex` ever
  needs backreferences or lookaround.
