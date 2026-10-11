# Elite 4 Randomizer

Start at the beginning of the Elite 4 with a random party and a set of basic items to help.

- `src/mod.c`: intro skip (`e4r_skip_intro`), new-game hook (`e4r_new_game`) and the Lorelei auto-start event script / ON_FRAME table.
- `build.py`: code layer (hooks `StartNewGameScene` and `WarpToPlayersRoom`, repoints Lorelei's ON_FRAME table).
- `edits/loadout.json`: team levels / fixed species / held items (`e4r_team`) and gift items (`e4r_items`).
- `test/`: harness regression tests. Details: `patches/elite-4-randomizer.md`.
