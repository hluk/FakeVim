FakeVim
=======

FakeVim is library to emulate Vim in QTextEdit, QPlainTextEdit and possibly other Qt widgets.

Supported Features
------------------

Most of supported commands can be followed by motion command or executed in visual mode, work with registers or can be prefixed with number of repetitions.

Here is list of emulated commands with description where it can diverge from Vim in functionality.

### Modes
* normal
* insert and replace
* visual
* command line (`:`)

### Normal and Visual Modes
* basic movement -- `h`/`j`/`k`/`l`, `<C-U>`, `<C-D>`, `<C-F>`, `<C-B>` etc.
* word movement -- `w`, `e`, `b` etc.
* "inner/a" movement -- `ciw`, `3daw`, `ya{` etc.
* `f`, `t` movement
* `{`, `}` -- paragraph movement
* delete/change/yank/paste with register
* undo/redo
* `.` -- repeat last change
* `/search`, `?search`, `*`, `#`, `n`, `N` -- most of regular expression syntax used in Vim except `\<` and `\>` just is the same as `\b` in QRegExp
* `@`, `q` (macro recording, execution) -- special keys are saved as `<S-Left>`
* marks
* gv -- last visual selection; can differ if text is edited around it
* indentation -- `=`, `<<`, `>>` etc. with movement, count and in visual mode
* "to upper/lower" -- `~`, `gU`, `gu` etc.
* `i`, `a`, `o`, `I`, `A`, `O` -- enter insert mode
* scroll window -- `zt`, `zb`, `zz` etc.

### Command Line Mode
* `:map`, `:unmap`, `:inoremap` etc.
* `:source` -- very basic line-by-line sourcing of vimrc files
* `:substitute` -- substitute expression in range
* `:'<,'>!cmd` -- filter through an external command (e.g. sort lines in file with `:%!sort`)
* `:.!cmd` -- insert standard output of an external command
* `:read`
* `:yank`, `:delete`, `:change`
* `:move`, `:join`
* `:20` -- go to address
* `:history`
* `:registers`, `:display`
* `:nohlsearch`
* `:undo`, `:redo`
* `:normal`
* `:<`, `:>`

### Insert Mode
* `<C-O>` -- execute single command and return to insert mode
* `<C-V>` -- insert raw character
* `<insert>` -- toggle replace mode

### Options (:set ...)
* `autoindent`
* `clipboard`
* `configbackspace`
* `expandtab`
* `hlsearch`
* `ignorecase`
* `incsearch`
* `indent`
* `iskeyword`
* `scrolloff`
* `shiftwidth`
* `showcmd`
* `smartcase`
* `smartindent`
* `smarttab`
* `startofline`
* `tabstop`
* `tildeop`
* `wrapscan`

Implementation
--------------

There are appropriate signals emitted for command which has to be processed by the underlying editor widget (folds, windows, tabs, command line, messages etc.).
See example in `test/` directory or implementation of FakeVim plugin in Qt Creator IDE.

