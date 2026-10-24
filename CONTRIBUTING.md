# Contributing

Report reproducible bugs using the issue template. Include alpha version, headset/controller/runtime, GPU/driver, Windows version, whether it reproduces on a clean install, steps and expected/actual behavior. Attach relevant logs after reviewing local paths and usernames. Never upload game archives or personal saves without reviewing their contents.

The supported baseline is original Steam CoP 1.6.02 with Quest 2 / Touch / USB Link. Keep original archives untouched and use a separate save directory. Mark headset evidence separately from unit tests and successful compilation. Do not enable new test kits or unlimited weight in release configuration.

See docs/BUILD-RELEASE.md for build and package steps. Preserve Windows-1251 files and the binary-safe engine patch. Do not publish a candidate before the clean-install checklist has been completed and the project owner has approved publication.
