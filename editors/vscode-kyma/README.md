# Kyma VS Code support

Provides `.ky` language registration, syntax highlighting, bracket/comment behavior, snippets, and `Kyma: Run File` / `Kyma: Check File` commands.

## Development install

From the repository root:

```sh
code --extensionDevelopmentPath="$PWD/editors/vscode-kyma"
```

This opens a VS Code extension-development window. To package and install permanently, run `make vscode-package`, then:

```sh
code --install-extension editors/vscode-kyma/kyma-language-support-0.1.0.vsix --force
```

Set `kyma.executable` if `kyma` is not on PATH. The extension intentionally delegates execution to the installed CLI; it does not duplicate the language runtime.
