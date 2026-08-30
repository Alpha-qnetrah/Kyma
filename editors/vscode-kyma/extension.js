const vscode = require('vscode');
const path = require('path');

function executable() {
  return vscode.workspace.getConfiguration('kyma').get('executable', 'kyma');
}
function run(file, check) {
  const terminal = vscode.window.createTerminal({ name: check ? 'Kyma Check' : 'Kyma Run', cwd: path.dirname(file) });
  terminal.show(true);
  terminal.sendText(`${executable()} ${check ? '--check ' : ''}"${file.replace(/"/g, '\\"')}"`);
}
function activate(context) {
  context.subscriptions.push(
    vscode.commands.registerCommand('kyma.runFile', () => {
      const editor = vscode.window.activeTextEditor;
      if (editor) run(editor.document.fileName, false);
    }),
    vscode.commands.registerCommand('kyma.checkFile', () => {
      const editor = vscode.window.activeTextEditor;
      if (editor) run(editor.document.fileName, true);
    })
  );
}
function deactivate() {}
module.exports = { activate, deactivate };
