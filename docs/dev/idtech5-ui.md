# idtech5-ui companion integration

Status: runtime integration in progress, 8 September 2026. Work is on branch `idtech5-ui`, created from
the current `android-gles` checkout to retain Android/GLES support.

The engine's [replacement plan](https://github.com/themuffinator/openQ4/blob/idtech5-ui/docs/dev/plans/idtech5-ui.md)
and [visual specification](https://github.com/themuffinator/openQ4/blob/idtech5-ui/docs/dev/ui-visual-design.md)
define the complete objective: replace all current GUI with scalable,
aspect-expanding vector interfaces and deliver an extensive visual editor.
The specification precedes visual implementation. The runtime is not implemented
by this planning change.

## Canonical game work

- Make SP/MP state/event and interface changes in this repository, never in an
  engine-side game mirror. Keep both game targets buildable throughout migration.
- Preserve GUI state dictionaries, named events, commands, per-entity instances,
  save/checkpoint/restore behavior and gameplay timing.
- Replace direct `idWindow`/`GetDesktop()` accesses with semantic queries and
  actions coordinated with the engine. A null-returning adapter is not parity.
- Preserve HUD, scope, vehicle, terminal, chat, join/team, scoreboard, buy, Arena,
  Match Control and every existing game-specific GUI behavior.
- Retain source-derived family appearance and localized keys while translating
  all GUI; complex imagery alone can remain bitmap artwork.
- Keep editor previews and game events consistent without OS input injection.

## Stage and publication contract

The engine plan owns stage definitions and exit evidence. Commit and push this
repository after each stage that changes canonical companion files, recording
paired exact engine/game revisions. A stage with no game changes should state
that fact rather than create an empty commit. Build and stage both modules with
the established Meson workflow. Do not merge or release before qualification.

Qualification requires SP and MP gameplay, terminal-triggered commands and
save restoration, HUD/weapon/vehicle transitions, dedicated compatibility and
the editor save/package/run round trip. A clean main menu is insufficient.

## Progress

- Stage 0: companion branch and integration contract created. No source or ABI
  changes yet. Full runtime, GUI migration, editor and qualification remain.
- Composition checkpoint: game API 47 accompanies renderer API 13. The shared
  `ClearRenderTarget` call now accepts alpha so UI layers start transparent.
  Its default remains opaque for existing SP/MP scene clears. Rebuild and ship
  engine, renderer modules and both game modules together; mismatched game APIs
  are rejected during loading. This is runtime infrastructure, not GUI migration.
- Input ownership: `SE_RETAINED_UI` is appended to the shared event enum for an
  engine-owned ordered input payload. Existing event values and `sysEvent_t`
  layout are unchanged, and the engine consumes these events before legacy/game
  dispatch. Game API 47 and renderer API 13 stay unchanged. Both SP/MP modules
  are rebuilt with the coordinated header. The engine's
  [input checkpoint](https://github.com/themuffinator/openQ4/blob/idtech5-ui/docs/dev/ui/input-routing.md)
  records session ownership and gameplay timing checks; semantic game dispatch,
  per-entity UI instances and full GUI migration remain pending.
- Header qualification: the companion `engineWindowState_t` copy now includes
  the engine's display scale and window-to-framebuffer density fields. Game code
  does not currently read this global, but the shared declaration must match.
  CI pins the published companion revision and checks every shared header
  against that exact revision as well as the local workspace.
