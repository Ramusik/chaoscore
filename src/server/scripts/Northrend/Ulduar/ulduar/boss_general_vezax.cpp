/*
 * Copyright (C) 2008-2011 TrinityCore <http://www.trinitycore.org/>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "ScriptPCH.h"
#include "SpellAuraEffects.h"
#include "ulduar.h"

enum Yells
{
    SAY_AGGRO                                   = -1603290,
    SAY_SLAY_1                                  = -1603291,
    SAY_SLAY_2                                  = -1603292,
    SAY_KITE                                    = -1603293,
    SAY_DEATH                                   = -1603294,
    SAY_BERSERK                                 = -1603295,
    SAY_HARDMODE_ON                             = -1603296,
};

enum Emotes
{
    EMOTE_VAPORS                                = -1603289,
    EMOTE_ANIMUS                                = -1603297,
    EMOTE_BARRIER                               = -1603298,
    EMOTE_SURGE_OF_DARKNESS                     = -1603299,
};

enum Achievments
{
    ACHIEVEMENTS_SMELL_OF_SARONIT_IN_THE_MORNING_10 = 3181,
    ACHIEVEMENTS_SMELL_OF_SARONIT_IN_THE_MORNING_25 = 3188,
    ACHIEVEMENTS_SHADOWDODGER_10                    = 2996,
    ACHIEVEMENTS_SHADOWDODGER_25                    = 2997,
};

enum Spells
{
    // General Vezax
    SPELL_AURA_OF_DESPAIR                       = 62692, // on combat start
    SPELL_AURA_OF_DESPAIR_EFFEKT_DESPAIR        = 64848, // dont know if needet ... need test
    SPELL_CORRUPTED_RAGE                        = 68415,
    SPELL_MARK_OF_THE_FACELESS                  = 63276, // Unknown Aura
    SPELL_MARK_OF_THE_FACELESS_LEECH            = 63278, // Leech Health 1 ... need custom cast
    SPELL_SARONIT_BARRIER                       = 63364, // Script Effekt, Apply while Saronit Animus spawned
    SPELL_SEARING_FLAMES                        = 62661,
    SPELL_SHADOW_CRASH                          = 62660, // Trigger Missile 62659 and 63277
    SPELL_SHADOW_CRASH_DAMAGE                   = 62659, // Explosion Damage
    SPELL_SHADOW_CRASH_AURA                     = 63277, // Triggered Cloud
    SPELL_SURGE_OF_DARKNESS                     = 62662, // every 60 seconds
    SPELL_SUMMON_SARONIT_VARPOR                 = 63081, // every 30 seconds
    // Saronit Animus - Spawnd after 6th Saronit Varpor
    SPELL_PROFOUND_DARKNESS                     = 63420,
    SPELL_VISUAL_SARONITE_ANIMUS                = 63319,
    // Saronit Varpor
    SPELL_SARONIT_VARPOR                        = 63323, // Casted on Death trigger 63322
    SPELL_SARONIT_VARPOR_AURA                   = 63322, // Unknown Aura Dummy needs Scripting ?
    // Player Shaman
    SPELL_SHAMANTIC_RAGE                        = 30823,
    SPELL_BERSERK                               = 47008,
};

enum NPCs
{
    ENTRY_GENERAL_VEZAX                         = 33271,
    ENTRY_SARONIT_VARPOR                        = 33488,
    ENTRY_SARONIT_ANIMUS                        = 33524
};

enum Actions
{
    ACTION_VARPOR_KILLED,
    ACTION_ANIMUS_KILLED
};

class boss_general_vezax : public CreatureScript
{
public:
    boss_general_vezax() : CreatureScript("boss_general_vezax") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new boss_general_vezaxAI(creature);
    }

    struct boss_general_vezaxAI : public BossAI
    {
        boss_general_vezaxAI(Creature* c) : BossAI(c, TYPE_VEZAX)
        {
            VarporList = std::list<uint64>();

            // add interrupt flag
            SpellEntry* tempSpell;
            tempSpell = GET_SPELL(SPELL_SEARING_FLAMES);
            if (tempSpell)
                tempSpell->InterruptFlags |= SPELL_INTERRUPT_FLAG_INTERRUPT;
        }

        std::list<uint64> VarporList;
        uint64 guidSaronitAnimus;

        bool AnimusSummoned;
        bool AnimusKilled;
        bool VarporKilled;
        bool HitByShadowCrash;
        bool ImmunityToReset;
        bool Berserk;
        uint32 SummonVarpor_Timer;
        uint32 SurgeOfDarkness_Timer;
        uint32 ShadowCrash_Timer;
        uint32 SearingFlames_Timer;
        uint32 ResetImmunity_Timer;
        uint32 MarkOfTheFaceless;
        uint32 Berserk_Timer;

        void Reset()
        {
            _Reset();

            VarporList.clear();
            guidSaronitAnimus = 0;
            AnimusSummoned = false;
            HitByShadowCrash = false;
            AnimusKilled = false;
            VarporKilled = false;
            ImmunityToReset = false;
            Berserk = false;

            SurgeOfDarkness_Timer = 60000;
            SummonVarpor_Timer = 30000;
            ShadowCrash_Timer = 10000;
            SearingFlames_Timer = urand(5000, 10000);
            MarkOfTheFaceless = urand(15000, 25000);
            ResetImmunity_Timer = 0;
            Berserk_Timer = 600000;
        }

        void EnterCombat(Unit* /*who*/)
        {
            _EnterCombat();

            //DoCast(me, SPELL_CORRUPTED_RAGE, true);
            DoCast(SPELL_AURA_OF_DESPAIR);
            DoScriptText(SAY_AGGRO, me);
        }

        void SpellHitTarget(Unit* target, const SpellEntry* spell)
        {
            if (target && target->ToPlayer())
            {
                switch (spell->Id)
                {
                    case SPELL_SHADOW_CRASH_DAMAGE:
                        HitByShadowCrash = true;
                        break;
                }
            }
        }

        void JustSummoned(Creature* summoned)
        {
            switch (summoned->GetEntry())
            {
                case ENTRY_SARONIT_VARPOR: 
                    VarporList.push_back(summoned->GetGUID());
                    if (!VarporKilled && VarporList.size() > 5)
                    {
                        summons.DespawnEntry(ENTRY_SARONIT_VARPOR);
                        if (Creature* animus = DoSummon(ENTRY_SARONIT_ANIMUS, me->GetHomePosition(), 30000, TEMPSUMMON_MANUAL_DESPAWN))
                            animus->AI()->AttackStart(me->getVictim());
                    }
                    break;
                case ENTRY_SARONIT_ANIMUS:
                    guidSaronitAnimus = summoned->GetGUID();
                    AnimusSummoned = true;
                    DoScriptText(SAY_HARDMODE_ON, me);
                    DoScriptText(EMOTE_BARRIER, me);
                    me->InterruptNonMeleeSpells(false);
                    me->AddLootMode(LOOT_MODE_HARD_MODE_1);
                    DoCast(SPELL_SARONIT_BARRIER);
                    break;
            }

            summons.Summon(summoned);
            DoZoneInCombat(summoned);
        }

        void KilledUnit(Unit* /*who*/)
        {
            if (urand(0, 5) == 0)
                DoScriptText(RAND(SAY_SLAY_1, SAY_SLAY_2), me);
        }

        void JustDied(Unit* /*who*/)
        {
            DoScriptText(SAY_DEATH, me);

            if (!instance)
                return;

            if (!VarporKilled && AnimusSummoned && AnimusKilled)
                instance->DoCompleteAchievement(RAID_MODE(ACHIEVEMENTS_SMELL_OF_SARONIT_IN_THE_MORNING_10, ACHIEVEMENTS_SMELL_OF_SARONIT_IN_THE_MORNING_25));

            if (!HitByShadowCrash)
                instance->DoCompleteAchievement(RAID_MODE(ACHIEVEMENTS_SHADOWDODGER_10, ACHIEVEMENTS_SHADOWDODGER_25));

            _JustDied();
        }

        void DoAction(const int32 action)
        {
            switch(action)
            {
                case ACTION_VARPOR_KILLED:
                    VarporKilled = true;
                    break;
                case ACTION_ANIMUS_KILLED:
                    AnimusKilled = true;
                    me->RemoveAurasDueToSpell(SPELL_SARONIT_BARRIER);
                    break;
            }
        }

        Unit* CheckPlayersInRange(uint32 PlayersMin, float RangeMin, float RangeMax)
        {
            Map* map = me->GetMap();
            if (map && map->IsDungeon())
            {
                std::list<Player*> PlayerList;
                Map::PlayerList const& Players = map->GetPlayers();
                for (Map::PlayerList::const_iterator itr = Players.begin(); itr != Players.end(); ++itr)
                {
                    if (Player* player = itr->getSource())
                    {
                        if (player->isDead() || player->isGameMaster())
                            continue;

                        float Distance = player->GetDistance(me->GetPositionX(), me->GetPositionY(), me->GetPositionZ());
                        if (Distance < RangeMin || Distance > RangeMax)
                            continue;

                        PlayerList.push_back(player);
                    }
                }

                if (PlayerList.empty())
                    return NULL;

                size_t size = PlayerList.size();
                if (size < PlayersMin)
                    return NULL;

                std::list<Player*>::const_iterator itr = PlayerList.begin();
                std::advance(itr, urand(0, size - 1));
                return *itr;
            }
            else
                return NULL;
        }

        void UpdateAI(const uint32 diff)
        {
            if (!UpdateVictim() )
                return;

            _DoAggroPulse(diff);

            if (!AnimusSummoned)
                if (SummonVarpor_Timer < diff)
                {
                    DoSummon(ENTRY_SARONIT_VARPOR, me, 45.0f, 30000, TEMPSUMMON_MANUAL_DESPAWN);
                    SummonVarpor_Timer = 30000;
                }else SummonVarpor_Timer -= diff;

            if (SurgeOfDarkness_Timer < diff)
            {
                if (!me->IsNonMeleeSpellCasted(false))
                {
                    DoCast(me, SPELL_SURGE_OF_DARKNESS);
                    DoScriptText(SAY_KITE, me);
                    DoScriptText(EMOTE_SURGE_OF_DARKNESS, me);
                    SurgeOfDarkness_Timer = 63000;
                }
            }else SurgeOfDarkness_Timer -= diff;

            if (ShadowCrash_Timer < diff)
            {
                if (!me->IsNonMeleeSpellCasted(false))
                {
                    Unit* target = CheckPlayersInRange(RAID_MODE<uint8>(4, 9), 15.0f, 50.0f);
                    if (!target)
                        target = SelectTarget(SELECT_TARGET_RANDOM, 0, 150.0f, true);
                    if (target)
                        DoCast(target, SPELL_SHADOW_CRASH);

                    ShadowCrash_Timer = 10000;
                }
            }else ShadowCrash_Timer -= diff;

            if (!AnimusSummoned || AnimusKilled)
                if (SearingFlames_Timer < diff)
                {
                    if (!me->IsNonMeleeSpellCasted(false))
                    {
                         me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_INTERRUPT, false);
                         DoCast(SPELL_SEARING_FLAMES);
                         SearingFlames_Timer = urand(10000, 15000);
                         ImmunityToReset = true;
                         ResetImmunity_Timer = 2000;
                    }
                }else SearingFlames_Timer -= diff;

            if (ImmunityToReset)
                if (ResetImmunity_Timer < diff)
                {
                    me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_INTERRUPT, true);
                    ResetImmunity_Timer = 0;
                    ImmunityToReset = false;
                }else ResetImmunity_Timer -= diff;

            if (MarkOfTheFaceless < diff)
            {
                if (!me->IsNonMeleeSpellCasted(false))
                {
                    Unit* target = CheckPlayersInRange(RAID_MODE<uint8>(4, 9), 15.0f, 50.0f);
                    if (!target)
                        target = SelectTarget(SELECT_TARGET_RANDOM, 0, 150.0f, true);
                    if (target)
                        DoCast(target, SPELL_MARK_OF_THE_FACELESS);

                    MarkOfTheFaceless = urand(15000, 25000);
                }
            }else MarkOfTheFaceless -= diff;

            if (!Berserk)
                if (Berserk_Timer < diff)
                {
                    if (!me->IsNonMeleeSpellCasted(false))
                    {
                        DoCast(me, SPELL_BERSERK, true);
                        DoScriptText(SAY_BERSERK, me);
                        Berserk = true;
                    }
                }else Berserk_Timer -= diff;

            DoMeleeAttackIfReady();
        }
    };
};

class mob_saronit_varpor : public CreatureScript
{
    public:
        mob_saronit_varpor() : CreatureScript("mob_saronit_varpor") { }

        struct mob_saronit_varporAI : public ScriptedAI
        {
            mob_saronit_varporAI(Creature* c) : ScriptedAI(c)
            {
                instance = c->GetInstanceScript();
                me->ApplySpellImmune(0, IMMUNITY_EFFECT, SPELL_EFFECT_KNOCK_BACK, true);
                me->ApplySpellImmune(0, IMMUNITY_ID, 49560, true); // Death Grip jump effect
                me->SetReactState(REACT_PASSIVE);
                DoScriptText(EMOTE_VAPORS, me);
            }

            void Reset()
            {
                RandomMove_Timer = urand(5000, 7500);
            }

            void DamageTaken(Unit* /*attacker*/, uint32 &damage)
            {
                if (damage >= me->GetHealth())
                {
                    damage = me->GetHealth() - 1;
                    me->GetMotionMaster()->Clear();
                    me->GetMotionMaster()->MoveIdle();
                    me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_DISABLE_MOVE);
                    me->SetStandState(UNIT_STAND_STATE_DEAD);
                    me->SetHealth(me->GetMaxHealth());
                    me->RemoveAllAuras();
                    me->DespawnOrUnsummon(30000);
                    me->CastSpell(me, SPELL_SARONIT_VARPOR, true);

                    if (instance)
                        if (Creature* vezax = Creature::GetCreature((*me), instance->GetData64(TYPE_VEZAX)))
                            vezax->AI()->DoAction(ACTION_VARPOR_KILLED);
                }
            }

            void UpdateAI(const uint32 diff)
            {
                if (RandomMove_Timer < diff)
                {
                    me->GetMotionMaster()->MoveRandom(30.0f);
                    RandomMove_Timer = urand(5000, 7500);
                }else RandomMove_Timer -= diff;
            }

        private:
            InstanceScript* instance;
            uint32 RandomMove_Timer;
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new mob_saronit_varporAI(creature);
        }
};

class mob_saronit_animus : public CreatureScript
{
    public:
        mob_saronit_animus() : CreatureScript("mob_saronit_animus") { }

        struct mob_saronit_animusAI : public ScriptedAI
        {
            mob_saronit_animusAI(Creature* c) : ScriptedAI(c)
            {
                instance = c->GetInstanceScript();
            }

            void Reset()
            {
                ProfoundDarkness_Timer = 1000;
                DoCast(me, SPELL_VISUAL_SARONITE_ANIMUS);
            }

            void JustDied(Unit* /*killer*/)
            {
                if (instance)
                    if (Creature* vezax = Creature::GetCreature((*me), instance->GetData64(NPC_VEZAX)))
                        vezax->AI()->DoAction(ACTION_ANIMUS_KILLED);
            }

            void UpdateAI(const uint32 diff)
            {
                if (!UpdateVictim() )
                    return;

                if (ProfoundDarkness_Timer < diff)
                {
                    if (!me->IsNonMeleeSpellCasted(false))
                    {
                        DoCast(SPELL_PROFOUND_DARKNESS);
                        ProfoundDarkness_Timer = RAID_MODE(7000, 3000);
                    }
                }else ProfoundDarkness_Timer -= diff;

                DoMeleeAttackIfReady();
            }

        private:
            InstanceScript* instance;
            uint32 ProfoundDarkness_Timer;
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new mob_saronit_animusAI(creature);
        }
};

class spell_general_vezax_aura_of_despair_aura : public SpellScriptLoader
{
public:
    spell_general_vezax_aura_of_despair_aura() : SpellScriptLoader("spell_general_vezax_aura_of_despair_aura") { }

    class spell_general_vezax_aura_of_despair_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_general_vezax_aura_of_despair_AuraScript);

        bool Validate(SpellEntry const* /*spellInfo*/)
        {
            if (!sSpellStore.LookupEntry(SPELL_AURA_OF_DESPAIR_EFFEKT_DESPAIR))
                return false;
            if (!sSpellStore.LookupEntry(SPELL_CORRUPTED_RAGE))
                return false;
            return true;
        }

        void OnApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
        {
            if (GetTarget()->GetTypeId() != TYPEID_PLAYER)
                return;

            Player* target = GetTarget()->ToPlayer();

            if (target->getClass() == CLASS_SHAMAN && target->HasSpell(SPELL_SHAMANTIC_RAGE))
                target->CastSpell(target, SPELL_CORRUPTED_RAGE, true);

            target->CastSpell(target, SPELL_AURA_OF_DESPAIR_EFFEKT_DESPAIR, true);
        }

        void OnRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
        {
            if (GetTarget()->GetTypeId() != TYPEID_PLAYER)
                return;

            Player* target = GetTarget()->ToPlayer();

            target->RemoveAurasDueToSpell(SPELL_CORRUPTED_RAGE);
            target->RemoveAurasDueToSpell(SPELL_AURA_OF_DESPAIR_EFFEKT_DESPAIR);
        }

        void Register()
        {
            OnEffectApply += AuraEffectApplyFn(spell_general_vezax_aura_of_despair_AuraScript::OnApply, EFFECT_0, SPELL_AURA_PREVENT_REGENERATE_POWER, AURA_EFFECT_HANDLE_REAL);
            OnEffectRemove += AuraEffectRemoveFn(spell_general_vezax_aura_of_despair_AuraScript::OnRemove, EFFECT_0, SPELL_AURA_PREVENT_REGENERATE_POWER, AURA_EFFECT_HANDLE_REAL);
        }

    };

    AuraScript* GetAuraScript() const
    {
        return new spell_general_vezax_aura_of_despair_AuraScript();
    }
};

class spell_general_vezax_mark_of_the_faceless_aura : public SpellScriptLoader
{
public:
    spell_general_vezax_mark_of_the_faceless_aura() : SpellScriptLoader("spell_general_vezax_mark_of_the_faceless_aura") { }

    class spell_general_vezax_mark_of_the_faceless_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_general_vezax_mark_of_the_faceless_AuraScript);

        bool Validate(SpellEntry const* /*spellInfo*/)
        {
            if (!sSpellStore.LookupEntry(SPELL_MARK_OF_THE_FACELESS_LEECH))
                return false;
            return true;
        }

        void HandleDummyTick(AuraEffect const* aurEff)
        {
            GetCaster()->CastCustomSpell(SPELL_MARK_OF_THE_FACELESS_LEECH, SPELLVALUE_BASE_POINT1, aurEff->GetAmount(), GetTarget(), true);
        }

        void Register()
        {
            OnEffectPeriodic += AuraEffectPeriodicFn(spell_general_vezax_mark_of_the_faceless_AuraScript::HandleDummyTick, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY);
        }

    };

    AuraScript* GetAuraScript() const
    {
        return new spell_general_vezax_mark_of_the_faceless_AuraScript();
    }
};

/*
UPDATE creature_template SET scriptname = 'boss_general_vezax' WHERE entry = 33271;
UPDATE creature_template SET scriptname = 'mob_saronit_varpor' WHERE entry = 33488;
UPDATE creature_template SET scriptname = 'mob_saronit_animus' WHERE entry = 33524;
DELETE FROM spell_script_names WHERE spell_id = 62692;
INSERT INTO spell_script_names VALUE (62692,'spell_general_vezax_aura_of_despair_aura');
DELETE FROM spell_script_names WHERE spell_id = 63276;
INSERT INTO spell_script_names VALUE (63276,'spell_general_vezax_mark_of_the_faceless_aura');
-- DELETE FROM spell_script_names WHERE spell_id = 63278;
--- INSERT INTO spell_script_names VALUE (63278,'spell_general_vezax_mark_of_the_faceless_spell');
*/

void AddSC_boss_general_vezax()
{
    new boss_general_vezax();
    new mob_saronit_varpor();
    new mob_saronit_animus();
    new spell_general_vezax_aura_of_despair_aura();
    new spell_general_vezax_mark_of_the_faceless_aura();
}
