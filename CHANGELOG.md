# Changelog
All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

## [1.1.2] - 2019-11-23
### Changed
- Rename `mdd_time` into `mdd_timer` to align more with the other commands of the timer hud.

## [1.1.1] - 2019-11-22
### Changed
- Don't bloat q3config.cfg and improve cross-platform config compatibility by only archiving cvars that have non-default values.

## [1.1.0] - 2019-11-22
### Added
- New cvar `mdd_fov`. When different from `0`, CampingGaz-HUD and Snap-HUD will use this fov instead of `cg_fov`.

### Removed
- Bounding box trigger display since engines support this now (`scr_triggers_draw 1` and `scr_clips_draw 1`).

## [1.0.0] - 2019-11-21
### Added
- Ammo and jump hud extended from [q3df](https://github.com/q3df/cgame_proxymod).
- Grenade, rocket, timer, trigger huds extended from [krsh732](https://github.com/krsh732/cgame_proxymod).
- Option to filter other player sounds extended from [krsh732](https://github.com/krsh732/cgame_proxymod).
- Initial CampingGaz-HUD which is more accurate/flexible than `df_hud_cgaz`.
- Initial Snap-HUD which is more accurate/flexible than `scr_hud_snap`.

[Unreleased]: https://github.com/Jelvan1/cgame_proxymod/compare/v1.1.2...HEAD
[1.1.2]: https://github.com/Jelvan1/cgame_proxymod/compare/v1.1.1...v1.1.2
[1.1.1]: https://github.com/Jelvan1/cgame_proxymod/compare/v1.1.0...v1.1.1
[1.1.0]: https://github.com/Jelvan1/cgame_proxymod/compare/v1.0.0...v1.1.0
[1.0.0]: https://github.com/Jelvan1/cgame_proxymod/releases/tag/v1.0.0
