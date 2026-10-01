# openQ4-game — historical archive

Game-code development moved to **[openQ4](https://github.com/themuffinator/openQ4)** on 1 October 2026.
This repository is retained read-only for its history. Use the openQ4 repository
for current source, builds, issues and contributions.

Canonical Quake 4 SDK-derived single-player and multiplayer sources now build directly inside the engine checkout.

The source locations are **`src/game/` and `src/mpgame/`**. See the
[current build guide](https://github.com/themuffinator/openQ4/blob/main/BUILDING.md),
[campaign guide](https://github.com/themuffinator/openQ4/blob/main/docs/user/campaigns.md) and
[migration details](MIGRATION.md). Current builds require neither archived
companion checkout. All runtime game modules remain under `baseoq4/`.

The import preserves this repository’s Git history and original licence notices.
The [last source snapshot](https://github.com/themuffinator/openQ4-game/tree/4d87dbdb6934307988b371d3c794190a1380f322)
includes the former build instructions for historical investigation.

## Licence

The Quake 4 SDK Limited Use License Agreement remains applicable. Source
consolidation does not relicense this code under the engine’s GPL or establish
new distribution permission. See [LICENSE](LICENSE) and openQ4’s
[component licensing notice](https://github.com/themuffinator/openQ4/blob/main/LICENSING.md).
Retail and expansion assets are supplied separately and are not included here.

## Credits

- [Emile Belanger (emileb)](https://github.com/emileb) — original Android, SigmaTouch and GLES support in [openQ4](https://github.com/emileb/openQ4); the matching renderer configuration header supports the current official integration.
- Upstream Quake4SDK (Quake 4 v1.4.2 SDK baseline)
- id Software
- Raven Software
- openQ4 contributors
