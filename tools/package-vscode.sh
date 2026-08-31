#!/bin/sh
set -eu
root=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
ext="$root/editors/vscode-kyma"
version=$(sed -n 's/.*"version": "\([^"]*\)".*/\1/p' "$ext/package.json" | head -1)
test -n "$version"
out="$ext/kyma-language-support-$version.vsix"
tmp=$(mktemp -d)
trap 'rm -rf "$tmp"' EXIT
mkdir -p "$tmp/extension"
cp -R "$ext"/* "$tmp/extension/"
rm -f "$tmp/extension/"*.vsix
cat > "$tmp/[Content_Types].xml" <<'XML'
<?xml version="1.0" encoding="utf-8"?><Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types"><Default Extension="json" ContentType="application/json"/><Default Extension="js" ContentType="application/javascript"/><Default Extension="xml" ContentType="application/xml"/><Default Extension="png" ContentType="image/png"/><Default Extension="svg" ContentType="image/svg+xml"/><Override PartName="/extension.vsixmanifest" ContentType="application/vsix-manifest"/></Types>
XML
cat > "$tmp/extension.vsixmanifest" <<XML
<?xml version="1.0" encoding="utf-8"?><PackageManifest xmlns="http://schemas.microsoft.com/developer/vsx-schema/2011" Version="2.0.0"><Metadata><Identity Id="kyma-language-support" Version="$version" Language="en-US" Publisher="kyma-lang" /><DisplayName>Kyma Language Support</DisplayName><Description xml:space="preserve">Kyma syntax, completions, live diagnostics, imports, comments, and run/check tools.</Description></Metadata><Installation AllUsers="false" Scope="CurrentUser" /><Dependencies /><Assets><Asset Type="Microsoft.VisualStudio.Code.Manifest" Path="extension/package.json" /><Asset Type="Microsoft.VisualStudio.Services.Content.Details" Path="extension/README.md" /></Assets></PackageManifest>
XML
rm -f "$out"
(cd "$tmp" && zip -qr "$out" .)
echo "$out"
