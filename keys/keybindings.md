# tam-edit keybindings

The editor follows the [Tamlinux keybinding standard](https://github.com/greenermoose/tam-idol/blob/dev/keys/keybindings.md).

- Ctrl+N creates an untitled tab. Ctrl+D closes the active split or tab
  and leaves an untitled buffer when closing the last document.
- Ctrl+R starts the replace command. Ctrl+U reloads the active file.
  Closing its last view or reloading asks before discarding unsaved edits.
  Ctrl+Q checks all tabs before quitting.
- Ctrl+E opens the named-action palette and accepts editor commands with
  arguments. Alt+E opens the command prompt. Ctrl+G shows the current
  keybindings or guidance for the active prompt.
- Alt+J and Alt+Shift+J step through search matches. Alt+T and Alt+Shift+T
  switch tabs. Alt+F starts literal search. Alt+Shift+M adds cursors to
  the selected lines. Alt+W toggles soft wrap.
- Home goes to column one. The smart-home action toggles between
  indentation and column one. Ctrl+Backspace uses the terminal's
  distinct word-delete input where available. Alt+Backspace works
  on the Linux console and in graphical terminals.
- User bindings are single chords in the settings.ini bindings section.
  Standard spellings and lowercase, hyphenated action names are used
  for new bindings. Existing imported spellings remain accepted.
