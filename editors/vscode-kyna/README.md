# Kyna VS Code support

Provides canonical `.kyna` and legacy `.ky` language registration, syntax highlighting, Python-style `#` line comments, snippets, keyword and declaration completion, import-path and exported-member completion, live compiler diagnostics, and `Kyna: Run File` / `Kyna: Check File` commands. A Run button appears in the editor title and status bar. Version 0.3.0 also displays compiler best-practice warnings, including unprotected `fetch` and filesystem calls.

The extension uses the purple Kyna K as its marketplace icon and matching purple light/dark `.kyna` file icons. The black-and-white icon remains in `assets/kyna-extension.png` and `assets/kyna-extension.svg`; it is preserved for branding outside the installed extension. Run and Check toolbar icons remain theme-adaptive.

## Completion rules

- At an ordinary identifier, completion offers Kyna keywords, primitive types, standard-library names, snippets, and declarations found in the current document.
- Between the quotes of `import "..."`, completion offers `.kyna` files with paths relative to the importing file.
- After an imported alias and dot, such as `math.`, completion reads that module and offers only declarations marked `export`.
- `.`, `"`, and `/` retrigger completion automatically; normal editor completion can also be invoked manually.

Diagnostics run 250 ms after edits. Older checker processes are cancelled so a slow result cannot overwrite diagnostics for a newer buffer.

## Development install

From the repository root:

```sh
"/Applications/Visual Studio Code.app/Contents/Resources/app/bin/code" --extensionDevelopmentPath="$PWD/editors/vscode-kyna"
```

This opens a VS Code extension-development window. To package and install permanently, run `make vscode-package`, then:

```sh
"/Applications/Visual Studio Code.app/Contents/Resources/app/bin/code" --install-extension editors/vscode-kyna/kyna-language-support-0.3.0.vsix --force
```

Set `kyna.executable` if the CLI is not at the workspace's `build/kyna` or on PATH. Live checking sends the unsaved buffer to the CLI together with its real file path, so relative imports resolve without writing temporary source files. The extension deliberately delegates language behavior to the CLI instead of duplicating the compiler.
