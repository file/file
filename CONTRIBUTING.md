<!-- SPDX-License-Identifier: PMPL-1.0-or-later -->
# Contributing

Thank you for your interest in contributing! We follow a "Dual-Track" architecture where human-readable documentation lives in the root and machine-readable policies live in `.machine_readable/`.

## How to Contribute

We welcome contributions in many forms:

- **Code:** Improving the core stack or extensions
- **Documentation:** Enhancing docs or AI manifests
- **Testing:** Adding property-based tests or formal proofs
- **Bug reports:** Filing clear, reproducible issues

## Getting Started

1. **Read the AI Manifest:** Start with `0-AI-MANIFEST.a2ml` (if present) to understand the repository structure.
2. **Environment:** Use `nix develop` or `direnv allow` to set up your tools.
3. **Task Runner:** Use `just` to see available commands (`just --list`).

## Development Workflow

### Branch Naming

```
docs/short-description       # Documentation
test/what-added              # Test additions
feat/short-description       # New features
fix/issue-number-description # Bug fixes
refactor/what-changed        # Code improvements
security/what-fixed          # Security fixes
```

### Commit Messages

We follow [Conventional Commits](https://www.conventionalcommits.org/):

```
<type>(<scope>): <description>

[optional body]

[optional footer]
```

Types: `feat`, `fix`, `docs`, `test`, `refactor`, `ci`, `chore`, `security`

## Reporting Bugs

Before reporting:
1. Search existing issues
2. Check if it's already fixed in `main`

When reporting, include:
- Clear, descriptive title
- Environment details (OS, versions, toolchain)
- Steps to reproduce
- Expected vs actual behaviour

## Code of Conduct

All contributors are expected to adhere to our [Code of Conduct](CODE_OF_CONDUCT.md).

## License

By contributing, you agree that your contributions will be licensed under the same license as the project (see [LICENSE](LICENSE)).
