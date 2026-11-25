# Custom keybindings

Use Ctrl+G to inspect the effective key map. Use Ctrl+E to search for
bindable actions, then press Up or Down to choose one and Enter to run it.

Edit the bindings section in settings.ini to remap single chords, or
use the bind command in the editor:

    [bindings]
    Alt+L = goto-line
    Ctrl+Shift+Left = select-word-left

    bind Alt+L goto-line
    unbind Alt+L

The unbind command removes a custom mapping and restores the default for
that chord. Existing Micro action names and older key spellings remain
accepted. Physical keys are written with plus signs between modifiers
and named keys, such as Ctrl+Left, Alt+Backspace, and Escape.

Modified navigation depends on the terminal. On the Linux console use
Alt+, and Alt+. for words, Alt+< and Alt+> for buffer boundaries,
Alt+Backspace and Alt+D for word deletion, and Alt+M to mark a selection.
