# rommer-gba

Game Boy Advance ROM mods, with the reverse-engineering knowledge behind them: symbol files, data table
and text specs. Each mod comes with its source and regression tests.

**Patches only.** This repo contains no ROMs or game assets. You need your own copy of each game; every
game's README lists the exact ROM (name, CRC32, SHA-1) its patches apply to.

## Applying a patch

Use the `.bps` patch with [Rom Patcher JS](https://www.marcrobledo.com/RomPatcher.js/) (in the browser) or
[Floating IPS](https://github.com/Alcaro/Flips). BPS patches check the ROM and refuse the wrong one; the
`.ips` files are the same patches for older tools.

## Games

| Game | Mods | Code |
|---|---|---|
| [Pokemon FireRed](pokemon-firered/) | 1 | BPRE |

Made with [rommergba](https://github.com/dtand/rommergba).
