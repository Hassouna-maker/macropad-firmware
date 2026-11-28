# Core Terrain

World generation extracted from **Core Craft v1.2.9**: the `ct:snowy_dappled_forest` biome, per-biome
ground rules, custom trees (oak, birch, swamp, palm, pale oak, End trees...), vegetation, logs, boulders,
ice features, End terrain/cliffs, and the vanilla-tree resets, plus the structure templates and blocks
they use. No villages, ruins, temples or mineshafts.

- Namespace `cc:` -> `ct:` (don't enable alongside Core Craft in the same world).
- No scripts. Script-driven block behaviours (coconut growth, leaf decay, etc.) are removed.
- Only newly generated chunks are affected: use a new world.
- Source: Core Craft by its original author (CurseForge). Check its license before redistributing.

## v1.4.0 additions (original, not from Core Craft)
- `ct:alpine_peaks` (very high snowy mountains) and `ct:foothills` (rolling forested hills) biomes that replace parts of forest/plains/taiga/mountain areas.
- Natural snow cover: snow layers get thicker with altitude, with a ragged, noise-shifted snow line.
- `ct:grass_layer`: an 8-height grass layer block placed in noise patches on grass for low natural mounds.
- v1.4.1: palm leaf culling rule restored (palms no longer see-through); layers use a deterministic pattern; rare large caverns (`ct:land_big_cavern`, ~3% of chunks); mountain biomes more common. Find mountains with `/locate biome ct:alpine_peaks`.
