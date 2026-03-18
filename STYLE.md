**Style Guide**
- Base on the Google C++ Style Guide (https://google.github.io/styleguide/cppguide.html). Use this document as the reference for formatting, naming, and structuring code.

**Formatting**
- Run `clang-format` before commits. The repo contains `.clang-format` already configured to Google style.
- Group includes in this order: (1) header corresponding to implementing `.cpp`, (2) C standard headers, (3) C++ standard headers, (4) third-party/Qt, (5) project headers. Separate each group with a blank line.
- Prefer `constexpr` or `const` over magic literals and avoid `using namespace` at namespace scope.

**Headers**
- Every header must be “self-contained”: include everything it depends on, and guard it with `#ifndef KAPRAXIS_<PATH>_<FILE>_H_ … #endif`.
- Forward declarations are OK for pointers/references to reduce dependencies, but include the full definition before using members.

**Naming**
- Use `PascalCase` for types and functions, `snake_case` for variables and file names, and append `_` to private member variables (per Google style). Qt signal/slot names may keep Qt-style naming when overriding.
- Keep names English where possible, especially for public symbols. Localized identifiers are acceptable if they better describe UI-specific concepts.

**Structure**
- Prefer `namespace kapraxis { ... }` as soon as it makes sense, organizing functionality into sub-namespaces like `ui`, `repo`, or `domain`. Document and justify any deviations (e.g., Qt slots or auto-generated moc helpers) right beside the code block.
- Keep translation units focused: each `.cpp` should install its own helper functions before the class implementation to reduce global state.

**Behavior**
- Favor immutable data (`const` references, `const` methods) and small helper functions that explain intent.
- Log errors consistently (use `qDebug()` for now) and fail fast when invariants are violated.

Document deviations from these rules directly in the file, referencing the section that explains the exception.

**Internacionalização**
- Run `/usr/lib/qt6/bin/lupdate src -ts translations/kapraxis_pt_BR.ts` after new or updated `tr()` strings are introduced.
- Rebuild the binary `.qm` with `/usr/lib/qt6/bin/lrelease translations/kapraxis_pt_BR.ts` and keep `resources.qrc` in sync so the runtime can load the translation.
