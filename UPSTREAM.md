# Code taken from other projects

This file lists every piece of code tam-edit takes from another project.
An idea taken without its code goes in `ideas/`, not here.

The C11 editor in `src/` is written from scratch in C11 and links only
`libtam` and the C library. The runtime data files and embedded tables
below come from `micro`.

## micro runtime data and embedded tables

- **Project:** micro (https://github.com/micro-editor/micro)
- **License:** MIT (`LICENSE-MIT`, with third-party notices in
  `LICENSE-THIRD-PARTY`)
- **Source:** `runtime/syntax/`, `runtime/colorschemes/`, and
  `runtime/help/` in `micro-editor/micro` (carried over from the
  archived fork `greenermoose/tam-edit-micro-fork`, branch `c11` at
  `b1cc5a4e`, when this repository started fresh on 2026-11-18)
- **Here:** `runtime/syntax/`, `runtime/colorschemes/`, `runtime/help/`,
  and the generated C tables `data/syntax_data.[ch]`,
  `data/colorschemes_data.[ch]`, and `data/help_data.[ch]`
- **Changes:** Converted into embedded C tables by maintainer scripts in
  `tools/` so the C11 editor runs without Go or external runtime packs;
  original MIT notices are preserved in `LICENSE-MIT` and
  `LICENSE-THIRD-PARTY`.
- **Taken:** 2026-11-12 (carried into the fresh repository on 2026-11-18)

## How to record a piece

One section per piece, newest last:

    ## <what it is>

    - **Project:** name and URL
    - **License:** the project's license
    - **Source:** file path in the project, and the commit it came from
    - **Here:** the file or files it now lives in
    - **Changes:** how it was rewritten or reshaped, and why
    - **Taken:** YYYY-MM-DD

Add the project's copyright notice to `COPYRIGHT` in the same commit.
When upstream fixes a bug in code we took, check whether our copy has
the same bug, and record what we did in `bugs/`.
