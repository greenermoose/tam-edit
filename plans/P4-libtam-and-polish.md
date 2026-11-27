# P4: Post-v1 hardening, keybinding standard, and libtam adoption

- **Status:** done (1.0.4–1.2.0, 2026-11-26)
- **Versions:** 1.0.4 to 1.2.0

## Goal

Start the clean public repository history, close all R1 cursor-movement
findings, adopt the shared Tamlinux keybinding standard, and migrate
terminal/key/process/save paths onto `libtam` 0.10–0.14.

## Versions

- **1.0.4 (done 2026-11-18):** Fresh public repository on branch `dev`
  without micro's Go history (archived at `tam-edit-micro-fork`); adopted
  `tam-idol` standard.
- **1.0.5–1.0.6 (done 2026-11-20):** Closed all 8 findings of
  `reviews/R1-cursor-movement.md` (visual column tabsize, byte-vs-rune
  indexing, scroll clamping, multi-cursor metadata, Unicode word classes).
- **1.1.0 (done 2026-11-25):** Adopted Tamlinux keybinding standard and
  moved keybinding docs to `keys/keybindings.md`.
- **1.1.1–1.1.2 (done 2026-11-26):** `libtam` 0.10.0 (keep file modes and
  symlinks on save) and `libtam` 0.12.0 (designated key-event
  initializers).
- **1.2.0 (done 2026-11-26):** Moved terminal, key, and process code onto
  `libtam` shared primitives and updated docs to C17.

## Acceptance

`make test`, ASan/UBSan, PTY smoke tests, and `make check-deps` pass.

## Dependencies

`libtam` (static) and POSIX libc only.

## Risks

None (completed).

## Open points

None.
