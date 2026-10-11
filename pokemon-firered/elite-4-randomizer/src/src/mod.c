/* elite-4-randomizer: a new game starts in Lorelei's room, straight into her battle, with a
 * random party of six and some healing items.
 *
 * e4r_skip_intro() replaces StartNewGameScene() (main menu NEW GAME): instead of Oak's intro it
 * sets the default names (boy "RED", rival "GREEN") and goes straight to CB2_NewGame.
 * e4r_new_game() replaces WarpToPlayersRoom(), the last thing NewGameInitData() does before
 * the field loads (party, bag and flags have just been cleared; names and trainer ID are set).
 * e4r_lorelei_on_frame is the new ON_FRAME table of Lorelei's room: the vanilla "walk in"
 * entry plus ours, which runs when VAR_MAP_SCENE_POKEMON_LEAGUE == E4R_SCENE_AUTOSTART.
 */
typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef signed char s8;

/* Game functions (pret/pokefirered rev1 addresses). */
#define GAME_FN(addr, ret, ...) ((ret (*)(__VA_ARGS__))((addr) | 1))
#define SetMainCallback2        GAME_FN(0x08000558, void, void (*)(void))
#define CB2_NewGame             GAME_FN(0x08056658, void, void)
#define Random                  GAME_FN(0x08044EDC, u16, void)
#define WarpIntoMap             GAME_FN(0x0805538C, void, void)
#define SetWarpDestination      GAME_FN(0x080553A0, void, s8, s8, s8, s8, s8)
#define SetLastHealLocationWarp GAME_FN(0x080554E0, void, u8)
#define VarSet                  GAME_FN(0x0806E598, u8, u16, u16)
#define FlagSet                 GAME_FN(0x0806E694, u8, u16)
#define AddBagItem              GAME_FN(0x0809A098, u8, u16, u16)
#define ScriptGiveMon           GAME_FN(0x080A0130, u8, u16, u8, u16, u32, u32, u8)

/* Data (pret/pokefirered rev1 addresses). */
#define gSaveBlock1Ptr          (*(u8 *volatile *)0x03005008)
#define gSaveBlock2Ptr          (*(u8 *volatile *)0x0300500C)
#define gPlttBufferUnfaded      ((volatile u16 *)0x020371F8)
#define gPlttBufferFaded        ((volatile u16 *)0x020375F8)
#define gNameChoice_Red         ((const u8 *)0x081C57CE)
#define gNameChoice_Green       ((const u8 *)0x081C57C8)
#define SB2_PLAYER_NAME         0x000
#define SB2_PLAYER_GENDER       0x008
#define SB1_RIVAL_NAME          0x3A4C
#define PLAYER_NAME_LENGTH      7
#define EOS                     0xFF
#define MALE                    0

/* Constants (include/constants/...). */
#define MAP_GROUP_LORELEIS_ROOM      1
#define MAP_NUM_LORELEIS_ROOM        75
#define HEAL_LOCATION_INDIGO_PLATEAU 10
#define VAR_STARTER_MON              0x4031
#define VAR_MAP_SCENE_POKEMON_LEAGUE 0x4068
#define FLAG_BADGE01_GET             0x820
#define FLAG_SYS_POKEMON_GET         0x828
#define FLAG_SYS_POKEDEX_GET         0x829
#define SPECIES_CELEBI               251
#define SPECIES_TREECKO              277  /* 252-276 are the unused "?" placeholder slots */
#define SPECIES_CHIMECHO             411
#define ITEMS_COUNT                  375
#define PARTY_SIZE                   6

#define E4R_SCENE_AUTOSTART          2    /* vanilla only uses 0 (walk in) and 1 (inside) */
#define DEFAULT_LEVEL                60

/* Loadout tables, edited through the "loadout" edit set (re/tables/e4r_team.json, e4r_items.json).
 * They sit outside the code blob; unset rows are the 0xFF free-space fill. */
struct TeamSlot { u16 species; u8 level; u8 unused; u16 held_item; u16 unused2; };
struct ItemGift { u16 item; u16 quantity; };
#define TEAM  ((const struct TeamSlot *)0x08EB1F00)
#define GIFTS ((const struct ItemGift *)0x08EB1F40)
#define NUM_GIFTS 8

static int valid_species(u16 s)
{
    return (s >= 1 && s <= SPECIES_CELEBI) || (s >= SPECIES_TREECKO && s <= SPECIES_CHIMECHO);
}

static u16 random_species(void)
{
    u16 n = (u16)(Random() % (SPECIES_CELEBI + SPECIES_CHIMECHO - SPECIES_TREECKO + 1));
    return n < SPECIES_CELEBI ? n + 1 : n - SPECIES_CELEBI + SPECIES_TREECKO;
}

static void give_team(void)
{
    u16 chosen[PARTY_SIZE];
    int i, j;

    for (i = 0; i < PARTY_SIZE; i++)  /* fixed species first, so random picks can avoid them */
        chosen[i] = valid_species(TEAM[i].species) ? TEAM[i].species : 0;
    for (i = 0; i < PARTY_SIZE; i++)
    {
        while (chosen[i] == 0)
        {
            u16 s = random_species();
            for (j = 0; j < PARTY_SIZE && chosen[j] != s; j++)
                ;
            if (j == PARTY_SIZE)
                chosen[i] = s;
        }
    }
    for (i = 0; i < PARTY_SIZE; i++)
    {
        u8 level = TEAM[i].level;
        u16 held = TEAM[i].held_item;
        if (level == 0 || level > 100)
            level = DEFAULT_LEVEL;
        if (held >= ITEMS_COUNT)
            held = 0;
        ScriptGiveMon(chosen[i], level, held, 0, 0, 0);
    }
}

static void give_items(void)
{
    int i;
    for (i = 0; i < NUM_GIFTS; i++)
    {
        u16 item = GIFTS[i].item, qty = GIFTS[i].quantity;
        if (item == 0 || item >= ITEMS_COUNT)
            continue;
        if (qty == 0 || qty > 999)
            qty = 1;
        AddBagItem(item, qty);
    }
}

static void set_name(u8 *dest, const u8 *src)
{
    int i;
    for (i = 0; i < PLAYER_NAME_LENGTH && src[i] != EOS; i++)
        dest[i] = src[i];
    for (; i < PLAYER_NAME_LENGTH + 1; i++)
        dest[i] = EOS;
}

void e4r_skip_intro(void)
{
    gPlttBufferUnfaded[0] = 0;  /* as StartNewGameScene: backdrop black */
    gPlttBufferFaded[0] = 0;
    gSaveBlock2Ptr[SB2_PLAYER_GENDER] = MALE;
    set_name(gSaveBlock2Ptr + SB2_PLAYER_NAME, gNameChoice_Red);
    set_name(gSaveBlock1Ptr + SB1_RIVAL_NAME, gNameChoice_Green);  /* NewGameInitData keeps it */
    SetMainCallback2(CB2_NewGame);
}

void e4r_new_game(void)
{
    int i;

    give_team();
    give_items();
    for (i = 0; i < 8; i++)
        FlagSet(FLAG_BADGE01_GET + i);
    FlagSet(FLAG_SYS_POKEMON_GET);
    FlagSet(FLAG_SYS_POKEDEX_GET);
    VarSet(VAR_STARTER_MON, Random() % 3);  /* picks the Champion's team */
    VarSet(VAR_MAP_SCENE_POKEMON_LEAGUE, E4R_SCENE_AUTOSTART);
    SetLastHealLocationWarp(HEAL_LOCATION_INDIGO_PLATEAU);  /* whiting out sends you to the League's Pokémon Center */
    SetWarpDestination(MAP_GROUP_LORELEIS_ROOM, MAP_NUM_LORELEIS_ROOM, -1, 6, 6);  /* facing Lorelei (6,5) */
    WarpIntoMap();
}

/* Event script: Lorelei's own talk script without lock/faceplayer/famechecker (nobody was talked to). */
const u8 e4r_lorelei_autostart[] = {
    0x69,                                     /* lockall */
    0x16, 0x68, 0x40, 0x01, 0x00,             /* setvar VAR_MAP_SCENE_POKEMON_LEAGUE, 1 */
    0x04, 0x14, 0x76, 0x1A, 0x08,             /* call PokemonLeague_EventScript_CloseEntry */
    0x25, 0x8E, 0x00,                         /* special DrawWholeMapView */
    0x28, 0x14, 0x00,                         /* delay 20 */
    0x0F, 0x00, 0x1C, 0x80, 0x17, 0x08,       /* msgbox PokemonLeague_LoreleisRoom_Text_Intro */
    0x09, 0x04,
    0x29, 0x03, 0x00,                         /* setflag FLAG_TEMP_3 */
    0x29, 0x05, 0x00,                         /* setflag FLAG_TEMP_5 */
    0x5C, 0x03, 0x9A, 0x01, 0x00, 0x00,       /* trainerbattle_no_intro TRAINER_ELITE_FOUR_LORELEI, */
    0x09, 0x82, 0x17, 0x08,                   /*     PokemonLeague_LoreleisRoom_Text_Defeat */
    0x2A, 0x05, 0x00,                         /* clearflag FLAG_TEMP_5 */
    0x05, 0xEB, 0x26, 0x16, 0x08,             /* goto PokemonLeague_LoreleisRoom_EventScript_DefeatedLorelei */
};

struct MapScript2 { u16 var; u16 value; const void *script; };

const struct MapScript2 e4r_lorelei_on_frame[] = {
    {VAR_MAP_SCENE_POKEMON_LEAGUE, 0, (const void *)0x08162647},  /* vanilla: ..._EventScript_EnterRoom */
    {VAR_MAP_SCENE_POKEMON_LEAGUE, E4R_SCENE_AUTOSTART, e4r_lorelei_autostart},
    {0, 0, 0},
};
