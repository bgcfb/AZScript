#include "ScriptPCH.h"

#include "Player.h"

#include "Chat.h"

#include "ObjectMgr.h"

#include "ScriptMgr.h"

#include "Item.h"

#include "Config.h"

#include "Pet.h"

#pragma execution_character_set("utf-8")



class Gossip_OnNpcZy : public CreatureScript

{

public:

	Gossip_OnNpcZy() :CreatureScript("Gossip_OnNpcZy") {}



	//תְ����

	bool SetClassToNew(Player* player, uint8 newclass)

	{

		//���

		if (!player || !player->IsInWorld() || !player->IsAlive() || player->IsInCombat() || player->getClass() == newclass || newclass <= 0 || newclass >= MAX_CLASSES)

			return false;



		//�Ƴ����� ����صĶ���

		if (Pet* pet = player->GetPet())

		{

			uint32 PGuid = pet->GetCharmInfo()->GetPetNumber();

			player->RemovePet(pet, PET_SAVE_NOT_IN_SLOT);

			SQLTransaction trans = CharacterDatabase.BeginTransaction();

			PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_CHAR_PET_DECLINEDNAME);

			stmt->setUInt32(0, PGuid);

			trans->Append(stmt);

			stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_PET_AURAS);

			stmt->setUInt32(0, PGuid);

			trans->Append(stmt);

			stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_PET_SPELLS);

			stmt->setUInt32(0, PGuid);

			trans->Append(stmt);

			stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_PET_SPELL_COOLDOWNS);

			stmt->setUInt32(0, PGuid);

			trans->Append(stmt);

			stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_CHAR_PET_BY_OWNER);

			stmt->setUInt32(0, player->GetGUIDLow());

			trans->Append(stmt);

			CharacterDatabase.CommitTransaction(trans);

		}

		//�����츳
		player->resetTalents();

		//�Ƴ�����

		for (uint32 slot = 0; slot < MAX_GLYPH_SLOT_INDEX; ++slot)

		{

			if (slot >= MAX_GLYPH_SLOT_INDEX)

				continue;



			if (uint32 glyph = player->GetGlyph(slot))

			{

				if (GlyphPropertiesEntry const* gp = sGlyphPropertiesStore.LookupEntry(glyph))

				{

					player->RemoveAurasDueToSpell(gp->SpellId);

					player->SetGlyph(slot, 0, true);

					player->SendTalentsInfoData(false);

				}

			}

		}

		/*
		*����ӣ��⻷�ͼ�����ȴ�����
		*/
		uint32 guid = GUID_LOPART(player->GetGUID());
		SQLTransaction trans = CharacterDatabase.BeginTransaction();
		PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_CHAR_AURA);
		stmt->setUInt32(0, guid);
		trans->Append(stmt);
		stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_CHAR_SPELL_COOLDOWN);
		stmt->setUInt32(0, guid);
		trans->Append(stmt);
		/*
			����Ϊ���ݿ�ɾ����ر�����ݣ����Բ��ӣ��������������
		*/
		/*stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_CHAR_SPELL);
		stmt->setUInt32(0, guid);
		trans->Append(stmt);*/
		/*stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_CHAR_TALENT);
		stmt->setUInt32(0, guid);
		trans->Append(stmt);*/
		/*stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_CHAR_SKILLS);
		stmt->setUInt32(0, guid);
		trans->Append(stmt);
		CharacterDatabase.CommitTransaction(trans);*/

		/*
		* ��������
		*/
		ChrClassesEntry const* classEntry = sChrClassesStore.LookupEntry(player->getClass());
		if (!classEntry)
			return true;
		uint32 family = classEntry->spellfamily;

		for (uint32 i = 0; i < sSkillLineAbilityStore.GetNumRows(); ++i)
		{
			SkillLineAbilityEntry const* entry = sSkillLineAbilityStore.LookupEntry(i);
			if (!entry)
				continue;

			SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(entry->spellId);
			if (!spellInfo)
				continue;

			// skip server-side/triggered spells
			if (spellInfo->SpellLevel == 0)
				continue;

			// skip wrong class/race skills
			if (!player->IsSpellFitByClassAndRace(spellInfo->Id))
				continue;

			// skip other spell families
			if (spellInfo->SpellFamilyName != family)
				continue;

			// skip spells with first rank learned as talent (and all talents then also)
			uint32 firstRank = sSpellMgr->GetFirstSpellInChain(spellInfo->Id);
			if (GetTalentSpellCost(firstRank) > 0)
				continue;

			// skip broken spells
			if (!SpellMgr::IsSpellValid(spellInfo))
				continue;
			player->removeSpell(spellInfo->Id, SPEC_MASK_ALL, false);
		}
		//---------------------------------սʿ---��ʿ---����---����---��ʦ----DK----����----��ʦ---��ʿ--------С��--------------------//
		//�˴�Ϊԭ���Ƴ����ܣ��������ã�������©�ĺܶ༼��
		//int const DBclass[MAX_CLASSES] = { 80001, 80002, 80003, 80004, 80005, 80006, 80007, 80008, 80009, NULL, 80010 };
		//int const DBclass[MAX_CLASSES] = { 913, 71007, 987, 917, 376, 28472, 986, 331, 461, NULL, 3033 };


		//������ְҵ���� �����Ǽ���ѵ��ʦID �������в�ȫ ��֤תְ����Ӱ������������ȡ�ļ���
		/*TrainerSpellData const* Spells = sObjectMgr->GetNpcTrainerSpells(DBclass[ player->getClass() - 1]);

		if (Spells)

		{

			for (TrainerSpellMap::const_iterator its = Spells->spellList.begin(); its != Spells->spellList.end(); ++its)

			{

				���շ�ֹ����

				if (!player->IsSpellFitByClassAndRace(its->second.spell))

					continue;

				SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(its->second.spell);

				if (!spellInfo)

					continue;

				if (!SpellMgr::IsSpellValid(spellInfo))

					continue;

				player->removeSpell(its->second.spell, SPEC_MASK_ALL, false);
			}

		}


*/
		//��������ID-----------����/���ֽ�/˫�ֽ�/��/ذ��/Ͷ��-----//

		int const Wpells[6] = { 200, 201, 202, 264, 1180, 2567 };



		//���� ���ǽ�ɫ����Ĭ�ϵ���������

		for (int w = 0; w < 6; ++w)

			player->removeSpell(Wpells[w], SPEC_MASK_ALL, false);



		//��������Ĭ�ϵļ��� ���������س�

		PlayerInfo const* info = sObjectMgr->GetPlayerInfo(player->getRace(true), player->getClass());

		for (PlayerCreateInfoSpells::const_iterator itc = info->spell.begin(); itc != info->spell.end(); ++itc)

			player->removeSpell(*itc, SPEC_MASK_ALL, false);



		player->InitTalentForLevel();
		player->SendTalentsInfoData(false);



		//ִ��תְ

		player->SetByteValue(UNIT_FIELD_BYTES_0, 1, newclass);



		//���½�ɫ��ְҵ��Ϣ

		sWorld->UpdateGlobalPlayerData(player->GetGUIDLow(), PLAYER_UPDATE_DATA_CLASS, player->GetName(), player->getLevel(), player->getGender(), player->getRace(), newclass);



		//ǿ�ư����ϲ�������ְҵ������װ��ж�� ��λ�þͷ� ûλ���Զ�����������

		for (uint8 i = EQUIPMENT_SLOT_START; i < EQUIPMENT_SLOT_END; ++i)

		{

			if (Item* pItem = player->GetItemByPos(INVENTORY_SLOT_BAG_0, i))

			{

				//�жϲ�����Ҫ���

				uint16 Cdest = 0;

				ItemTemplate const* pProto = pItem->GetTemplate();

				InventoryResult Check = player->CanEquipItem(NULL_SLOT, Cdest, pItem, false);

				if ((pProto->AllowableClass & player->getClassMask()) == 0 || (pProto->RequiredSpell != 0 && !player->HasSpell(pProto->RequiredSpell)) || Check != EQUIP_ERR_OK)

				{

					ItemPosCountVec dest;

					uint8 Msg = player->CanStoreItem(NULL_BAG, NULL_SLOT, dest, pItem, false);

					if (Msg == EQUIP_ERR_OK)

					{

						player->RemoveItem(INVENTORY_SLOT_BAG_0, pItem->GetSlot(), true);

						player->StoreItem(dest, pItem, true);

						//����̩̹֮��

						player->UpdateTitansGrip();

					}

					else

					{

						player->MoveItemFromInventory(INVENTORY_SLOT_BAG_0, pItem->GetSlot(), true);

						SQLTransaction trans = CharacterDatabase.BeginTransaction();

						pItem->DeleteFromInventoryDB(trans);

						pItem->SaveToDB(trans);

						MailDraft("תְ", "��Щװ����֧����ְҵ").AddItem(pItem).SendMailTo(trans, player, MailSender(player, MAIL_STATIONERY_GM), MAIL_CHECK_MASK_COPIED);

						CharacterDatabase.CommitTransaction(trans);

					}

				}

			}

		}



		//�����ɫ��Ϣ

		player->SaveToDB(false, false);

		//����ѧϰĬ�ϼ���

		player->learnDefaultSpells();

		//�������Ի�ȡ��Ӧְҵ��Power(������/ŭ����֮���)

		player->InitStatsForLevel(true);

		//�����ְҵ��DK�����������ȴ

		player->InitRunes();

		//���½�ɫ����

		player->SetCanModifyStats(true);

		player->UpdateAllStats();
		
		player->GetSession()->LogoutPlayer(true);
		//����ѧϰ��ְҵ���ܡ��������е�ѵ��ʦ������ѧϰ�����������ܰڷź��ҡ�����С��
	    //��Ϊ�ο���δʹ��
		/*

		TrainerSpellData const* NewSpells = sObjectMgr->GetNpcTrainerSpells(DBclass[zxsq-anti-bbcode-newclass - 1]);

		if (NewSpells)

		{

		for (TrainerSpellMap::const_iterator its = NewSpells->spellList.begin(); its != NewSpells->spellList.end(); ++its)

		{

		//���շ�ֹ����

		if (!player->IsSpellFitByClassAndRace(its->second.spell))

		continue;

		SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(its->second.spell);

		if (!spellInfo)

		continue;

		if (!SpellMgr::IsSpellValid(spellInfo))

		continue;

		player->_addSpell(*its, SPEC_MASK_ALL, true);

		}

		}

		*/



		ChatHandler(player->GetSession()).PSendSysMessage("��ϲ��תְ���");

		player->GetSession()->SendAreaTriggerMessage("��ϲ��תְ���");



		return true;

	}



	bool OnGossipHello(Player* player, Creature* creature) 

	{
		uint32 Race = player->getRace();

		ClearGossipMenuFor(player);

		AddGossipItemFor(player, 10, "|TInterface\\ICONS\\Trade_Engineering.blp:30:30:-20:-3|t��ӭʹ�� תְ ����", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF);



		if (Race != RACE_BLOODELF && player->getClass() != CLASS_WARRIOR)

			AddGossipItemFor(player, 10, "|TInterface\\ICONS\\Trade_Engineering.blp:30:30:-20:-3|tתְΪսʿ", CLASS_WARRIOR, GOSSIP_ACTION_INFO_DEF + 999,"���۳�|cffffff0010000����|r,תְ֮�󽫷��ص���ɫѡ����棡",0,false);



		if (Race != RACE_NIGHTELF && Race != RACE_GNOME && Race != RACE_ORC && Race != RACE_UNDEAD_PLAYER && Race != RACE_TAUREN && Race != RACE_TROLL && player->getClass() != CLASS_PALADIN)

			AddGossipItemFor(player, 10, "|TInterface\\ICONS\\Trade_Engineering.blp:30:30:-20:-3|tתְΪ��ʿ", CLASS_PALADIN, GOSSIP_ACTION_INFO_DEF + 999, "���۳�|cffffff0010000����|r,תְ֮�󽫷��ص���ɫѡ����棡", 0, false);



		if (Race != RACE_HUMAN && Race != RACE_GNOME && Race != RACE_UNDEAD_PLAYER && player->getClass() != CLASS_HUNTER)

			AddGossipItemFor(player, 10, "|TInterface\\ICONS\\Trade_Engineering.blp:30:30:-20:-3|tתְΪ����", CLASS_HUNTER, GOSSIP_ACTION_INFO_DEF + 999, "���۳�|cffffff0010000����|r,תְ֮�󽫷��ص���ɫѡ����棡", 0, false);



		if (Race != RACE_DRAENEI && Race != RACE_TAUREN && player->getClass() != CLASS_ROGUE)

			AddGossipItemFor(player, 10, "|TInterface\\ICONS\\Trade_Engineering.blp:30:30:-20:-3|tתְΪ����", CLASS_ROGUE, GOSSIP_ACTION_INFO_DEF + 999, "���۳�|cffffff0010000����|r,תְ֮�󽫷��ص���ɫѡ����棡", 0, false);



		if (Race != RACE_GNOME && Race != RACE_ORC && Race != RACE_TAUREN && player->getClass() != CLASS_PRIEST)

			AddGossipItemFor(player, 10, "|TInterface\\ICONS\\Trade_Engineering.blp:30:30:-20:-3|tתְΪ��ʦ", CLASS_PRIEST, GOSSIP_ACTION_INFO_DEF + 999, "���۳�|cffffff0010000����|r,תְ֮�󽫷��ص���ɫѡ����棡", 0, false);



		if (player->getClass() != CLASS_DEATH_KNIGHT)

			AddGossipItemFor(player, 10, "|TInterface\\ICONS\\Trade_Engineering.blp:30:30:-20:-3|tתְΪ����", CLASS_DEATH_KNIGHT, GOSSIP_ACTION_INFO_DEF + 999, "���۳�|cffffff0010000����|r,תְ֮�󽫷��ص���ɫѡ����棡", 0, false);



		if (Race == RACE_DRAENEI || Race == RACE_ORC || Race == RACE_TAUREN || Race == RACE_TROLL && player->getClass() != CLASS_SHAMAN)

			AddGossipItemFor(player, 10, "|TInterface\\ICONS\\Trade_Engineering.blp:30:30:-20:-3|tתְΪ����", CLASS_SHAMAN, GOSSIP_ACTION_INFO_DEF + 999, "���۳�|cffffff0010000����|r,תְ֮�󽫷��ص���ɫѡ����棡", 0, false);



		if (Race == RACE_HUMAN || Race == RACE_GNOME || Race == RACE_DRAENEI || Race == RACE_UNDEAD_PLAYER || Race == RACE_TROLL || Race == RACE_BLOODELF && player->getClass() != CLASS_MAGE)

			AddGossipItemFor(player, 10, "|TInterface\\ICONS\\Trade_Engineering.blp:30:30:-20:-3|tתְΪ��ʦ", CLASS_MAGE, GOSSIP_ACTION_INFO_DEF + 999, "���۳�|cffffff0010000����|r,תְ֮�󽫷��ص���ɫѡ����棡", 0, false);



		if (Race == RACE_HUMAN || Race == RACE_GNOME || Race == RACE_ORC || Race == RACE_UNDEAD_PLAYER || Race == RACE_BLOODELF && player->getClass() != CLASS_WARLOCK)

			AddGossipItemFor(player, 10, "|TInterface\\ICONS\\Trade_Engineering.blp:30:30:-20:-3|tתְΪ��ʿ", CLASS_WARLOCK, GOSSIP_ACTION_INFO_DEF + 999, "���۳�|cffffff0010000����|r,תְ֮�󽫷��ص���ɫѡ����棡", 0, false);



		if (Race == RACE_NIGHTELF || Race == RACE_TAUREN && player->getClass() != CLASS_DRUID)

			AddGossipItemFor(player, 10, "|TInterface\\ICONS\\Trade_Engineering.blp:30:30:-20:-3|tתְΪС��", CLASS_DRUID, GOSSIP_ACTION_INFO_DEF + 999, "���۳�|cffffff0010000����|r,תְ֮�󽫷��ص���ɫѡ����棡", 0, false);



		SendGossipMenuFor(player, DEFAULT_GOSSIP_MESSAGE, creature->GetGUID());



		return true;

	}



	bool OnGossipSelect(Player* player, Creature* creature, uint32 sender, uint32 action) 

	{

		ClearGossipMenuFor(player);



		if (sender == GOSSIP_SENDER_MAIN && action == GOSSIP_ACTION_INFO_DEF)

		{

			OnGossipHello(player, creature);

			return true;

		}



		if (action == GOSSIP_ACTION_INFO_DEF + 999 && sender)

		{

			if (sender <= 0 || sender >= MAX_CLASSES)

				return true;



			if (!SetClassToNew(player, (uint8)sender))
				player->ModifyUSERJF(-10000);
				return true;



			//�����ǿ۲���

			//----

		}



		CloseGossipMenuFor(player);



		return true;

	}

};

void AddGossipOnNpcZyScripts()
{
	new Gossip_OnNpcZy();
}