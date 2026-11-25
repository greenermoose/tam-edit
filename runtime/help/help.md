# tam-edit help text

tam-edit is an easy to use, intuitive, text editor that takes advantage of the
full capabilities of modern terminals.

tam-edit can be controlled by commands entered on the command bar, or with
keybindings. To open the command prompt, press `Alt+E`. The `>` prompt will
display. From now on, when the documentation shows a command to run (such as
`> help`), press `Ctrl+E` and type the command followed by enter.

For a list of the default keybindings, run `> help defaultkeys`.
For more information on keybindings, see `> help keybindings`.
Press Ctrl+G to see the active keybindings.

## Quick-start

To quit, press `Ctrl+Q`. Save by pressing `Ctrl+S`. Press `Ctrl+E`, as previously
mentioned, to start typing commands. To see which commands are available, at the
prompt, press tab, or view the help topic with `> help commands`.

Move the cursor around with the mouse or with the arrow keys. Enter text simply
by pressing character keys.

If the colorscheme doesn't look good, you can change it with
`> set colorscheme ...`. You can press tab to see the available colorschemes,
or see more information about colorschemes and syntax highlighting with `> help
colors`.

Run `next-split` from the palette to move between splits. Use
`> vsplit filename` or `> hsplit filename` to open a split.

## Accessing more help

tam-edit has a built-in help system which can be accessed with the `> help` command.

To view help for the various available topics, press `Ctrl+E` to access command
mode and type in `> help` followed by a topic. Typing just `> help` will open
this page.

Here are the available help topics:

* `tutorial`: A brief tutorial which gives an overview of all the other help
   topics
* `keybindings`: Gives a full list of the default keybindings as well as how to
   rebind them
* `defaultkeys`: Gives a more straight-forward list of the hotkey commands and
   what they do
* `commands`: Gives a list of all the commands and what they do
* `options`: Gives a list of all the options you can customize
* `plugins`: Explains how tam-edit's plugin system works and how to create your own
   plugins
* `colors`: Explains tam-edit's colorscheme and syntax highlighting engine and how
   to create your own colorschemes or add new languages to the engine
* `copypaste`: Explains tam-edit's copy and paste usage and configuration.
   It describes how copy and paste is working on different operating systems and
   setups.

For example, to open the help page on plugins you would run `> help plugins`.

I recommend looking at the `tutorial` help file because it is short for each
section and gives concrete examples of how to use the various configuration
options in tam-edit. However, it does not give the in-depth documentation that the
other topics provide.
