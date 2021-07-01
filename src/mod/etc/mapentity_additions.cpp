#include "mod.h"
#include "stub/baseentity.h"
#include "stub/entities.h"
#include "stub/gamerules.h"
#include "stub/tfbot.h"
#include "stub/tf_shareddefs.h"
#include "stub/misc.h"
#include "stub/strings.h"
#include "util/scope.h"
#include "util/misc.h"


namespace Mod::Etc::Mapentity_Additions
{

    static const char *SPELL_TYPE[] = {
        "Fireball",
        "Ball O' Bats",
        "Healing Aura",
        "Pumpkin MIRV",
        "Superjump",
        "Invisibility",
        "Teleport",
        "Tesla Bolt",
        "Minify",
        "Meteor Shower",
        "Summon Monoculus",
        "Summon Skeletons"
    };

    void FireFormatInput(CLogicCase *entity, CBaseEntity *activator, CBaseEntity *caller)
    {
        variant_t variant;
        entity->ReadKeyField("Case16", &variant);
        std::string fmtstr = variant.String();
        unsigned int pos = 0;
        unsigned int index = 1;
        while ((pos = fmtstr.find('%', pos)) != std::string::npos ) {
            if (pos != fmtstr.size() - 1 && fmtstr[pos + 1] == '%') {
                fmtstr.erase(pos, 1);
                pos++;
            }
            else
            {
                entity->ReadKeyField(CFmtStr("Case%02d", index), &variant);
                fmtstr.replace(pos, 1, variant.String());
                index++;
                pos += strlen(variant.String());
            }
        }

        variant_t variant1;
        variant1.SetString(AllocPooledString(fmtstr.c_str()));
        entity->m_OnDefault->FireOutput(variant1, activator, entity);
        DevMsg("output: %s\n", fmtstr.c_str());
    }

    DETOUR_DECL_MEMBER(void, CTFPlayer_InputIgnitePlayer, inputdata_t &inputdata)
    {
        reinterpret_cast<CTFPlayer *>(this)->m_Shared->Burn(inputdata.pActivator->IsPlayer() ? ToTFPlayer(inputdata.pActivator) : reinterpret_cast<CTFPlayer *>(this), nullptr, inputdata.value.Float());
    }

    const char *logic_case_classname;
    const char *tf_gamerules_classname;
    const char *player_classname;
	DETOUR_DECL_MEMBER(bool, CBaseEntity_AcceptInput, const char *szInputName, CBaseEntity *pActivator, CBaseEntity *pCaller, variant_t Value, int outputID)
    {
        CBaseEntity *ent = reinterpret_cast<CBaseEntity *>(this);
        if (szInputName[0] == '$') {
            if (ent->GetClassname() == logic_case_classname) {
                CLogicCase *logic_case = static_cast<CLogicCase *>(ent);
                if (strnicmp(szInputName, "$FormatInput", strlen("$FormatInput")) == 0) {
                    int num = strtol(szInputName + strlen("$FormatInput"), nullptr, 10);
                    if (num > 0 && num < 16) {

                        ent->KeyValue(CFmtStr("Case%02d", num), Value.String());
                        FireFormatInput(logic_case, pActivator, pCaller);
                        
                        return true;
                    }
                }
                else if (strnicmp(szInputName, "$FormatInputNoFire", strlen("$FormatInputNoFire")) == 0) {
                    int num = strtol(szInputName + strlen("$FormatInputNoFire"), nullptr, 10);
                    if (num > 0 && num < 16) {
                        ent->KeyValue(CFmtStr("Case%02d", num), Value.String());
                        return true;
                    }
                }
                else if (FStrEq(szInputName, "$FormatString")) {
                    ent->KeyValue("Case16", Value.String());
                    FireFormatInput(logic_case, pActivator, pCaller);
                    return true;
                }
                else if (FStrEq(szInputName, "$FormatStringNoFire")) {
                    ent->KeyValue("Case16", Value.String());
                    return true;
                }
                else if (FStrEq(szInputName, "$Format")) {
                    FireFormatInput(logic_case, pActivator, pCaller);
                    return true;
                }
                else if (FStrEq(szInputName, "$TestSigsegv")) {
                    ent->m_OnUser1->FireOutput(Value, pActivator, pCaller);
                    return true;
                }
                else if (FStrEq(szInputName, "$ToInt")) {
                    variant_t convert;
                    convert.SetInt(strtol(Value.String(), nullptr, 10));
                    logic_case->m_OnDefault->FireOutput(convert, pActivator, ent);
                    return true;
                }
                else if (FStrEq(szInputName, "$ToFloat")) {
                    variant_t convert;
                    convert.SetFloat(strtof(Value.String(), nullptr));
                    logic_case->m_OnDefault->FireOutput(convert, pActivator, ent);
                    return true;
                }
                else if (FStrEq(szInputName, "$CallerToActivator")) {
                    logic_case->m_OnDefault->FireOutput(Value, pCaller, ent);
                    return true;
                }
                else if (FStrEq(szInputName, "$GetKeyValueFromActivator")) {
                    variant_t variant;
                    pActivator->ReadKeyField(Value.String(), &variant);
                    logic_case->m_OnDefault->FireOutput(variant, pActivator, ent);
                    return true;
                }
                else if (FStrEq(szInputName, "$GetConVar")) {
                    ConVarRef cvref(Value.String());
                    if (cvref.IsValid() && cvref.IsFlagSet(FCVAR_REPLICATED) && !cvref.IsFlagSet(FCVAR_PROTECTED)) {
                        variant_t variant;
                        variant.SetFloat(cvref.GetFloat());
                        logic_case->m_OnDefault->FireOutput(variant, pActivator, ent);
                    }
                    return true;
                }
                else if (FStrEq(szInputName, "$GetConVarString")) {
                    ConVarRef cvref(Value.String());
                    if (cvref.IsValid() && cvref.IsFlagSet(FCVAR_REPLICATED) && !cvref.IsFlagSet(FCVAR_PROTECTED)) {
                        variant_t variant;
                        variant.SetString(AllocPooledString(cvref.GetString()));
                        logic_case->m_OnDefault->FireOutput(variant, pActivator, ent);
                    }
                    return true;
                }
            }
            else if (ent->GetClassname() == tf_gamerules_classname) {
                if (stricmp(szInputName, "$StopVO") == 0) {
                    TFGameRules()->BroadcastSound(SOUND_FROM_LOCAL_PLAYER, Value.String(), SND_STOP);
                    return true;
                }
                else if (stricmp(szInputName, "$StopVORed") == 0) {
                    TFGameRules()->BroadcastSound(TF_TEAM_RED, Value.String(), SND_STOP);
                    return true;
                }
                else if (stricmp(szInputName, "$StopVOBlue") == 0) {
                    TFGameRules()->BroadcastSound(TF_TEAM_BLUE, Value.String(), SND_STOP);
                    return true;
                }
                else if (stricmp(szInputName, "$SetBossHealthPercentage") == 0) {
                    float val = atof(Value.String());
                    if (g_pMonsterResource.GetRef() != nullptr)
                        g_pMonsterResource->m_iBossHealthPercentageByte = (int) (val * 255.0f);
                    return true;
                }
                else if (stricmp(szInputName, "$SetBossState") == 0) {
                    int val = atoi(Value.String());
                    if (g_pMonsterResource.GetRef() != nullptr)
                        g_pMonsterResource->m_iBossState = val;
                    return true;
                }
            }
            else if (ent->GetClassname() == player_classname) {
                if (stricmp(szInputName, "$AllowClassAnimations") == 0) {
                    CTFPlayer *player = ToTFPlayer(ent);
                    if (player != nullptr) {
                        player->GetPlayerClass()->m_bUseClassAnimations = Value.Bool();
                    }
                    return true;
                }
                else if (stricmp(szInputName, "$SwitchClass") == 0) {
                    CTFPlayer *player = ToTFPlayer(ent);
                    
                    if (player != nullptr) {

                        int index = strtol(Value.String(), nullptr, 10);
                        if (index > 0 && index < 10) {
                            player->HandleCommand_JoinClass(g_aRawPlayerClassNames[index]);
                        }
                        else {
                            player->HandleCommand_JoinClass(Value.String());
                        }
                    }
                    return true;
                }
                else if (stricmp(szInputName, "$ForceRespawn") == 0) {
                    CTFPlayer *player = ToTFPlayer(ent);
                    if (player != nullptr) {
                        if (player->GetTeamNumber() >= TF_TEAM_RED && player->GetPlayerClass() != nullptr && player->GetPlayerClass()->GetClassIndex() != TF_CLASS_UNDEFINED) {
                            player->ForceRespawn();
                        }
                        else {
                            player->m_bAllowInstantSpawn = true;
                        }
                    }
                    return true;
                }
                else if (stricmp(szInputName, "$ForceRespawnDead") == 0) {
                    CTFPlayer *player = ToTFPlayer(ent);
                    if (player != nullptr && !player->IsAlive()) {
                        if (player->GetTeamNumber() >= TF_TEAM_RED && player->GetPlayerClass() != nullptr && player->GetPlayerClass()->GetClassIndex() != TF_CLASS_UNDEFINED) {
                            player->ForceRespawn();
                        }
                        else {
                            player->m_bAllowInstantSpawn = true;
                        }
                    }
                    return true;
                }
                else if (stricmp(szInputName, "$DisplayTextCenter") == 0) {
                    gamehelpers->TextMsg(ENTINDEX(ent), TEXTMSG_DEST_CENTER, Value.String());
                    return true;
                }
                else if (stricmp(szInputName, "$DisplayTextChat") == 0) {
                    gamehelpers->TextMsg(ENTINDEX(ent), TEXTMSG_DEST_CHAT , Value.String());
                    return true;
                }
                else if (stricmp(szInputName, "$Suicide") == 0) {
                    CTFPlayer *player = ToTFPlayer(ent);
                    if (player != nullptr) {
                        player->CommitSuicide(false, true);
                    }
                    return true;
                }
                else if (stricmp(szInputName, "$ChangeAttributes") == 0) {
                    CTFBot *bot = ToTFBot(ent);
                    if (bot != nullptr) {
                        auto *attrib = bot->GetEventChangeAttributes(Value.String());
                        if (attrib != nullptr){
                            bot->OnEventChangeAttributes(attrib);
                        }
                    }
                    return true;
                }
                else if (stricmp(szInputName, "$RollCommonSpell") == 0) {
                    CTFPlayer *player = ToTFPlayer(ent);
                    CBaseEntity *weapon = player->GetEntityForLoadoutSlot(LOADOUT_POSITION_ACTION);
                    
                    if (weapon == nullptr || !FStrEq(weapon->GetClassname(), "tf_weapon_spellbook")) return true;

                    CTFSpellBook *spellbook = rtti_cast<CTFSpellBook *>(weapon);
                    spellbook->RollNewSpell(0);

                    return true;
                }
                else if (stricmp(szInputName, "$SetSpell") == 0) {
                    CTFPlayer *player = ToTFPlayer(ent);
                    CBaseEntity *weapon = player->GetEntityForLoadoutSlot(LOADOUT_POSITION_ACTION);
                    
                    if (weapon == nullptr || !FStrEq(weapon->GetClassname(), "tf_weapon_spellbook")) return true;
                    
                    const char *str = Value.String();
                    int index = strtol(str, nullptr, 10);
                    for (int i = 0; i < ARRAYSIZE(SPELL_TYPE); i++) {
                        if (FStrEq(str, SPELL_TYPE[i])) {
                            index = i;
                        }
                    }

                    CTFSpellBook *spellbook = rtti_cast<CTFSpellBook *>(weapon);
                    spellbook->m_iSelectedSpellIndex = index;
                    spellbook->m_iSpellCharges = 1;

                    return true;
                }
                else if (stricmp(szInputName, "$AddSpell") == 0) {
                    CTFPlayer *player = ToTFPlayer(ent);
                    
                    CBaseEntity *weapon = player->GetEntityForLoadoutSlot(LOADOUT_POSITION_ACTION);
                    
                    if (weapon == nullptr || !FStrEq(weapon->GetClassname(), "tf_weapon_spellbook")) return true;
                    
                    const char *str = Value.String();
                    int index = strtol(str, nullptr, 10);
                    for (int i = 0; i < ARRAYSIZE(SPELL_TYPE); i++) {
                        if (FStrEq(str, SPELL_TYPE[i])) {
                            index = i;
                        }
                    }

                    CTFSpellBook *spellbook = rtti_cast<CTFSpellBook *>(weapon);
                    if (spellbook->m_iSelectedSpellIndex != index) {
                        spellbook->m_iSpellCharges = 0;
                    }
                    spellbook->m_iSelectedSpellIndex = index;
                    spellbook->m_iSpellCharges += 1;

                    return true;
                }
                else if (stricmp(szInputName, "$AddCond") == 0) {
                    CTFPlayer *player = ToTFPlayer(ent);
                    int index = 0;
                    float duration = -1.0f;
                    sscanf(Value.String(), "%d %f", &index, &duration);
                    if (player != nullptr) {
                        player->m_Shared->AddCond(index, duration);
                    }
                    return true;
                }
                else if (stricmp(szInputName, "$RemoveCond") == 0) {
                    CTFPlayer *player = ToTFPlayer(ent);
                    int index = strtol(Value.String(), nullptr, 10);
                    if (player != nullptr) {
                        player->m_Shared->RemoveCond(index);
                    }
                    return true;
                }
                else if (stricmp(szInputName, "$AddPlayerAttribute") == 0) {
                    CTFPlayer *player = ToTFPlayer(ent);
                    char param_tokenized[256];
                    V_strncpy(param_tokenized, Value.String(), sizeof(param_tokenized));
                    
                    char *attr = strtok(param_tokenized,"|");
                    char *value = strtok(NULL,"|");

                    if (player != nullptr) {
                        player->AddCustomAttribute(attr, atof(value), -1.0f);
                    }
                    return true;
                }
                else if (stricmp(szInputName, "$RemovePlayerAttribute") == 0) {
                    CTFPlayer *player = ToTFPlayer(ent);
                    if (player != nullptr) {
                        player->RemoveCustomAttribute(Value.String());
                    }
                    return true;
                }
                else if (stricmp(szInputName, "$AddItemAttribute") == 0) {
                    CTFPlayer *player = ToTFPlayer(ent);
                    char param_tokenized[256];
                    V_strncpy(param_tokenized, Value.String(), sizeof(param_tokenized));
                    
                    char *attr = strtok(param_tokenized,"|");
                    char *value = strtok(NULL,"|");
                    char *slot = strtok(NULL,"|");

                    if (player != nullptr) {
                        CEconEntity *item = nullptr;
                        if (slot != nullptr) {
                            item = GetEconEntityAtLoadoutSlot(player, atoi(slot));
                        }
                        else {
                            item = player->GetActiveTFWeapon();
                        }
                        if (item != nullptr) {
                            CEconItemAttributeDefinition *attr_def = GetItemSchema()->GetAttributeDefinitionByName(attr);
                            if (attr_def == nullptr) {
                                int idx = -1;
                                if (StringToIntStrict(attr, idx)) {
                                    attr_def = GetItemSchema()->GetAttributeDefinition(idx);
                                }
                            }
				            item->GetItem()->GetAttributeList().AddStringAttribute(attr_def, value);

                        }
                    }
                    return true;
                }
                else if (stricmp(szInputName, "$RemoveItemAttribute") == 0) {
                    CTFPlayer *player = ToTFPlayer(ent);
                    char param_tokenized[256];
                    V_strncpy(param_tokenized, Value.String(), sizeof(param_tokenized));
                    
                    char *attr = strtok(param_tokenized,"|");
                    char *slot = strtok(NULL,"|");

                    if (player != nullptr) {
                        CEconEntity *item = nullptr;
                        if (slot != nullptr) {
                            item = GetEconEntityAtLoadoutSlot(player, atoi(slot));
                        }
                        else {
                            item = player->GetActiveTFWeapon();
                        }
                        if (item != nullptr) {
                            CEconItemAttributeDefinition *attr_def = GetItemSchema()->GetAttributeDefinitionByName(attr);
                            if (attr_def == nullptr) {
                                int idx = -1;
                                if (StringToIntStrict(attr, idx)) {
                                    attr_def = GetItemSchema()->GetAttributeDefinition(idx);
                                }
                            }
				            item->GetItem()->GetAttributeList().RemoveAttribute(attr_def);

                        }
                    }
                    return true;
                }
                else if (stricmp(szInputName, "$PlaySoundToSelf") == 0) {
                    CTFPlayer *player = ToTFPlayer(ent);
                    CRecipientFilter filter;
		            filter.AddRecipient(player);
                    
                    if (!enginesound->PrecacheSound(Value.String(), true))
						CBaseEntity::PrecacheScriptSound(Value.String());

                    EmitSound_t params;
                    params.m_pSoundName = Value.String();
                    params.m_flSoundTime = 0.0f;
                    params.m_pflSoundDuration = nullptr;
                    params.m_bWarnOnDirectWaveReference = true;
                    CBaseEntity::EmitSound(filter, ENTINDEX(player), params);
                return true;
                }
            }

            if (stricmp(szInputName, "$FireUserAsActivator1") == 0) {
                ent->m_OnUser1->FireOutput(Value, ent, ent);
                return true;
            }
            else if (stricmp(szInputName, "$FireUserAsActivator2") == 0) {
                ent->m_OnUser2->FireOutput(Value, ent, ent);
                return true;
            }
            else if (stricmp(szInputName, "$FireUserAsActivator3") == 0) {
                ent->m_OnUser3->FireOutput(Value, ent, ent);
                return true;
            }
            else if (stricmp(szInputName, "$FireUserAsActivator4") == 0) {
                ent->m_OnUser4->FireOutput(Value, ent, ent);
                return true;
            }
            else if (stricmp(szInputName, "$TakeDamage") == 0) {
                CTakeDamageInfo info(UTIL_EntityByIndex(0), UTIL_EntityByIndex(0), nullptr, vec3_origin, ent->GetAbsOrigin(), strtol(Value.String(), nullptr, 10), DMG_PREVENT_PHYSICS_FORCE, 0 );
                ent->TakeDamage(info);
                return true;
            }
            else if (stricmp(szInputName, "$SetModelOverride") == 0) {
                int replace_model = CBaseEntity::PrecacheModel(Value.String());
                if (replace_model != -1) {
                    for (int i = 0; i < MAX_VISION_MODES; ++i) {
                        ent->SetModelIndexOverride(i, replace_model);
                    }
                }
                return true;
            }
            else if (stricmp(szInputName, "$SetModel") == 0) {
                CBaseEntity::PrecacheModel(Value.String());
                ent->SetModel(Value.String());
                return true;
            }
            else if (stricmp(szInputName, "$SetModelSpecial") == 0) {
                int replace_model = CBaseEntity::PrecacheModel(Value.String());
                if (replace_model != -1) {
                    ent->SetModelIndex(replace_model);
                }
                return true;
            }
            else if (stricmp(szInputName, "$SetOwner") == 0) {
                auto owner = servertools->FindEntityByName(nullptr, Value.String(), ent, pActivator, pCaller);
                if (owner != nullptr) {
                    ent->SetOwnerEntity(owner);
                }
                return true;
            }
            else if (stricmp(szInputName, "$GetKeyValue") == 0) {
                variant_t variant;
                ent->ReadKeyField(Value.String(), &variant);
                ent->m_OnUser1->FireOutput(variant, pActivator, ent);
                return true;
            }
            else if (stricmp(szInputName, "$InheritOwner") == 0) {
                auto owner = servertools->FindEntityByName(nullptr, Value.String(), ent, pActivator, pCaller);
                if (owner != nullptr) {
                    ent->SetOwnerEntity(owner->GetOwnerEntity());
                }
                return true;
            }
            else if (stricmp(szInputName, "$InheritParent") == 0) {
                auto owner = servertools->FindEntityByName(nullptr, Value.String(), ent, pActivator, pCaller);
                if (owner != nullptr) {
                    ent->SetParent(owner->GetMoveParent(), -1);
                }
                return true;
            }
            else if (stricmp(szInputName, "$MoveType") == 0) {
                variant_t variant;
                int val1=0;
                int val2=MOVECOLLIDE_DEFAULT;

                sscanf(Value.String(), "%d,%d", &val1, &val2);
                ent->SetMoveType(val1, val2);
                return true;
            }
            else if (stricmp(szInputName, "$PlaySound") == 0) {
                
                if (!enginesound->PrecacheSound(Value.String(), true))
                    CBaseEntity::PrecacheScriptSound(Value.String());

                ent->EmitSound(Value.String());
                return true;
            }
            else if (stricmp(szInputName, "$StopSound") == 0) {
                ent->StopSound(Value.String());
                return true;
            }
            else if (stricmp(szInputName, "$SetLocalOrigin") == 0) {
                Vector vec;
                sscanf(Value.String(), "%f %f %f", &vec.x, &vec.y, &vec.z);
                ent->SetLocalOrigin(vec);
                return true;
            }
            else if (stricmp(szInputName, "$SetLocalAngles") == 0) {
                QAngle vec;
                sscanf(Value.String(), "%f %f %f", &vec.x, &vec.y, &vec.z);
                ent->SetLocalAngles(vec);
                return true;
            }
            else if (stricmp(szInputName, "$SetLocalVelocity") == 0) {
                Vector vec;
                sscanf(Value.String(), "%f %f %f", &vec.x, &vec.y, &vec.z);
                ent->SetLocalVelocity(vec);
                return true;
            }
            else if (stricmp(szInputName, "$TeleportToEntity") == 0) {
                auto target = servertools->FindEntityByName(nullptr, Value.String(), ent, pActivator, pCaller);
                Vector targetpos = target->GetAbsOrigin();
                ent->Teleport(&targetpos, nullptr, nullptr);
                return true;
            }
            else if (stricmp(szInputName, "$MoveRelative") == 0) {
                Vector vec;
                sscanf(Value.String(), "%f %f %f", &vec.x, &vec.y, &vec.z);
                vec += ent->GetLocalOrigin();
                ent->SetLocalOrigin(vec);
                return true;
            }
            else if (stricmp(szInputName, "$RotateRelative") == 0) {
                QAngle vec;
                sscanf(Value.String(), "%f %f %f", &vec.x, &vec.y, &vec.z);
                vec += ent->GetLocalAngles();
                ent->SetLocalAngles(vec);
                return true;
            }
            else if (stricmp(szInputName, "$TestEntity") == 0) {
                auto target = servertools->FindEntityByName(nullptr, Value.String(), ent, pActivator, pCaller);

                if (target != nullptr) {
                    auto filter = rtti_cast<CBaseFilter *>(ent);
                    if (filter != nullptr && filter->PassesFilter(pCaller, target)) {
                        filter->m_OnPass->FireOutput(Value, pActivator, target);
                    }
                }
                return true;
            }
            else if (stricmp(szInputName, "$StartTouchEntity") == 0) {
                auto target = servertools->FindEntityByName(nullptr, Value.String(), ent, pActivator, pCaller);

                if (target != nullptr) {
                    auto filter = rtti_cast<CBaseTrigger *>(ent);
                    if (filter != nullptr) {
                        filter->StartTouch(target);
                    }
                }
                return true;
            }
            else if (stricmp(szInputName, "$EndTouchEntity") == 0) {
                auto target = servertools->FindEntityByName(nullptr, Value.String(), ent, pActivator, pCaller);

                if (target != nullptr) {
                    auto filter = rtti_cast<CBaseTrigger *>(ent);
                    if (filter != nullptr) {
                        filter->EndTouch(target);
                    }
                }
                return true;
            }
        }
        return DETOUR_MEMBER_CALL(CBaseEntity_AcceptInput)(szInputName, pActivator, pCaller, Value, outputID);
    }

    void ActivateLoadedInput()
    {
        DevMsg("ActivateLoadedInput\n");
        auto entity = servertools->FindEntityByName(nullptr, "sigsegv_load");
        
        if (entity != nullptr) {
            variant_t variant1;
            variant1.SetString(NULL_STRING);

            entity->AcceptInput("FireUser1", UTIL_EntityByIndex(0), UTIL_EntityByIndex(0) ,variant1,-1);
        }
    }

    DETOUR_DECL_MEMBER(void, CTFGameRules_CleanUpMap)
	{
		DETOUR_MEMBER_CALL(CTFGameRules_CleanUpMap)();
        ActivateLoadedInput();
	}
    int getspawnflags(CBaseEntity *ent) 
    {
        return ent->m_spawnflags;
    }
    DETOUR_DECL_MEMBER(void, CTFMedigunShield_RemoveShield)
	{
        CTFMedigunShield *shield = reinterpret_cast<CTFMedigunShield *>(this);
        int spawnflags = shield->m_spawnflags;
        //DevMsg("ShieldRemove %d f\n", spawnflags);
        
        if (spawnflags == 3) {
            DevMsg("Spawnflags is 3\n");
            shield->SetModel("models/props_mvm/mvm_player_shield2.mdl");
        }

        if (spawnflags == 0) {
            //DevMsg("Spawnflags is 0\n");
        }
        else{
            //DevMsg("Spawnflags is not 0\n");
            shield->SetBlocksLOS(false);
            return;
        }

        
		DETOUR_MEMBER_CALL(CTFMedigunShield_RemoveShield)();
	}

    DETOUR_DECL_MEMBER(void, CTFMedigunShield_UpdateShieldPosition)
	{   
		DETOUR_MEMBER_CALL(CTFMedigunShield_UpdateShieldPosition)();
	}

    DETOUR_DECL_MEMBER(void, CTFMedigunShield_ShieldThink)
	{
        
		DETOUR_MEMBER_CALL(CTFMedigunShield_ShieldThink)();
	}
    
    RefCount rc_CTriggerHurt_HurtEntity;
    DETOUR_DECL_MEMBER(bool, CTriggerHurt_HurtEntity, CBaseEntity *other, float damage)
	{
        SCOPED_INCREMENT(rc_CTriggerHurt_HurtEntity);
		return DETOUR_MEMBER_CALL(CTriggerHurt_HurtEntity)(other, damage);
	}

    DETOUR_DECL_MEMBER(int, CBaseEntity_TakeDamage, CTakeDamageInfo &info)
	{
		//DevMsg("Take damage damage %f\n", info.GetDamage());
        if (rc_CTriggerHurt_HurtEntity) {
            auto owner = info.GetAttacker()->GetOwnerEntity();
            if (owner != nullptr && owner->IsPlayer()) {
                info.SetAttacker(owner);
                info.SetInflictor(owner);
            }
        }
		return DETOUR_MEMBER_CALL(CBaseEntity_TakeDamage)(info);
	}

    class CMod : public IMod, IModCallbackListener
	{
	public:
		CMod() : IMod("Etc:Mapentity_Additions")
		{
			MOD_ADD_DETOUR_MEMBER(CTFPlayer_InputIgnitePlayer, "CTFPlayer::InputIgnitePlayer");
			MOD_ADD_DETOUR_MEMBER(CBaseEntity_AcceptInput, "CBaseEntity::AcceptInput");
			MOD_ADD_DETOUR_MEMBER(CTFGameRules_CleanUpMap, "CTFGameRules::CleanUpMap");
			MOD_ADD_DETOUR_MEMBER(CTFMedigunShield_RemoveShield, "CTFMedigunShield::RemoveShield");
			MOD_ADD_DETOUR_MEMBER(CTriggerHurt_HurtEntity, "CTriggerHurt::HurtEntity");
            MOD_ADD_DETOUR_MEMBER_PRIORITY(CBaseEntity_TakeDamage, "CBaseEntity::TakeDamage", HIGHEST);
    
		//	MOD_ADD_DETOUR_MEMBER(CTFMedigunShield_UpdateShieldPosition, "CTFMedigunShield::UpdateShieldPosition");
		//	MOD_ADD_DETOUR_MEMBER(CTFMedigunShield_ShieldThink, "CTFMedigunShield::ShieldThink");
		//	MOD_ADD_DETOUR_MEMBER(CBaseGrenade_SetDamage, "CBaseGrenade::SetDamage");
		}

        virtual bool OnLoad() override
		{
            ActivateLoadedInput();
            logic_case_classname = STRING(AllocPooledString("logic_case"));
            tf_gamerules_classname = STRING(AllocPooledString("tf_gamerules"));
            player_classname = STRING(AllocPooledString("player"));
			return true;
		}
        virtual bool ShouldReceiveCallbacks() const override { return this->IsEnabled(); }

        virtual void LevelInitPreEntity() override
        {
            logic_case_classname = STRING(AllocPooledString("logic_case"));
            tf_gamerules_classname = STRING(AllocPooledString("tf_gamerules"));
            player_classname = STRING(AllocPooledString("player"));
        }
	};
	CMod s_Mod;
	
	
	ConVar cvar_enable("sig_etc_mapentity_additions", "0", FCVAR_NOTIFY,
		"Mod: tell maps that sigsegv extension is loaded",
		[](IConVar *pConVar, const char *pOldValue, float flOldValue){
			s_Mod.Toggle(static_cast<ConVar *>(pConVar)->GetBool());
		});
}