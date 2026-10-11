# Elite 4 randomizer v1.0

Start at the beginning of the Elite 4 with a random party and a set of basic items to help

## Base ROM

- **Pokemon - FireRed Version (USA, Europe) (Rev 1)** (POKEMON FIRE, game code BPRE)
- CRC32 `84ee4776`, SHA-1 `dd5945db9b930750cb39d00c84da8571feebf417`

## How to apply

Patch your own copy of the game with `elite-4-randomizer-v1.0.bps` using [Rom Patcher JS](https://www.marcrobledo.com/RomPatcher.js/) or [Floating IPS](https://github.com/Alcaro/Flips). The BPS patch checks the ROM and refuses a different version; `elite-4-randomizer-v1.0.ips` is the same patch for older tools (no check: make sure your ROM matches the checksums above).

## Changes in this version

- Start a new game and go straight into the Elite Four. Pick NEW GAME, Professor Oak's intro is skipped, and you appear in Lorelei's room with her battle already starting. You play as RED and your rival is GREEN.
- You get a random team of six different Pokémon at level 60, drawn from all 386 species. Each one knows its normal level-up moves, and a new team is rolled every time you start a new game.
- Your bag starts with Full Restore ×5, Max Revive ×3, Full Heal ×3 and Max Elixir ×2. You also have all 8 badges and a Pokédex.
- After Lorelei, the challenge goes on as in the original game: Bruno, Agatha, Lance, the Champion and the Hall of Fame. The Champion uses one of the three starter teams, chosen at random.
- If you lose, you wake up in the Indigo Plateau Pokémon Center instead of Pallet Town. You keep your team and can walk back in to try again, and the League restarts from Lorelei as usual.
- On the Data page you can change each party slot's level, set a fixed species or give it a held item, and change the starting items.
- Requires Pokémon FireRed (USA, Europe) Rev 1 (v1.1). It does not work on v1.0.

## Tests

4/4 regression tests passed at release.

## Details

On the title screen press START and choose **NEW GAME**. Professor Oak's intro (controls guide,
gender and name selection) is skipped. You are a boy named **RED** and your rival is **GREEN** (the
game's own first preset names). You appear in **Lorelei's room**, facing her, and her battle starts
right away. You have:

- **A random party of six**, all **level 60**. They are different, picked from all 386 species, and
  rerolled on every new game. Moves are the species' normal level-up moves for that level.
- **Gift items in the bag:** Full Restore ×5, Max Revive ×3, Full Heal ×3, Max Elixir ×2.
- **All 8 badges, a Pokédex, and the POKéMON menu.** The Champion (your rival) gets a random one of
  the three starter teams.

After Lorelei, the run continues as in the original game: Bruno, Agatha, Lance, the Champion, then the
Hall of Fame. If you **lose**, you wake up in the **Indigo Plateau Pokémon Center** (not Pallet Town).
You keep your team and can walk back in for another try. As in the original, the League resets to Lorelei.

### Customising (Data page, edit set "loadout")

| Table | Rows | Fields |
|---|---|---|
| `e4r_team` (0x08EB1F00, 6 × 8 bytes) | party slots 0-5 | `species`: (random) or a fixed species; `level`: 1-100 (0 / 255 means 60); `held_item` |
| `e4r_items` (0x08EB1F40, 8 × 4 bytes) | gift rows | `item` ((none) skips the row), `quantity` (1-999) |

Random slots never duplicate each other or a fixed species. These blocks are only read by this mod's
code. On the clean ROM they are 0xFF free space, which means "random / level 60 / none".

### How it works

0. **Intro skip.** `StartNewGameScene` (0x0812EBA4) is only called by the main menu's NEW GAME. Its
   body is replaced with a long call to `e4r_skip_intro`, which does what Oak's intro would leave
   behind and nothing else:
   - `playerGender` = MALE;
   - `playerName` = `gNameChoice_Red`;
   - `rivalName` = `gNameChoice_Green` (`NewGameInitData` saves and restores it);
   - then `SetMainCallback2(CB2_NewGame)`.
1. **New game hook.** `WarpToPlayersRoom` (0x08054A0C) is static and only called by
   `NewGameInitData`. It is the last step before the field loads: party, bag and flags have just been
   cleared, and the names and trainer ID are already set. Its body is replaced with a long call to
   `e4r_new_game` (C in free space), which:
   - calls `ScriptGiveMon` 6× and `AddBagItem` for each gift row;
   - sets flags 0x820-0x829 (badges, POKéMON, Pokédex);
   - sets `VAR_STARTER_MON` = Random() % 3 (the Champion's team);
   - sets `VAR_MAP_SCENE_POKEMON_LEAGUE` = 2 (a value the game never uses);
   - calls `SetLastHealLocationWarp(HEAL_LOCATION_INDIGO_PLATEAU)`;
   - warps to `MAP_POKEMON_LEAGUE_LORELEIS_ROOM` at (6,6).
2. **Auto-start.** Lorelei's room's map-script list has its ON_FRAME table repointed to a copy holding
   the vanilla entry (scene var 0 → walk in) plus a new one (scene var 2 → `e4r_lorelei_autostart`).
   That event script:
   - does `lockall`, then sets the scene var to 1 and closes the entry door (as the vanilla walk-in does);
   - shows Lorelei's intro text;
   - runs `trainerbattle_no_intro TRAINER_ELITE_FOUR_LORELEI`;
   - then `goto`s Lorelei's own "defeated" script (flag, open door, text).

   It is her talk script without `lock`/`faceplayer`/fame-checker updates, since nobody was talked to.
3. **Whiteout** uses the game's own path: `DoWhiteOut` resets the Elite Four (scene var back to 0) and
   respawns at the Indigo Plateau heal location set in step 1.

### ROM changes

| ROM address | File offset | Original | Patched | Meaning |
|---|---|---|---|---|
| 0x0812EBA4-0x0812EBCF | 0x12EBA4 | `StartNewGameScene` body + literal pool (44 bytes, pool used only by it) | `push {lr}; ldr r0,=e4r_skip_intro; bl _call_via_r0; pop {r0}; bx r0; .word 0x08EB101D`, NOP pad | intro skip |
| 0x08054A0C-0x08054A2B | 0x054A0C | `WarpToPlayersRoom` body (32 bytes) | `push {lr}; ldr r0,=e4r_new_game; bl _call_via_r0; pop {r0}; bx r0; .word 0x08EB107D`, NOP pad | new-game hook |
| 0x081625F9 | 0x1625F9 | `3D 26 16 08` (0x0816263D) | `48 13 EB 08` (0x08EB1348) | Lorelei's room ON_FRAME table → `e4r_lorelei_on_frame` |
| 0x08EB1000-0x08EB1390 | 0xEB1000 | 0xFF | code layer: `e4r_skip_intro` (0x08EB101C), `e4r_new_game` (0x08EB107C), `e4r_lorelei_on_frame` (0x08EB1348), `e4r_lorelei_autostart` (0x08EB1360) | mod code + data |
| 0x08EB1F00-0x08EB1F2F | 0xEB1F00 | 0xFF | `loadout` edit set: level 60 per slot (species/held item left 0xFFFF = random/none) | `e4r_team` |
| 0x08EB1F40-0x08EB1F4F | 0xEB1F40 | 0xFF | `loadout` edit set: 19×5, 25×3, 23×3, 37×2 | `e4r_items` |

Symbols are in `mods/elite-4-randomizer/out/symbols.txt`.

### Verification

All tests start from `re/states/title.ss0` and mash START/A, which picks NEW GAME on the empty save (`rgba test run elite-4-randomizer`):

| Test | Checks |
|---|---|
| `new_game_loadout` | Both hooks and the repointed ON_FRAME pointer. At the hook's return: player name RED, gender male, 6 party mons; the 4 bag items and quantities; badge and menu flags; scene var 2; warp target Lorelei's room (6,6); respawn Indigo Plateau. |
| `lorelei_autostart` | The battle starts with only START/A presses from the title screen, the rival name is GREEN, against trainer 410 (Lorelei, Dewgong Lv52 lead). The player's lead is Lv60. |
| `beat_lorelei` | A forced win (`gBattleOutcome` = 1) sets `FLAG_DEFEATED_LORELEI`, scene var 1, and the door opens. Walking north enters Bruno's room (map 1.76) and his vanilla walk-in. |
| `whiteout` | A forced loss (`gBattleOutcome` = 2) lands in the Indigo Plateau Pokémon Center 1F (map 13.0). The scene var and Lorelei's flag are reset. |

Screenshots during development showed:
- Lorelei's room with her intro text;
- the battle start;
- the post-battle "Go on ahead" with the door open;
- Bruno's room;
- the Pokémon Center whiteout scene.

It was also checked by hand from `main_menu_100pct.ss0` (an existing save: DOWN to NEW GAME, then A), which went straight to the battle. Different input timing gave different random leads (Hitmonlee, Fearow, …), all Lv60.

Not tested in the harness: walking from the Pokémon Center back to Lorelei after a loss (vanilla path;
the door guard only blocks when the national dex is enabled, which a new game doesn't do), and the
later Elite Four battles themselves (unchanged game code).
