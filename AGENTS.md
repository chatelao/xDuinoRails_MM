# Agent Guidelines for xDuinoRails_MM

This document provides guidelines for agents working on the xDuinoRails_MM codebase.

## Pre-commit Checks

Run CI/CD before commit:
- Use PlatformIO.
- Execute ```pio run``` and ```pio test``` to validate the changes.

## Test coverage
- Write test cases for all new and refactored features
- Link the underlying specification file and line in the comment above the test case if available
- Don't comment tests permanently on check-in to pass the tests, fix the tests or the code

## General Coding and Style

- **English Language:** All documentation, comments, and commit messages must be in English.
- **Commit Messages:** Follow the [Conventional Commits specification](https://www.conventionalcommits.org/en/v1.0.0/).
- **C++ Style Guide:** Adhere to the [Google C++ Style Guide](https://google.github.io/styleguide/cppguide.html) for consistent coding style.
- **Use this clang format conventions:**

```---
# .clang-format
Language: Cpp

# Aligns variable declarations (e.g., int a; double b;)
AlignConsecutiveDeclarations: true

# Aligns assignment operators (e.g., a   = 1; var = 2;)
AlignConsecutiveAssignments: true

# Aligns bitfield widths (e.g., int a : 1; int b : 2;)
AlignConsecutiveBitFields: true

# Aligns macro definitions (e.g., #define A   1)
AlignConsecutiveMacros: true

# Aligns trailing comments
AlignTrailingComments: true
```

- **Dependency Injection:** For hardware configurations like pin numbers, use dependency injection by passing them into class constructors from a central configuration point (e.g., the main .ino file) rather than using global definitions via header files. This decouples classes from specific hardware setups.
- **READMEs:** Add a basic `README.md` file to any new subdirectories to explain the purpose of the code within that directory.

## CV Reset Logic
- Writing the value 0 or 8 to CV 8 (Manufacturer ID) triggers a factory reset, restoring all CVs to their default values.
- A reboot is automatically triggered after a CV reset or whenever CV 8 is written to.
- Every new feature or logic change, like this reset mechanism, must be accompanied by native test cases in `test/test_native/` to ensure correctness and prevent regressions. These tests are executed automatically in the CI/CD pipeline.
