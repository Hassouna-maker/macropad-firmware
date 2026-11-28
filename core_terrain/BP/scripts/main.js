import { world, system, GameMode } from "@minecraft/server";

// ---- stackable layers (like snow layers); a full stack becomes the full block ----
const LAYERS = {
  "ct:grass_layer": "minecraft:grass_block",
  "ct:dirt_layer": "minecraft:dirt",
  "ct:coarse_dirt_layer": "minecraft:coarse_dirt",
  "ct:sand_layer": "minecraft:sand",
  "ct:red_sand_layer": "minecraft:red_sand",
  "ct:moss_layer": "minecraft:moss_block",
  "ct:gravel_layer": "minecraft:gravel",
  "ct:stone_layer": "minecraft:stone",
  "ct:mud_layer": "minecraft:mud",
};

function consumeOne(player) {
  if (player.getGameMode() === GameMode.Creative) return;
  const container = player.getComponent("minecraft:inventory").container;
  const slot = container.getSlot(player.selectedSlotIndex);
  if (slot.amount > 1) slot.amount -= 1;
  else slot.setItem(undefined);
}

// ---- palm saplings ----
function advanceSapling(block, steps) {
  const dim = block.dimension;
  const loc = block.location;
  const state = block.permutation.getState("ct:grow_state");
  const next = (typeof state === "number" ? state : 1) + steps;
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
}

function loadSaplings() {
  try {
    return JSON.parse(world.getDynamicProperty("ct_saplings") ?? "[]");
  } catch (e) {
    return [];
  }
}
function saveSaplings(list) {
  world.setDynamicProperty("ct_saplings", JSON.stringify(list.slice(-300)));
}

world.afterEvents.playerPlaceBlock.subscribe((ev) => {
  if (ev.block.typeId !== "ct:palm_sapling") return;
  const l = ev.block.location;
  const list = loadSaplings();
  list.push([l.x, l.y, l.z, ev.dimension.id]);
  saveSaplings(list);
});

// natural growth: every 30 seconds each tracked sapling has a chance to grow a stage
system.runInterval(() => {
  const list = loadSaplings();
  if (list.length === 0) return;
  const keep = [];
  for (const [x, y, z, dimId] of list) {
    let block;
    try {
      block = world.getDimension(dimId).getBlock({ x, y, z });
    } catch (e) {
      block = undefined;
    }
    if (!block) {
      keep.push([x, y, z, dimId]);
      continue;
    }
    if (block.typeId !== "ct:palm_sapling") continue;
    if (Math.random() < 0.2) {
      advanceSapling(block, 1);
      if (block.typeId === "ct:palm_sapling") keep.push([x, y, z, dimId]);
    } else {
      keep.push([x, y, z, dimId]);
    }
  }
  saveSaplings(keep);
}, 600);

// ---- layer stacking + bone meal ----
world.beforeEvents.playerInteractWithBlock.subscribe((ev) => {
  const { player, block, itemStack } = ev;
  if (!itemStack) return;
  const id = itemStack.typeId;

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

  if (id === "minecraft:bone_meal" && block.typeId === "ct:palm_sapling") {
    ev.cancel = true;
    if (!ev.isFirstEvent) return;
    system.run(() => {
      const loc = block.location;
      consumeOne(player);
      block.dimension.spawnParticle("minecraft:crop_growth_emitter", { x: loc.x + 0.5, y: loc.y + 0.5, z: loc.z + 0.5 });
      advanceSapling(block, 1 + Math.floor(Math.random() * 2));
    });
  }
});

// ---- palm leaf decay: breaking a palm log lets nearby leaves with no log within 4 blocks fall ----
const PALM_LEAVES = new Set(["ct:palm_leaves", "ct:hanging_palm_leaves"]);
function logNear(dim, p) {
  for (let dx = -4; dx <= 4; dx++)
    for (let dy = -4; dy <= 4; dy++)
      for (let dz = -4; dz <= 4; dz++) {
        const b = dim.getBlock({ x: p.x + dx, y: p.y + dy, z: p.z + dz });
        if (b && b.typeId === "ct:palm_log") return true;
      }
  return false;
}
function* decay(dim, c) {
  let n = 0;
  for (let dx = -6; dx <= 6; dx++)
    for (let dy = -6; dy <= 6; dy++)
      for (let dz = -6; dz <= 6; dz++) {
        const p = { x: c.x + dx, y: c.y + dy, z: c.z + dz };
        const b = dim.getBlock(p);
        if (b && PALM_LEAVES.has(b.typeId) && !logNear(dim, p)) b.setType("minecraft:air");
        if (++n % 12 === 0) yield;
      }
}
world.afterEvents.playerBreakBlock.subscribe((ev) => {
  if (ev.brokenBlockPermutation.type.id !== "ct:palm_log") return;
  system.runJob(decay(ev.dimension, ev.block.location));
});

// ---- ambient particles: falling leaves under tree canopies, pollen over flowers ----
const FLOWERS = new Set(["minecraft:dandelion", "minecraft:poppy", "minecraft:azure_bluet", "minecraft:oxeye_daisy", "minecraft:cornflower", "minecraft:lilac", "minecraft:peony", "minecraft:rose_bush"]);
system.runInterval(() => {
  for (const player of world.getAllPlayers()) {
    const dim = player.dimension;
    const p = player.location;
    for (let i = 0; i < 2; i++) {
      const x = Math.floor(p.x + (Math.random() - 0.5) * 28);
      const z = Math.floor(p.z + (Math.random() - 0.5) * 28);
      let top;
      try {
        top = dim.getTopmostBlock({ x, z });
      } catch (e) {
        top = undefined;
      }
      if (!top) continue;
      if (top.typeId.endsWith("_leaves") && Math.random() < 0.5) {
        dim.spawnParticle("ct:falling_leaf", { x: x + 0.5, y: top.y - 0.1, z: z + 0.5 });
      } else if (FLOWERS.has(top.typeId) && Math.random() < 0.6) {
        dim.spawnParticle("ct:pollen", { x: x + 0.5, y: top.y + 0.6, z: z + 0.5 });
      }
    }
  }
}, 60);
