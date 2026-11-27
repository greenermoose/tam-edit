# P3: Feature parity, stabilization, and v1.0.0

- **Status:** done (0.4.0–1.0.0, 2026-11-13)
- **Versions:** 0.4.0 to 1.0.0

## Goal

Complete the non-plugin micro feature set, pass the parity audit and
fuzzing/stress gates, and ship `tam-edit` 1.0.0.

## Versions

- **0.4.0 (done 2026-11-13):** Full editing (Milestone 4): regex captures
  and search/replace (`incsearch`, `hlsearch`), `iconv`/BOM encoding
  support, non-blocking background `run` and foreground `ShellMode`,
  `Tab` completion, multi-cursor editing, and embedded help topics.
- **1.0.0 (done 2026-11-13):** Parity gate and stabilization (Milestone
  5): `docs/PARITY_AUDIT.md`, 57 ported VSCode/micro buffer fixtures,
  50k-line / 220-cursor stress tests, ASan/UBSan fuzz harnesses, man page
  and desktop entry, benchmarks (`3.25 ms` startup, `4.81 MB` 10k-line
  `VmHWM`), and Go tree pruned.

## Acceptance

All unit, ported, stress, fuzz, PTY smoke, and `make check-deps` checks
pass within Section 8 performance targets.

## Dependencies

`libtam` (static) and POSIX libc only.

## Risks

None (completed; recorded retrospectively).

## Open points

None.
