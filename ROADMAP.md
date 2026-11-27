# tam-edit roadmap

`tam-edit` is Tamlinux's native C17 terminal text editor reproducing
micro's core editing experience while linking only `libtam` and libc.

- **Current version:** `1.2.0` (branch `dev`)
- **Progress:** 4 of 5 phases done (P1–P4 done; P5 deferred on Fred's word)
- **Versioning:** Semantic versioning; pre-1.0 minor versions marked
  migration milestones through `1.0.0` parity gate, post-1.0 minor adds
  capabilities and patch fixes defects.

## Phase summary

| Phase | Versions | Goal | Plan | Status |
| --- | --- | --- | --- | --- |
| **P1** | 0.0.1–0.1.0 | C17 scaffolding, document core, renderer, and daily-usable editor | [P1-c17-foundation.md](plans/P1-c17-foundation.md) | Done (2026-11-12) |
| **P2** | 0.2.0–0.3.0 | Splits, tabs, softwrap, INI config, and 158-language syntax highlighting | [P2-views-and-syntax.md](plans/P2-views-and-syntax.md) | Done (2026-11-13) |
| **P3** | 0.4.0–1.0.0 | Search/replace, encodings, shell, multi-cursor, parity audit, fuzzing, v1.0.0 | [P3-parity-and-v1.md](plans/P3-parity-and-v1.md) | Done (2026-11-13) |
| **P4** | 1.0.4–1.2.0 | Fresh repo, R1 cursor-movement fixes, keybinding standard, `libtam` adoption | [P4-libtam-and-polish.md](plans/P4-libtam-and-polish.md) | Done (2026-11-26) |
| **P5** | 1.3.0+ | Optional post-v1 extensions (plugins, diff, terminfo, grapheme clusters) | [P5-post-v1-extensions.md](plans/P5-post-v1-extensions.md) | Deferred |

## Decided

Fred decided these on 2026-11-12 (and provenance on 2026-11-17):

- **v1 scope:** Core editor without Lua plugins or embedded terminal.
- **Dependencies:** Pure C17 linking only `libtam` and libc (`make
  check-deps` enforces libc-only runtime linkage).
- **Configuration:** Native INI via `tam_ini` plus one-time micro JSON5
  importer.
- **License:** GPLv3 for the linked work, retaining micro's MIT notices.

## Phases and milestones

### Phase P1: C17 foundation and usable editor (0.0.1–0.1.0) — [P1-c17-foundation.md](plans/P1-c17-foundation.md)

| Version | Milestone | Status |
| --- | --- | --- |
| **0.0.1** | First runnable C17 editor: UTF-8 editing, LF/CRLF, save, dirty-quit, status line, PTY smoke tests | Done (2026-11-12) |
| **0.1.0** | Daily-usable editor: damage-tracked cell grid, grouped undo/redo, selections, prompts, clipboard, mouse, paste | Done (2026-11-12) |

### Phase P2: Views, configuration, and syntax highlighting (0.2.0–0.3.0) — [P2-views-and-syntax.md](plans/P2-views-and-syntax.md)

| Version | Milestone | Status |
| --- | --- | --- |
| **0.2.0** | Decoupled buffer/view, recursive splits, tabs, softwrap, truecolor schemes, `tam_ini` config, micro importer | Done (2026-11-12) |
| **0.3.0** | Native `tam_regex` engine, 158 embedded syntax definitions, 25 colorschemes, incremental line-state highlighter | Done (2026-11-13) |

### Phase P3: Full editing, parity audit, and v1.0.0 (0.4.0–1.0.0) — [P3-parity-and-v1.md](plans/P3-parity-and-v1.md)

| Version | Milestone | Status |
| --- | --- | --- |
| **0.4.0** | Regex search/replace, `iconv`/BOM encodings, background/interactive shell, completion, multi-cursor, embedded help | Done (2026-11-13) |
| **1.0.0** | Parity audit (`docs/PARITY_AUDIT.md`), 57 ported buffer fixtures, stress & fuzz suites, man page, Go tree pruned | Done (2026-11-13) |

### Phase P4: Hardening, keybindings, and libtam adoption (1.0.4–1.2.0) — [P4-libtam-and-polish.md](plans/P4-libtam-and-polish.md)

| Version | Milestone | Status |
| --- | --- | --- |
| **1.0.4** | Clean public repository on `dev` without micro's Go history (archived at `tam-edit-micro-fork`); `tam-idol` layout | Done (2026-11-18) |
| **1.0.5–1.0.6** | Close all 8 findings of `reviews/R1-cursor-movement.md` | Done (2026-11-20) |
| **1.1.0** | Adopt Tamlinux keybinding standard and move docs to `keys/keybindings.md` | Done (2026-11-25) |
| **1.1.1–1.1.2** | `libtam` 0.10.0 save permissions/symlinks and `libtam` 0.12.0 key-event initializers | Done (2026-11-26) |
| **1.2.0** | Move terminal, key, and process code onto `libtam` shared primitives; C17 standard | Done (2026-11-26) |

### Phase P5: Post-v1 extensions (1.3.0+) — [P5-post-v1-extensions.md](plans/P5-post-v1-extensions.md)

| Version | Milestone | Status |
| --- | --- | --- |
| **TBD** | Optional extensions: plugin path, terminfo reader, full grapheme clusters (only on Fred's word) | Deferred |

## How a milestone is done

A milestone is done when its acceptance checks in the phase plan pass
(`make test`, ASan/UBSan, `make check-deps`) and Fred has tried the
build. Bump `VERSION`, update the phase plan, and update this roadmap in
the same change.
