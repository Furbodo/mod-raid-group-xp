# mod-raid-group-xp

An [AzerothCore](https://www.azerothcore.org/) module that gives raid-group members the kill XP share they would earn in a 5-player party.

## Why

The core caps the group XP bonus at 5 members and removes it in raid instances, then divides kill XP by the summed level of everyone in range. A 10-player raid group therefore earns roughly half the XP per member that a 5-player party does. That is fine for retail-style raiding, but it punishes servers that run dungeons or open-world content in oversized groups, for example with player bots.

With this module, each member of a raid group gets the same share as a member of a 5-player party of equal average level, times a configurable multiplier.

## What it changes

For kills made by a raid group, the per-member XP rate becomes:

```
partyRate(min(count, 5)) * memberLevel / sumOfLevels * max(count, 5) / 5 * Multiplier
```

where `count` is the number of living group members at reward distance.

Not affected:

- Ordinary parties of 5 or fewer
- Battlegrounds, arenas and battlefield groups
- Player kills
- Quest XP, reputation and loot

## Requirements

An AzerothCore build that provides the `OnPlayerRewardKillRewarder` player hook (`PLAYERHOOK_ON_REWARD_KILL_REWARDER`). Developed and run on the mod-playerbots fork of AzerothCore, September 2026.

## Install

1. Clone into your core's `modules` directory:
   ```
   cd azerothcore-wotlk/modules
   git clone https://github.com/Furbodo/mod-raid-group-xp.git
   ```
2. Re-run CMake and rebuild the core.
3. Copy `mod_raid_group_xp.conf.dist` to `mod_raid_group_xp.conf` in your `etc/modules` directory and adjust it.

No SQL is needed.

## Configuration

| Option | Default | Meaning |
| --- | --- | --- |
| `RaidGroupXP.Enable` | `1` | Turn the module on or off |
| `RaidGroupXP.Multiplier` | `1.15` | Multiplier on the computed share. `1.0` is exactly a 5-player party's share |

Both are read at startup and on `.reload config`.

## Related core settings

Kill XP is only part of running normal content in a raid group. You will probably also want these in `worldserver.conf`:

- `Quests.IgnoreRaid = 1` so raid groups get quest kill credit and quest drops
- `Instance.IgnoreRaid = 1` if raid groups should enter raid instances without restriction

## License

GNU AGPL v3. See [LICENSE](LICENSE).
