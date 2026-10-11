"""elite-4-randomizer: a new game drops you into the Lorelei battle with a random team of six.

Code layer only (the team levels and gift items are the "loadout" edit set, see src/mod.c):
- StartNewGameScene (0x0812EBA4) is rewritten to long-call e4r_skip_intro(), which sets default
  names and goes straight to CB2_NewGame (no Oak intro).
- WarpToPlayersRoom (0x08054A0C, static in src/new_game.c, called only by NewGameInitData) is
  rewritten to long-call e4r_new_game(), which gives the party/items/flags and warps to Lorelei.
- Lorelei's room map-script list (0x081625E4) gets its ON_FRAME table repointed to
  e4r_lorelei_on_frame (the vanilla entry + our auto-start entry).
"""
import modbuild as mb

WARP_TO_PLAYERS_ROOM = 0x08054A0C
START_NEW_GAME_SCENE = 0x0812EBA4  # called only by the main menu's NEW GAME (src/main_menu.c)
LORELEI_ON_FRAME_PTR = 0x081625F9  # 5th entry of PokemonLeague_LoreleisRoom_MapScripts (type 2, ON_FRAME_TABLE)
LORELEI_ON_FRAME = 0x0816263D
LOADOUT_BASE = 0x08EB1F00          # e4r_team / e4r_items tables (edit set), must stay outside the code blob

proj = mb.Project.find()
blob, syms = mb.compile_mod(proj, ["src/mod.c"], entry="e4r_new_game",
                            keep=["e4r_new_game", "e4r_skip_intro", "e4r_lorelei_on_frame"])
assert proj.mod_base + len(blob) <= LOADOUT_BASE, "code overlaps the loadout tables"

p = mb.Patcher(proj.rom_bytes())
p.put(proj.mod_base, blob, expect=b"\xff" * len(blob))


def replace_with_long_call(site: int, orig: bytes, target: int) -> None:
    """Function body -> push {lr}; ldr r0,=target; bl _call_via_r0; pop {r0}; bx r0; .word target|1; NOPs."""
    assert site % 4 == 0
    code = (b"\x00\xb5" + mb.thumb_ldr_pc(0, site + 2, site + 12) + mb.thumb_bl(site + 4, proj.long_call)
            + b"\x01\xbc\x00\x47" + (target | 1).to_bytes(4, "little"))
    p.put(site, code + mb.NOP * ((len(orig) - len(code)) // 2), expect=orig)


replace_with_long_call(WARP_TO_PLAYERS_ROOM,
                       bytes.fromhex("00b581b0012252420620009004200121062300f0bffc00f0b3fc01b001bc0047"),
                       syms["e4r_new_game"])
replace_with_long_call(START_NEW_GAME_SCENE,
                       bytes.fromhex("00b506480021018005480180054848f73dfc0548d1f6cefc01bc0047"
                                     "f8710302f8750302d1eb120889eb1208"), syms["e4r_skip_intro"])

p.put(LORELEI_ON_FRAME_PTR, syms["e4r_lorelei_on_frame"].to_bytes(4, "little"),
      expect=LORELEI_ON_FRAME.to_bytes(4, "little"))

proj.emit("elite-4-randomizer", p, {**{k: v for k, v in syms.items() if k.startswith("e4r_")},
                                    "WarpToPlayersRoom": WARP_TO_PLAYERS_ROOM,
                                    "StartNewGameScene": START_NEW_GAME_SCENE})
