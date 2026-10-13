# 4coder-fleury

A working copy of [4coder](https://github.com/4coder-archive/4coder) with
[Ryan Fleury's customization layer](https://github.com/4coder-archive/4coder_fleury)
plus hand-written **Python, Lua and Perl** lexers and a standalone test suite
for them.

This repo bundles the three upstream pieces so a single `git clone --recursive`
gets you a buildable editor without juggling sibling directories.

## Layout

| Path | What it is |
| --- | --- |
| `code/` | The 4coder base (Allen Gilliam's C/C++ editor core). |
| `code/custom/4coder_fleury/` | Fleury's customization layer, tracked as a `git subtree` so upstream history is preserved, with the language additions on top. |
| `4coder-non-source/` | Prebuilt fonts / themes / third-party headers, tracked as a submodule pointing at `4coder-archive/4coder-non-source`. |
| `build/` | Local output directory. Not tracked. |

## Build

Requires a recent g++ (or clang) and the same X11 / GLX / ALSA / freetype
packages the upstream 4coder build docs list.

```sh
git clone --recursive git@github.com:justaperlhacker/4coder-fleury.git
cd 4coder-fleury
./code/bin/build-linux.sh
```

The result lands in `build/`. Install with:

```sh
cp build/4ed build/4ed_app.so build/custom_4coder.so ~/.local/4coder-fleury/
```

## Adding a language

Fleury's `4coder_fleury_lang_list.h` is the single registration point. To add
a new language:

1. Write a hand-rolled lexer `4coder_fleury_lang_<name>.cpp` — copy the
   structure of `4coder_fleury_lang_python.cpp` / `4coder_fleury_lang_lua.cpp`
   / `4coder_fleury_lang_perl.cpp`.
2. Add the `F4_RegisterLanguage(...)` block for the language's extensions.
3. Rebuild `custom_4coder.so`.

## Testing the lexers

Under `code/custom/4coder_fleury/tests/`:

```sh
./run.sh         # build + run all lexer suites
./run.sh -v      # verbose: PASS lines and full token dumps on failure
```

Adding a new language's cases means dropping one `cases_<lang>.h` file into
`tests/`, wiring it into `lexer_tests.cpp`, and re-running `./run.sh`. See the
headers for the expected shape of `Suite`, `Case` and `TokExpect`.

## Attribution

- 4coder: Copyright Allen Gilliam / 4coder-archive, MIT.
- 4coder_fleury: Copyright Ryan Fleury, MIT.

The Python / Lua / Perl lexers and the standalone test suite in this repo are
original work built on top of Fleury's multi-language layer.
