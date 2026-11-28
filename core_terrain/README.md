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
- v1.5.0: item definitions and inventory icons for the included blocks; grass/snow layers rebuilt as smooth height gradients.
- v1.5.1: grass layers now replace the surface grass block (placing them in the air above was blocked by grass plants).
- v1.6.0: 8 missing block models restored (palm leaves, End moss, thin logs, pebbles, barnacles, small cactus); grass layer inventory icon fixed; new dirt and moss layers.
- v1.7.0: layers sit on top of the ground again (replacing grass plants) and stack like snow layers when you use one on another; bone meal grows palm saplings (small script).
- v1.8.0: grass and moss layers form steps at the base of one-block ledges; dirt and coarse dirt layers are craft-only (3 blocks in a row -> 6 layers, like snow); layers stack.
