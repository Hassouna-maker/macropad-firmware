import { world, system, GameMode } from "@minecraft/server";

// Layer blocks stack like snow layers; a full stack becomes the matching full block.
const LAYERS = {
  "ct:grass_layer": "minecraft:grass_block",
  "ct:dirt_layer": "minecraft:dirt",
  "ct:coarse_dirt_layer": "minecraft:coarse_dirt",
  "ct:moss_layer": "minecraft:moss_block",
};

function consumeOne(player) {
  if (player.getGameMode() === GameMode.Creative) return;
  const container = player.getComponent("minecraft:inventory").container;
  const slot = container.getSlot(player.selectedSlotIndex);
  if (slot.amount > 1) slot.amount -= 1;
  else slot.setItem(undefined);
}

world.beforeEvents.playerInteractWithBlock.subscribe((ev) => {
  const { player, block, itemStack } = ev;
  if (!itemStack) return;
  const id = itemStack.typeId;

  // Stack layers: use a layer on the same kind of layer to make it one step thicker.
  if (id in LAYERS && block.typeId === id) {
    ev.cancel = true;
    if (!ev.isFirstEvent) return;
    const level = block.permutation.getState("ct:layer");
    system.run(() => {
      if (typeof level === "number" && level < 7) {
        block.setPermutation(block.permutation.withState("ct:layer", level + 1));
      } else {
        block.setType(LAYERS[id]);
      }
      consumeOne(player);
    });
    return;
  }

  // Bone meal on a palm sapling.
  if (id === "minecraft:bone_meal" && block.typeId === "ct:palm_sapling") {
    ev.cancel = true;
    if (!ev.isFirstEvent) return;
    system.run(() => {
      const dim = block.dimension;
      const loc = block.location;
      consumeOne(player);
      dim.spawnParticle("minecraft:crop_growth_emitter", { x: loc.x + 0.5, y: loc.y + 0.5, z: loc.z + 0.5 });
      const state = block.permutation.getState("ct:grow_state");
      const next = (typeof state === "number" ? state : 1) + 1 + Math.floor(Math.random() * 2);
      if (next < 4) {
        block.setPermutation(block.permutation.withState("ct:grow_state", next));
        return;
      }
      const sapling = block.permutation;
      block.setType("minecraft:air");
      let grown = false;
      try {
        grown = dim.placeFeature("ct:palm_tree_weighted", loc);
      } catch (e) {
        grown = false;
      }
      if (!grown) block.setPermutation(sapling.withState("ct:grow_state", 3));
    });
  }
});
