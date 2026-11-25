# Copy, cut, and paste

Ctrl+C copies the selection, or the current line when there is no
selection. Ctrl+X copies and removes the same text. Ctrl+K deletes the
current line and preserves the clipboard. Ctrl+V pastes at the cursor
or replaces the selection.

On graphical terminals, the editor writes the clipboard through OSC 52.
Ctrl+V can read the desktop clipboard when a local clipboard helper is
available. The editor also accepts bracketed paste from the terminal.
Use the terminal's paste command for clipboard text from the local
desktop when editing over SSH. On the Linux console Ctrl+V reads the
editor's internal clipboard.
