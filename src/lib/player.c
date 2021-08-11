/*
 * Copyright (c) 2020 Devin Smith <devin@devinsmith.net>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <stdio.h>

#include "player.h"

// Much of the source of this material comes from the following blog:
// https://dewimorgan.livejournal.com/35639.html
//
// Player data is loaded from DATA1. The characters are stored at the following offsets:
// 2E26, 3036, 3226, 3426, 3626 (and onwards)

struct skill_info {
  unsigned char arcane_lore; // 0x20
  unsigned char cave_lore; // 0x21
  unsigned char forest_lore; // 0x22
  unsigned char mountain_lore; // 0x23
  unsigned char town_lore; // 0x24
  unsigned char bandage; // 0x25
  unsigned char climb; // 0x26
  unsigned char fistfighting; // 0x27
  unsigned char hide; // 0x28
  unsigned char lockpick; // 0x29
  unsigned char pickpocket; // 0x2A
  unsigned char swim; // 0x2B
  unsigned char tracking; // 0x2C
  unsigned char bureaucracy; // 0x2D
  unsigned char druid_magic; // 0x2E
  unsigned char high_magic; // 0x2F
  unsigned char low_magic; // 0x30
  unsigned char merchant; // 0x31
  unsigned char sun_magic; // 0x32
  unsigned char axe; // 0x33
  unsigned char flail; // 0x34
  unsigned char mace; // 0x35
  unsigned char sword; // 0x36
  unsigned char two_handed_sword; // 0x37
  unsigned char bow; // 0x38
  unsigned char crossbow; // 0x39
  unsigned char thrown_weapons; // 0x3A
};

// Every spell byte below is a bitmask.
struct spell_info {
  // 0x01 - Elvar's Fire
  // 0x02 - Fire Light
  //  (Low Magic)
  // 0x04 - Mage Light
  // 0x08 - Lesser Heal
  // 0x10 - Luck
  // 0x20 - Charm
  // 0x40 - Disarm
  // 0x80 - Mage Fire
  unsigned char spell1; // 0x3C

  // 0x01 - Vorn's Guard
  // 0x02 - Sala's Swift
  // 0x04 - Reveal Glamor
  // 0x08 - Mystic Might
  // 0x10 - Dazzle
  // 0x20 - Big Chill
  // 0x40 - Ice Chill
  // 0x80 - Poog's Vortex
  unsigned char spell2; // 0x3D

  // 0x01 - Water Summon
  // 0x02 - Earth Summon
  // 0x04 - Air Summon
  // 0x08 - Sense Traps
  // 0x10 - Cloak Arcane
  // 0x20 - Group Heal
  // 0x40 - Healing
  // 0x80 - Cowardice
  unsigned char spell3; // 0x3E

  // 0x01 - Greater Healing
  // 0x02 - Brambles
  // 0x04 - Scare
  // 0x08 - Whirlwind
  // 0x10 - Insect Plague
  // 0x20 - Fire Blast
  // 0x40 - Death Curse
  // (High magic)
  // 0x80 - Fire Summon
  unsigned char spell4; // 0x3F

  // 0x01 - Exorcism
  // 0x02 - Sunstroke
  // (Druid magic)
  // 0x04 - Wood Spirit
  // 0x08 - Beast Call
  // 0x10 - Invoke Spirit
  // 0x20 - Soften Stone
  // 0x40 - Create Wall
  // 0x80 - Cure All
  unsigned char spell5; // 0x40

  // 0x01 - Exorcism
  // 0x02 - Sunstroke
  // 0x04 - Wood Spirit
  // 0x08 - Beast Call
  // 0x10 - Invoke Spirit
  // 0x20 - Soften Stone
  // 0x40 - Create Wall
  // 0x80 - Cure All
  unsigned char spell6; // 0x41

  // 0x01 - Mithras' Bless
  // 0x02 - Column of Fire
  // 0x04 - Battle Power
  // 0x08 - Holy Aim
  // 0x10 - Inferno
  // 0x20 - Fire Storm
  // 0x40 - Wrath of Mithras
  // 0x80 - Rage of Mithras
  unsigned char spell7; // 0x42

  // 0x01 - Radiance
  // 0x02 - Guidance
  // 0x04 - Disarm Trap
  // (Sun Spells)
  // 0x08 - Major Heal
  // 0x10 - Heal
  // 0x20 - Sun Light
  // 0x40 - Armor of Light
  // 0x80 - Light Flash
  unsigned char spell8; // 0x43

  // 0x01 - [Not used]
  // 0x02 - [Not used]
  // 0x04 - [Not used]
  // (Misc spells)
  // 0x08 - Poison
  // 0x10 - Kill Ray
  // 0x20 - Zak's Speed
  // 0x40 - Charger
  // 0x80 - Summon Salamander
  unsigned char spell9; // 0x44
};

struct item_info {
  unsigned char wielded;   // 8 if wielded, 0 if not.
  unsigned char req1;      // Bits 2 & 3 are the stat required to use the
                           // weapon. 0x02=Dex, 0x04=Int, 0x06=Spi. Bit 0
                           // seems to do nothing, and using any higher bits
                           // in the byte gives garbage skillnames when
                           // taking the item to be evaluated at a trader.

  unsigned char req2;      // Bits 1-5 are how many points are required for the
                           // above skill, 0 to 31. Don't know what the top 3
                           // bits are for, but setting them does not affect the
                           // displayed skill requirement when evaluating.
  unsigned char unknown1;  // Seen as 0x00 (hand axe) and 0x10 (battle axe,
                           // dagger, shortsword).
  unsigned char type;      // See below.
  unsigned char unknown2;  // Unknown. Only seen as 0x80.
  unsigned char unknown3;  // Unknown. Only seen as 0x00.
  unsigned char unknown4;  // Unknown. Seen as 0x40 (hand axe),
                           // 0x80 (battle axe), 0x00 (dagger), 0x20 (shortsword).
  unsigned char unknown5;  // Unknown. Only seen as 0x00.
  unsigned char unknown6;  // Unknown. Only seen as 0x01.
  unsigned char name[12];  // All bytes, except the last byte have the high bit set.
};

// Item types
// 0x00 - General Item
// 0x01 - Shield
// 0x02 - Full Shield
// 0x03 - Axe
// 0x04 - Flail
// 0x05 - Sword
// 0x06 - Two-handed sword
// 0x07 - Mace
// 0x08 - Bow
// 0x09 - Crossbow
// 0x0A - Gun
// 0x0B - Thrown weapon
// 0x0C - Ammunition
// 0x0D - Gloves
// 0x0E - Mage Gloves
// 0x0F - Ammo Clip
// 0x10 - Cloth Armor
// 0x11 - Leather Armor
// 0x12 - Cuir Bouilli Armor
// 0x13 - Brigandine Armor
// 0x14 - Scale Armor
// 0x15 - Chain Armor
// 0x16 - Plate And Chain Armor
// 0x17 - Full plate Armor
// 0x18 - Helmet
// 0x19 - Scroll
// 0x1A - Pair of Boots
// 0x1B and above - blank (no text).


// This should be 512 (0x200) bytes long.
struct player_record {
  // All bytes, except the last byte have the high bit set.
  // "Muskels" => 4D75736B656C73
  //  | 0x80   => CDF5F3EBE5EC73
  unsigned char name[12];

  // Stats
  unsigned char strength; // 0x0C
  unsigned char max_strength; // 0x0D
  unsigned char dexterity; // 0x0E
  unsigned char max_dexterity; // 0x0F
  unsigned char intel; // 0x10
  unsigned char max_intel; // 0x11
  unsigned char spirit; // 0x12
  unsigned char max_spirit; // 0x13
  unsigned short health; // 0x14,0x15
  unsigned short max_health; // 0x16,0x17
  unsigned short stun; // 0x18-0x19
  unsigned short max_stun; // 0x1A-0x1B
  unsigned short power; // 0x1C-0x1D
  unsigned short max_power; // 0x1E-0x1F

  // Skills
  struct skill_info skills; // 0x20 -> 0x3A
  unsigned char advancement_points; // 0x3B

  struct spell_info spells; // 0x3C -> 0x44

  unsigned char unknown[8]; // 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4A, 0x4B
  unsigned char status; // bitfield (0 = Ok, 1 = dead, 2 = chained, 4 = poisoned)   0x4C
  unsigned char unknown_byte; // always 0 ? 0x4D
  unsigned char gender; // 0 = male, 1 = female // 0x4E
  unsigned short level; // 0x4F-0x50
  unsigned int xp; // 0x51, 0x52, 0x53, 0x54
  unsigned int gold; // 0x55, 0x56, 0x57, 0x58
  unsigned char armor; // 0x59
  unsigned char defense; // 0x5A
  unsigned char armor_class; // 0x5B
  unsigned char unknown_byte2; // 0x5C

  unsigned char padding[143];

  struct item_info inventory_items[12];
};

// In the dragon.com implementation this occupies 0E73:0000-0DFF, but in the
// COM file it's at 0x1DD:C960 (where CS = 0x1DD)
//
// This is because it is calculated as
// ax = 0xC960 >> 4
// mov bx, cs
// add ax, bx
//
// Where CS is 0x1DD
//    0xE73 -     (0xC960 >> 4) + 0x1DD
//
// 01DD:C960 -> 0E73:0000
//
// This is character data. A Dragon Wars party can be 7 people.
// Each character uses 512 bytes (0x200) so 512 * 7 = 0xE00
static unsigned char data_C960[0xE00] = { 0 };

#define SIZE_OF_PLAYER 512

unsigned char *get_player_data_base()
{
  return data_C960;
}

unsigned char *get_player_data(int player)
{
  size_t offset = player * SIZE_OF_PLAYER;
  return data_C960 + offset;
}

const char *player_property_name(int prop_idx)
{
  switch (prop_idx) {
  case 0x0C:
    return "Strength";
    break;
  case 0x0D:
    return "Max Strength";
    break;
  case 0x0E:
    return "Dexterity";
    break;
  case 0x0F:
    return "Max Dexterity";
    break;
  case 0x4C:
    return "Status";
    break;
  }

  return "Unknown Property";
}
