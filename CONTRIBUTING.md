# Contributing to Caster Soundboard

Thank you for your interest in contributing to **Caster Soundboard**! Whether you are reporting a bug, proposing a new feature, or submitting code, your contributions help make this project better for everyone.

---

## 🧭 Getting Started

* **Architecture & Coding Standards**: Please read **[`AGENTS.md`](AGENTS.md)** before contributing code. It documents our project architecture, build commands, technical guardrails (audio routing, PipeWire/Flatpak safety, QML interaction guidelines), and verification protocols.
* **Target Branch**: All active development and pull requests must target the **`main`** branch. The legacy Qt Widgets v1 codebase is archived in `legacy-v1`.

---

## 🐛 Reporting Bugs

If you encounter unexpected behavior or a crash:
1. Search [existing issues](https://github.com/covarianttensor/CasterSoundboard/issues) to ensure the problem has not already been reported.
2. Open a new issue using our **[Bug Report Template](https://github.com/covarianttensor/CasterSoundboard/issues/new?template=bug_report.yml)**.
3. Include your Linux distribution, desktop environment (Wayland vs. X11), audio system (PipeWire vs. PulseAudio), and how you installed Caster Soundboard (AppImage, Flatpak, NixOS, or native build).
4. Run `CasterSoundboard` from a terminal and include any error messages or backtraces.

---

## 💡 Suggesting Features

Have an idea for a new feature or improvement?
1. Check [existing issues and pull requests](https://github.com/covarianttensor/CasterSoundboard/issues) to avoid duplicate proposals.
2. Open an issue using our **[Feature Request Template](https://github.com/covarianttensor/CasterSoundboard/issues/new?template=feature_request.yml)**.
3. Describe the problem you are solving, your proposed workflow or interface change, and any mockups or alternatives you have considered.

---

## 🛠️ Development & Testing Protocol

Before submitting a pull request, ensure your code compiles cleanly and passes all automated unit tests:

```bash
# 1. Configure and build
cmake -B build-cmake -DCMAKE_BUILD_TYPE=Debug
cmake --build build-cmake -j$(nproc)

# 2. Run test suite (must pass 100%)
ctest --test-dir build-cmake --output-on-failure
```

For headless visual testing without an X11 or Wayland display, use our built-in screenshot flags:
```bash
./build-cmake/bin/CasterSoundboard -s /tmp/screenshot_main.png
./build-cmake/bin/CasterSoundboard --view 1 --test-routing-active -s /tmp/screenshot_routing.png
```

---

## 📋 Pull Request Guidelines

1. Create a focused feature branch off `main` (`git checkout -b feat/my-enhancement`).
2. Adhere to our [Conventional Commits](https://www.conventionalcommits.org/) format (`feat:`, `fix:`, `refactor:`, `test:`, `docs:`).
3. Do not introduce legacy Qt Widgets into active views; use declarative QML (`QtQuick` / `QuickControls 2`).
4. Ensure cross-platform packaging compatibility (Native CMake, AppImage, Flatpak, and NixOS).
5. Open your pull request against `main` and complete the PR checklist.
