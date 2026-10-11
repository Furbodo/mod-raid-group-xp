/*
 * mod-raid-group-xp
 *
 * Gives raid-group members the kill XP share they would get in a 5-player
 * party. The core caps the group XP bonus at 5 members (1.4x) and drops it to
 * 1.0x in raid instances, then divides by the summed level of everyone at
 * reward distance, so a 10-player group earns roughly half per member.
 *
 * This replaces the per-member rate with:
 *   partyRate(min(count, 5)) * level / aliveSumLevel * max(count, 5) / 5 * Multiplier
 * i.e. the same share a member of an equal-average-level 5-man party gets,
 * scaled by RaidGroupXP.Multiplier. Battlegrounds and PvP kills are left untouched.
 */

#include "Config.h"
#include "Formulas.h"
#include "Group.h"
#include "KillRewarder.h"
#include "Player.h"
#include "ScriptMgr.h"

namespace
{
    bool  s_enabled    = true;
    float s_multiplier = 1.15f;
}

class RaidGroupXP_WorldScript : public WorldScript
{
public:
    RaidGroupXP_WorldScript() : WorldScript("RaidGroupXP_WorldScript", { WORLDHOOK_ON_AFTER_CONFIG_LOAD }) { }

    void OnAfterConfigLoad(bool /*reload*/) override
    {
        s_enabled    = sConfigMgr->GetOption<bool>("RaidGroupXP.Enable", true);
        s_multiplier = sConfigMgr->GetOption<float>("RaidGroupXP.Multiplier", 1.15f);
        if (s_multiplier < 0.0f)
            s_multiplier = 0.0f;
    }
};

class RaidGroupXP_PlayerScript : public PlayerScript
{
public:
    RaidGroupXP_PlayerScript() : PlayerScript("RaidGroupXP_PlayerScript", { PLAYERHOOK_ON_REWARD_KILL_REWARDER }) { }

    void OnPlayerRewardKillRewarder(Player* player, KillRewarder* rewarder, bool /*isDungeon*/, float& rate) override
    {
        if (!s_enabled)
            return;

        Player* killer = rewarder->GetKiller();
        Unit* victim = rewarder->GetVictim();
        if (!killer || !victim || victim->IsPlayer())
            return;

        Group* group = killer->GetGroup();
        if (!group || !group->isRaidGroup() || group->isBGGroup() || group->isBFGroup())
            return;

        if (killer->GetMap() && killer->GetMap()->IsBattlegroundOrArena())
            return;

        // Recount exactly as KillRewarder::_InitGroupData does (its fields are private).
        uint32 count = 0;
        uint32 aliveSumLevel = 0;
        for (GroupReference* itr = group->GetFirstMember(); itr != nullptr; itr = itr->next())
        {
            Player* member = itr->GetSource();
            if (!member || !member->IsAlive())
                continue;
            if (member != killer && !member->IsAtGroupRewardDistance(victim))
                continue;

            ++count;
            aliveSumLevel += LevelForXP(member);
        }

        if (!count || !aliveSumLevel)
            return;

        uint32 partySize = std::min<uint32>(count, 5);
        float partyRate = Acore::XP::xp_in_group_rate(partySize, false);
        rate = partyRate * float(LevelForXP(player)) / float(aliveSumLevel) * float(std::max<uint32>(count, 5)) / 5.0f
            * s_multiplier;
    }

private:
    static uint8 LevelForXP(Player const* player)
    {
        uint8 level = player->GetLevel();
        sScriptMgr->OnPlayerBeforeGetLevelForXPGain(player, level);
        return level;
    }
};

void AddRaidGroupXPScripts()
{
    new RaidGroupXP_WorldScript();
    new RaidGroupXP_PlayerScript();
}
