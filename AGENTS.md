# Agent Guidelines for xDuinoRails_MM

This document provides guidelines for agents working on the xDuinoRails_MM codebase.

## Pre-commit Checks

Run CI/CD before commit:
- Install PlatformIO if not available.
- Execute ```pio run``` and ```pio test``` to validate the changes.

## General Coding and Style

- **English Language:** All documentation, comments, and commit messages must be in English.
- **Commit Messages:** Follow the [Conventional Commits specification](https://www.conventionalcommits.org/en/v1.0.0/).
- **C++ Style Guide:** Adhere to the [Google C++ Style Guide](https://google.github.io/styleguide/cppguide.html) for consistent coding style.
- **Dependency Injection:** For hardware configurations like pin numbers, use dependency injection by passing them into class constructors from a central configuration point (e.g., the main .ino file) rather than using global definitions via header files. This decouples classes from specific hardware setups.
- **READMEs:** Add a basic `README.md` file to any new subdirectories to explain the purpose of the code within that directory.
