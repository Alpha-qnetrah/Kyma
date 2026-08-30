# Standard library

The v0.1 runtime exposes two deliberately small native functions: `print(value, ...)` writes values separated by spaces, and `typeOf(value)` returns the concrete runtime type name. They are installed in the global environment rather than special-cased in the parser. Future standard-library modules will use the planned module loader and will remain ordinary Kyma-callable functions at the language boundary.
