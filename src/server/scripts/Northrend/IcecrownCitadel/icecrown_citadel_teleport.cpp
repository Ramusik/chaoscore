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

#include "ScriptedGossip.h"
#include "ScriptMgr.h"
#include "InstanceScript.h"
#include "icecrown_citadel.h"
#include "Spell.h"

#define GOSSIP_SENDER_ICC_PORT 631
#define GOSSIP_TEXT(id) sObjectMgr->GetGossipText(id)->Options[0].Text_0
#define IS_TELEPORT_ENABLED(id) ((go->GetMap()->GetGameObject(instance->GetData64(id))->GetGoState() == GO_STATE_ACTIVE) ? true: false)

enum eTeleportGossips
{
    GOSSIP_TELEPORT_LIGHTS_HAMMER           = 800000,
    GOSSIP_TELEPORT_ORATORY_OF_THE_DAMNED   = 800001,
    GOSSIP_TELEPORT_RAMPART_OF_SKULLS       = 800002,
    GOSSIP_TELEPORT_DEATHBRINGERS_RISE      = 800003,
    GOSSIP_TELEPORT_UPPER_SPIRE             = 800004,
    GOSSIP_TELEPORT_SINDRAGOSAS_LAIR        = 800005,
    GOSSIP_TELEPORT_FROZEN_THRONE           = 800006
};

enum TeleportSpells
{
    SPELL_TELEPORT_ICC_LIGHT_S_HAMMER         = 70781,
    SPELL_TELEPORT_ICC_ORATORY_OF_THE_DAMNED  = 70856,
    SPELL_TELEPORT_ICC_RAMPART_OF_SKULLS      = 70857,
    SPELL_TELEPORT_ICC_DEATHBRINGER_S_RISE    = 70858,
    SPELL_TELEPORT_ICC_UPPER_SPIRE            = 70859,
    SPELL_TELEPORT_ICC_SINDRAGOSA_S_LAIR      = 70861,
    SPELL_TELEPORT_ICC_FROZEN_THRONE          = 70860
};

class go_icecrown_teleporter : public GameObjectScript
{

public:
    go_icecrown_teleporter() : GameObjectScript("go_icecrown_teleporter") { }

    bool OnGossipHello(Player* player, GameObject* go)
    {
        InstanceScript* instance = go->GetInstanceScript();
        if(!instance)
            return false;
        if (instance->IsEncounterInProgress())
            return false;

        switch (go->GetEntry())
        {
            case GO_TELEPORT_ORATORY_OF_THE_DAMNED:
                instance->SetData(DATA_TELEPORT_ORATORY_OF_THE_DAMNED_ACTIVATED, DONE);
                break;
            case GO_TELEPORT_RAMPART_OF_SKULLS:
                instance->SetData(DATA_TELEPORT_RAMPART_OF_SKULLS_ACTIVATED, DONE);
                instance->SetData(DATA_TELEPORT_DEATHBRINGER_S_RISE_ACTIVATED, DONE);
                break;
            case GO_TELEPORT_DEATHBRINGER_RISE:
                instance->SetData(DATA_TELEPORT_DEATHBRINGER_S_RISE_ACTIVATED, DONE);
                break;
            case GO_TELEPORT_UPPER_SPIRE:
                instance->SetData(DATA_TELEPORT_UPPER_SPIRE_ACTIVATED, DONE);
                break;
            case GO_TELEPORT_SINDRAGOSA_S_LAIR:
                instance->SetData(DATA_TELEPORT_SINDRAGOSA_S_LAIR_ACTIVATED, DONE);
                break;
            case GO_TELEPORT_FROZEN_THRONE:
                instance->SetData(DATA_TELEPORT_FROZEN_THRONE_ACTIVATED, DONE);
                break;
        }
        instance->HandleGameObject(NULL, true, go);
        if (go->GetEntry() != GO_TELEPORT_LIGHT_S_HAMMER)
        player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_TEXT(GOSSIP_TELEPORT_LIGHTS_HAMMER), GOSSIP_SENDER_MAIN, SPELL_TELEPORT_ICC_LIGHT_S_HAMMER);

        if (go->GetEntry() != GO_TELEPORT_ORATORY_OF_THE_DAMNED && instance->GetData(DATA_LORD_MARROWGAR) == DONE && instance->GetData(DATA_TELEPORT_ORATORY_OF_THE_DAMNED_ACTIVATED) == DONE)
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_TEXT(GOSSIP_TELEPORT_ORATORY_OF_THE_DAMNED), GOSSIP_SENDER_MAIN, SPELL_TELEPORT_ICC_ORATORY_OF_THE_DAMNED);
        if (go->GetEntry() != GO_TELEPORT_RAMPART_OF_SKULLS && instance->GetData(DATA_LADY_DEATHWHISPER) == DONE && instance->GetData(DATA_TELEPORT_RAMPART_OF_SKULLS_ACTIVATED) == DONE)
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_TEXT(GOSSIP_TELEPORT_RAMPART_OF_SKULLS), GOSSIP_SENDER_MAIN, SPELL_TELEPORT_ICC_RAMPART_OF_SKULLS);
        if (go->GetEntry() != GO_TELEPORT_DEATHBRINGER_RISE
            && instance->GetData(DATA_GUNSHIP_EVENT) == DONE
            && instance->GetData(DATA_LADY_DEATHWHISPER) == DONE
            && instance->GetData(DATA_TELEPORT_DEATHBRINGER_S_RISE_ACTIVATED) == DONE
                        //&& IS_TELEPORT_ENABLED(DATA_TELEPORT_DEATHBRINGERS_RISE) //Disabled until Gunship Battle encounter is implemented
            )
        {
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_TEXT(GOSSIP_TELEPORT_DEATHBRINGERS_RISE), GOSSIP_SENDER_MAIN, SPELL_TELEPORT_ICC_DEATHBRINGER_S_RISE);
        }
        if (go->GetEntry() != GO_TELEPORT_UPPER_SPIRE &&
            instance->GetData(DATA_DEATHBRINGER_SAURFANG) == DONE && instance->GetData(DATA_TELEPORT_UPPER_SPIRE_ACTIVATED) == DONE)
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_TEXT(GOSSIP_TELEPORT_UPPER_SPIRE), GOSSIP_SENDER_MAIN, SPELL_TELEPORT_ICC_UPPER_SPIRE);
        if (go->GetEntry() != GO_TELEPORT_SINDRAGOSA_S_LAIR &&
            instance->GetData(DATA_VALITHRIA_DREAMWALKER) == DONE && instance->GetData(DATA_TELEPORT_SINDRAGOSA_S_LAIR_ACTIVATED) == DONE)
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_TEXT(GOSSIP_TELEPORT_SINDRAGOSAS_LAIR), GOSSIP_SENDER_MAIN, SPELL_TELEPORT_ICC_SINDRAGOSA_S_LAIR);
        if (instance->GetData(DATA_BLOOD_QUEEN_LANA_THEL) == DONE && instance->GetData(DATA_PROFESSOR_PUTRICIDE) == DONE && instance->GetData(DATA_SINDRAGOSA) == DONE)
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_TEXT(GOSSIP_TELEPORT_FROZEN_THRONE), GOSSIP_SENDER_MAIN, SPELL_TELEPORT_ICC_FROZEN_THRONE);

        player->SEND_GOSSIP_MENU(DEFAULT_GOSSIP_MESSAGE, go->GetGUID());
        return true;
    }

    bool OnGossipSelect(Player* player, GameObject* /*go*/, uint32 uiSender, uint32 uiAction)
        {
            //player->PlayerTalkClass->ClearMenus();
            if(!player->getAttackers().empty())
                return false;

            SpellEntry const* spell = sSpellStore.LookupEntry(uiAction);
            if (!spell)
                return false;

            if (player->isInCombat())
            {
                Spell::SendCastResult(player, spell, 0, SPELL_FAILED_AFFECTING_COMBAT);
                return true;
            }

            if (uiSender == GOSSIP_SENDER_MAIN)
            {
                //Preload the Lich King's platform before teleporting player to there
                if (uiAction == SPELL_TELEPORT_ICC_FROZEN_THRONE)
                    player->GetMap()->LoadGrid(530.3f, -2122.67f);
                player->CastSpell(player, spell, true);
                //Give him 2 tries, just in case if player will fall through the ground
                if (uiAction == SPELL_TELEPORT_ICC_FROZEN_THRONE)
                    TeleportPlayerToFrozenThrone(player);
            }
            return true;
        }
};


void AddSC_icecrown_citadel_teleport()
{
    new go_icecrown_teleporter();
}
