# Contributing to sxwm

Firstly, thanks for taking the time to contribute to `sxwm`! Your interest
in improving the project is truly appreciated.

## Code Style and Formatting

There is an included `clangd` formatting file, but it won’t do everything for you.
Please follow the rules below to keep the codebase consistent and clean.

### Indentation

* Use **tabs** for indentation — never spaces.

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
void function_x(void)
{
}

void function_y(int y)
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

### Header Guards

If you create a header file, use header guards:

```c
#ifndef HEADER_NAME_H
#define HEADER_NAME_H

// content

#endif /* HEADER_NAME_H */
```

### Line Length

* Keep lines under **100 characters** when possible to improve terminal readability.

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

* `make` must succeed **with no warnings** on your system.
* Don’t commit build artifacts (e.g., `.o`, `sxwm`) or backup files (e.g., `*~`).
* Test your changes, especially on **multi-monitor setups** using **Xephyr**.

---

## File Layout

* Don’t create new `.c` or `.h` files unless absolutely necessary.
* Keep most changes within `sxwm.c` to maintain cohesion.

---

## Submitting Changes

* Open a pull request with a **clear description** of what and why.
* Keep **one purpose per commit** — don’t mix unrelated changes.
* If fixing a bug, describe **how to reproduce it**.
* If adding a feature, ensure it fits with `sxwm`’s **minimalist philosophy**.
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

* Use the provided `.clang-format` file where applicable.
* You may optionally run:

  * `cppcheck`
  * `clang-tidy`
* Avoid committing debug code like stray `printf()` or `fprintf(stderr, ...)`.

---

## Editor Configuration

A sample `.editorconfig` file is provided (or you can request one).
It ensures editors match the project’s formatting conventions.

---

## Documentation

Update all relevant documentation if applicable:

* `sxwm.1`
* `README.md`
* `default_sxwmrc`
* `CHANGELOG.md`

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
