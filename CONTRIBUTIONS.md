# Contributing to sxwm

Firstly, thanks for taking the time to contribute to sxwm! I appreciate your interest in improving the project.

## Code Style and Formatting

There is an included clangd formatting file with this inside, but it wont do everything for you.

### Indentation
Indentation must always be **tabs** not spaces.

### Blocks
- All blocks of C code must be encased with curly braces `{}`
- Blocks must also be seperated, each statement on a new line
- Statements must be formatted with a space between the keyword and the brackets

**Example**:
``` c
if (x) {
    y();
}
else {
    z();
}
```

### Comments
Comments must be like this to keep the look consistent across the whole program.
`/* this is a comment */`

### Function Declarations
They must look like this
``` c
void function_x(void)
{
}

void function_y(int y)
{
}
```

### Variable Naming
Variables and function names must be in `snake_case`
``` c
void function_a(void);
void function_b(void);
```
``` c
int some_variable = 2;
float other_variable = 5;
```

## Build

- Make sure `make` succeeds with no warnings on your system
- Don't commit any build artifacts or backup files
- Test your changes on **multiple monitors (Xephyr)**

## File Layouts

- Please try to not make a new C file or header unless it is necissary. Most of the codebase is kept in the `sxwm.c` file.

## Submitting Changes

- Open a pull request with a **clear report** of the change(s)
- Keep commits having one purpose per commit
- If you fix a bug, please describe how to reproduce it
- If adding a feature, make sure it is consistent with sxwm’s minimalist philosophy
- Please don't keep multiple changes in one PR. Open a new one for that

## Documentation

- If applicable, **update `sxwm.1`, `README.md` and `default_sxwmrc`**

## Respect the Existing Structure

- Before large changes, open an issue discussion
- Please don’t change for style unless it improves clarity or fixes a problem

## Testing

- [x] Builds with no warnings
- [x] Tested to work fully
- [x] All changes are explained in the PR
- [x] Configuration reload works (if relevant)

---

**Happy hacking!**
