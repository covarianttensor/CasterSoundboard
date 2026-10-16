## Summary of Changes
<!-- Briefly describe the changes introduced in this pull request and why they are needed. -->

## Type of Change
- [ ] Bug fix (non-breaking change resolving an issue)
- [ ] New feature (non-breaking enhancement to functionality)
- [ ] Refactor / Performance optimization
- [ ] Packaging / CI/CD configuration
- [ ] Documentation update

## Verification & Quality Checklist
<!-- Refer to AGENTS.md for architecture context and contribution standards. -->
- [ ] Code compiles with zero errors (`cmake --build build-cmake`)
- [ ] All 8 unit tests pass (`ctest --test-dir build-cmake --output-on-failure`)
- [ ] No hardcoded `/usr/bin/` paths (uses `QStandardPaths::findExecutable` for NixOS/Flatpak compatibility)
- [ ] QML container cards use `HoverHandler` instead of event-swallowing `MouseArea`
- [ ] Commit messages follow conventional commit formatting (`feat:`, `fix:`, `refactor:`, `test:`, `docs:`)

## Related Issues
<!-- Link relevant issues: e.g. Closes #123 -->
