# Contributing to sxwm

Firstly, thanks for taking the time to contribute to `sxwm`! Your interest
in improving the project is truly appreciated.

## Code Style and Formatting

Please follow the rules below to keep the codebase consistent and clean.  
You can also run `make clangd` to generate `compile_flags.txt`

### Indentation

* Use **tabs** for indentation - never spaces.
* You can use spaces to pad things e.g. parameters:
```c
foo(dpy, root
    NULL, NULL);
```

### Blocks

* All blocks of C code must be wrapped in curly braces `{}`.
* Each statement must be on its own line.
* Always add a space between keywords and the opening parenthesis.

**Example**:

```c
if (x) {
    y();
}
else {
    z();
}
```

### Comments

* Use this format for comments to maintain consistency:

```c
/* this is a comment */
```

* For temporary notes, use:

```c
/* TODO: something to fix */
/* FIXME: known issue */
```

### Function Declarations

Functions should look like this:

```c
void f(void)
{
}

void g(int y)
{
}
```

### Variable and Function Naming

* Use `snake_case` for all variable and function names.

**Examples**:

```c
int some_variable = 2;
float other_variable = 5;

void do_something(void);
```

### Headers

If you create a header file, use pragma. It is supported on most modern compilers:

```c
#pragma once
/* content */
```

### Line Length

* Keep lines under **100 characters** when possible to improve readability.

### Whitespace

* No trailing whitespace.
* One blank line between function definitions.
* Avoid unnecessary vertical spacing.

### Include Order

Organize includes like this:

1. Corresponding header (if any)
2. Standard library headers
3. Other project headers

---

## Build

* `make` must succeed **with no warnings or errors**.
* Don’t commit build artifacts (e.g., `.o`, `sxwm`) or backup files (e.g., `*~`).
* Test your changes, if possible on **multi-monitor setups** using **Xephyr**.

---

## File Layout

* Don’t create new `.c` or `.h` files unless absolutely necessary.
* Keep most changes within `sxwm.c` to maintain cohesion.

---

## Submitting Changes

* Open a pull request with a **clear description** of what and why.
* Keep **one purpose per commit** - don’t mix unrelated changes.
* If fixing a bug, describe **how to reproduce it**.
* If adding a feature, ensure it fits with `sxwm`’s **minimalist philosophy**.
    * If said feature doesn't fit, you can make a patch instead.
* **Open separate PRs** for unrelated changes.

---

## Git Commit Guidelines

* Use imperative mood: `Add foo`, not `Added foo`.
* Keep the summary under 50 characters.
* Add a detailed body if needed.

**Example**:

```
Fix crash when window is closed

Previously, sxwm would segfault if a client closed itself.
This patch adds a check for null pointers before accessing window data.
```

---

## Tooling & Analysis

* Avoid committing debug code like stray `printf()` or `fprintf(stderr, ...)`.
* You _may_ want to run:
  * `cppcheck`
  * `clang-tidy`
---

## Documentation

Update all relevant documentation if applicable:

* `default_sxwmrc`
* `docs/CHANGELOG.md`
* `docs/sxwm.1`
* `docs/sxwm.md`
* `docs/sxwm-dev.md`
* `README.md`

---

## Respect the Existing Structure

* For **large changes**, open an issue for discussion first.
* Do not change code for personal style unless it improves clarity or solves a specific problem.

---

## Testing Checklist

Please ensure the following before opening a PR:

* [x] Code builds **without warnings**
* [x] Changes are **fully tested**
* [x] PR includes a **clear explanation**
* [x] Configuration reload works (if relevant)

---

## Code Review Etiquette

* Be respectful and open to feedback.
* Feel free to ask questions about any review comments.
* Reviews are collaborative and meant to improve the code, not criticize you personally.

---

**Happy hacking!**
