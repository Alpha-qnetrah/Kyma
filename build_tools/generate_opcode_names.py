#!/usr/bin/env python3
"""Generate the private opcode-name switch body from the bytecode specification."""

from __future__ import annotations

import json
import pathlib
import sys


def main() -> int:
    if len(sys.argv) != 3:
        raise SystemExit("usage: generate_opcode_names.py <opcodes.json> <output.inc>")
    specification = json.loads(pathlib.Path(sys.argv[1]).read_text(encoding="utf-8"))
    lines = [
        "// Generated from spec/bytecode/opcodes.json. Do not edit.",
        *[
            f'case OpCode::{item["enum"]}: return "{item["mnemonic"]}";'
            for item in specification["opcodes"]
        ],
        "",
    ]
    output = pathlib.Path(sys.argv[2])
    output.parent.mkdir(parents=True, exist_ok=True)
    contents = "\n".join(lines)
    if not output.exists() or output.read_text(encoding="utf-8") != contents:
        output.write_text(contents, encoding="utf-8")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
