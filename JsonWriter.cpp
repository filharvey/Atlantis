#include "gamedata.h"
#include "game.h"

#if EXPORT_JSON
extern AString TownString(int i);

AString* GetCleanName(AString *v, int num) {
	// determine length of string representing number
	string numStr = to_string(num);
	int numLen = numStr.length();

	// make new buffer for string in the Heap
	// it will be less then numLen + 3 characters
	// because "Name [1234]"
	int bufferLen = v->Len() + 1 - numLen - 3;
	char *strPtr = new char[bufferLen];
	strncpy(strPtr, v->Str(), bufferLen - 1);
	strPtr[bufferLen - 1] = NULL;

	return new AString(strPtr);
}

void Faction::WriteReportJSON(AreportJSON *f, Game *pGame)
{
	if (IsNPC() && num == 1) {
		if (Globals->GM_REPORT || (pGame->month == 0 && pGame->year == 1)) {
			f->StartObject();
			int i, j;
			// Put all skills, items and objects in the GM report
			shows.DeleteAll();
			for (i = 0; i < NSKILLS; i++) {
				for (j = 1; j < 6; j++) {
					shows.Add(new ShowSkill(i, j));
				}
			}
			if (shows.Num()) {
				f->Key("Skill reports");
				f->StartArray();

				//				f->PutStr("Skill reports:");
				forlist(&shows) {
					AString *string = ((ShowSkill *)elem)->Report(this);
					if (string) {
						//						f->PutStr("");
						//						f->PutStr(*string);
						f->String(*string);
						delete string;
					}
				}
				shows.DeleteAll();
				f->EndArray();
				//				f->EndLine();
			}

			itemshows.DeleteAll();
			for (i = 0; i < NITEMS; i++) {
				AString *show = ItemDescription(i, 1);
				if (show) {
					itemshows.Add(show);
				}
			}
			if (itemshows.Num()) {
				f->Key("Item reports");
				f->StartArray();

				//				f->PutStr("Item reports:");
				forlist(&itemshows) {
					//					f->PutStr("");
					//					f->PutStr(*((AString *)elem));
					f->String(*((AString *)elem));
				}
				itemshows.DeleteAll();
				//				f->EndLine();
				f->EndArray();
			}

			objectshows.DeleteAll();
			for (i = 0; i < NOBJECTS; i++) {
				AString *show = ObjectDescription(i);
				if (show) {
					objectshows.Add(show);
				}
			}
			if (objectshows.Num()) {
				f->Key("Object reports");
				f->StartArray();
				//				f->PutStr("Object reports:");
				forlist(&objectshows) {
					//					f->PutStr("");
					//					f->PutStr(*((AString *)elem));
					f->String(*((AString *)elem));
				}
				objectshows.DeleteAll();
				//				f->EndLine();
				f->EndArray();
			}

			present_regions.DeleteAll();
			forlist(&(pGame->regions)) {
				ARegion *reg = (ARegion *)elem;
				ARegionPtr *ptr = new ARegionPtr;
				ptr->ptr = reg;
				present_regions.Add(ptr);
			}
			{
				f->Key("regions");
				f->StartArray();
				forlist(&present_regions) {
					((ARegionPtr*)elem)->ptr->WriteReportJSON(f, this,
						pGame->month,
						&(pGame->regions));
				}
				f->EndArray();
			}
			present_regions.DeleteAll();
			f->EndObject();
		}
		// do not delete so they are exported via normal turn report
		//		errors.DeleteAll();
		//		events.DeleteAll();
		//		battles.DeleteAll();
		return;
	}
	f->StartObject();

	//	f->PutStr("Atlantis Report For:");
	f->Key("faction");
	if ((Globals->FACTION_LIMIT_TYPE == GameDefs::FACLIM_MAGE_COUNT) ||
		(Globals->FACTION_LIMIT_TYPE == GameDefs::FACLIM_UNLIMITED)) {
		//		f->PutStr(*name);
		f->String(*name);
	}
	else if (Globals->FACTION_LIMIT_TYPE == GameDefs::FACLIM_FACTION_TYPES) {
		//		f->PutStr(*name + " (" + FactionTypeStr() + ")");
		f->String(*name);
		f->Key("factionType");
		//		f->String(FactionTypeStr());

		FactionTypeStrJSON(f);
	}
	//	f->PutStr(AString(MonthNames[pGame->month]) + ", Year " + pGame->year);
	f->Key("month");
	f->Int(pGame->month);
	f->Key("year");
	f->Int(pGame->year);
	//	f->EndLine();

	f->Key("engineVersion:");
	f->String(ATL_VER_STRING(CURRENT_ATL_VER));
	//	f->PutStr(AString("Atlantis Engine Version: ") +
	//		ATL_VER_STRING(CURRENT_ATL_VER));
	f->Key("ruleSet:");
	f->String(AString(Globals->RULESET_NAME));
	f->Key("ruleSetVersion:");
	f->String(ATL_VER_STRING(Globals->RULESET_VERSION));
	//	f->PutStr(AString(Globals->RULESET_NAME) + ", Version: " +
	//		ATL_VER_STRING(Globals->RULESET_VERSION));
	//	f->EndLine();

	if (!times) {
		f->Key("timesSent");
		f->Bool(false);
		//		f->PutStr("Note: The Times is not being sent to you.");
		//		f->EndLine();
	}

	if (!password || (*password == "none")) {
		f->Key("password");
		f->Bool(false);

		//		f->PutStr("REMINDER: You have not set a password for your faction!");
		//		f->EndLine();
	}

	if (Globals->MAX_INACTIVE_TURNS != -1) {
		int cturn = pGame->TurnNumber() - lastorders;
		if ((cturn >= (Globals->MAX_INACTIVE_TURNS - 3)) && !IsNPC()) {
			cturn = Globals->MAX_INACTIVE_TURNS - cturn;
			f->Key("turnWarning");
			f->Int(cturn);

			//			f->PutStr(AString("WARNING: You have ") + cturn +
			//				AString(" turns until your faction is automatically ") +
			//				AString("removed due to inactivity!"));
			//			f->EndLine();
		}
	}

	// TODO
	if (!exists) {
		if (quit == QUIT_AND_RESTART) {
			//			f->PutStr("You restarted your faction this turn. This faction "
			//				"has been removed, and a new faction has been started "
			//				"for you. (Your new faction report will come in a "
			//				"separate message.)");
		}
		else if (quit == QUIT_GAME_OVER) {
			//			f->PutStr("I'm sorry, the game has ended. Better luck in "
			//				"the next game you play!");
		}
		else if (quit == QUIT_WON_GAME) {
			//			f->PutStr("Congratulations, you have won the game!");
		}
		else {
			//			f->PutStr("I'm sorry, your faction has been eliminated.");
			//			// LLS
			//			f->PutStr("If you wish to restart, please let the "
			//				"Gamemaster know, and you will be restarted for "
			//				"the next available turn.");
		}
		//		f->PutStr("");
	}

	f->Key("factionStatus");
	f->StartObject();
	//	f->PutStr("Faction Status:");
	if (Globals->FACTION_LIMIT_TYPE == GameDefs::FACLIM_MAGE_COUNT) {
		f->Key("mages");
		f->Int(nummages);
		f->Key("maxMages");
		f->Int(pGame->AllowedMages(this));

		//		f->PutStr(AString("Mages: ") + nummages + " (" +
		//			pGame->AllowedMages(this) + ")");
		if (Globals->APPRENTICES_EXIST) {
			f->Key(Globals->APPRENTICE_NAME);
			f->Int(numapprentices);
			f->Key("maxApprentice");
			f->Int(pGame->AllowedApprentices(this));
			//			AString temp;
			//			temp = (char)toupper(Globals->APPRENTICE_NAME[0]);
			//			temp += Globals->APPRENTICE_NAME + 1;
			//			temp += "s: ";
			//			temp += numapprentices;
			//			temp += " (";
			//			temp += pGame->AllowedApprentices(this);
			//			temp += ")";
			//			f->PutStr(temp);
		}
	}
	else if (Globals->FACTION_LIMIT_TYPE == GameDefs::FACLIM_FACTION_TYPES) {
		f->Key("taxRegions");
		f->Int(war_regions.Num());
		f->Key("maxTaxRegions");
		f->Int(pGame->AllowedTaxes(this));

		f->Key("tradeRegions");
		f->Int(trade_regions.Num());
		f->Key("maxTradeRegions");
		f->Int(pGame->AllowedTrades(this));

		//		f->PutStr(AString("Tax Regions: ") + war_regions.Num() + " (" +
		//			pGame->AllowedTaxes(this) + ")");
		//		f->PutStr(AString("Trade Regions: ") + trade_regions.Num() + " (" +
		//			pGame->AllowedTrades(this) + ")");
		if (Globals->TRANSPORT & GameDefs::ALLOW_TRANSPORT) {
			f->Key("quartermasters");
			f->Int(numqms);
			f->Key("maxQuartermasters");
			f->Int(pGame->AllowedQuarterMasters(this));

			//			f->PutStr(AString("Quartermasters: ") + numqms + " (" +
			//				pGame->AllowedQuarterMasters(this) + ")");
		}
		if (Globals->TACTICS_NEEDS_WAR) {
			f->Key("tacticians");
			f->Int(numtacts);
			f->Key("maxTacticians");
			f->Int(pGame->AllowedTacticians(this));

			//			f->PutStr(AString("Tacticians: ") + numtacts + " (" +
			//				pGame->AllowedTacticians(this) + ")");
		}
		//		f->PutStr(AString("Mages: ") + nummages + " (" +
		//			pGame->AllowedMages(this) + ")");
		f->Key("mages");
		f->Int(nummages);
		f->Key("maxMages");
		f->Int(pGame->AllowedMages(this));

		if (Globals->APPRENTICES_EXIST) {
			f->Key(Globals->APPRENTICE_NAME);
			f->Int(numapprentices);
			f->Key("maxApprentice");
			f->Int(pGame->AllowedApprentices(this));

			//			AString temp;
			//			temp = (char)toupper(Globals->APPRENTICE_NAME[0]);
			//			temp += Globals->APPRENTICE_NAME + 1;
			//			temp += "s: ";
			//			temp += numapprentices;
			//			temp += " (";
			//			temp += pGame->AllowedApprentices(this);
			//			temp += ")";
			//			f->PutStr(temp);
		}
	}
	//	f->PutStr("");
	f->EndObject();

	if (errors.Num()) {
		f->Key("errors");
		f->StartArray();

		//		f->PutStr("Errors during turn:");
		forlist((&errors)) {
			//			f->PutStr(*((AString *)elem));
			f->String(*((AString *)elem));
		}
		// do not delete so they are exported via normal turn report
//		errors.DeleteAll();
//		f->EndLine();
		f->EndArray();
	}

	if (battles.Num()) {
		f->Key("Battles");
		f->StartArray();
		//		f->PutStr("Battles during turn:");
		forlist(&battles) {
			//			((BattlePtr *)elem)->ptr->Report(f, this);
			((BattlePtr *)elem)->ptr->ReportJSON(f, this);
		}
		// do not delete so they are exported via normal turn report
//		battles.DeleteAll();
		f->EndArray();
	}

	if (events.Num()) {
		f->Key("events");
		f->StartArray();
		//		f->PutStr("Events during turn:");
		forlist((&events)) {
			//			f->PutStr(*((AString *)elem));
			f->String(*((AString *)elem));
		}
		// do not delete so they are exported via normal turn report
//		events.DeleteAll();
//		f->EndLine();
		f->EndArray();
	}

	if (shows.Num()) {
		f->Key("skillReports");
		f->StartArray();
		//		f->PutStr("Skill reports:");
		forlist(&shows) {
			// TODO
			AString* string = ((ShowSkill *)elem)->Report(this);
			if (string) {
				//				f->PutStr("");
				//				f->PutStr(*string);
				f->String(*string);
			}
			delete string;
		}
		shows.DeleteAll();
		//		f->EndLine();
		f->EndArray();
	}

	if (itemshows.Num()) {
		f->Key("itemReports");
		f->StartArray();
		//		f->PutStr("Item reports:");
		forlist(&itemshows) {
			//			f->PutStr("");
			//			f->PutStr(*((AString *)elem));
			f->String(*((AString *)elem));
		}
		itemshows.DeleteAll();
		//		f->EndLine();
		f->EndArray();
	}

	if (objectshows.Num()) {
		f->Key("objectReports");
		f->StartArray();
		//		f->PutStr("Object reports:");
		forlist(&objectshows) {
			//			f->PutStr("");
			//			f->PutStr(*((AString *)elem));
			f->String(*((AString *)elem));
		}
		objectshows.DeleteAll();
		//		f->EndLine();
		f->EndArray();
	}

	/* Attitudes */
	f->Key("attitudes");
	f->StartObject();
	f->Key("default");
	f->String(AttitudeStrs[defaultattitude]);
	//	AString temp = AString("Declared Attitudes (default ") +
	//		AttitudeStrs[defaultattitude] + "):";
	//	f->PutStr(temp);
	for (int i = 0; i < NATTITUDES; i++) {
		int j = 0;
		AString temp = AString(AttitudeStrs[i]) + " : ";
		f->Key(AttitudeStrs[i]);
		f->StartArray();
		forlist((&attitudes)) {
			Attitude* a = (Attitude *)elem;
			if (a->attitude == i) {
				if (j) temp += ", ";
				Faction *fac = GetFaction(&(pGame->factions),
					a->factionnum);

				//				temp += *(GetFaction(&(pGame->factions),
				//					a->factionnum)->name);
				f->StartObject();
				f->Key("name");
				f->String(*fac->name);
				f->Key("num");
				f->Int(fac->num);
				f->EndObject();
				j = 1;
			}
		}
		//		if (!j) temp += "none";
		//		temp += ".";
		//		f->PutStr(temp);
		f->EndArray();
	}
	//	f->EndLine();
	f->EndObject();

	f->Key("unclaimedSilver");
	f->Int(unclaimed);
	//	temp = AString("Unclaimed silver: ") + unclaimed + ".";
	//	f->PutStr(temp);
	//	f->PutStr("");

	f->Key("regions");
	f->StartArray();
	forlist(&present_regions) {
		//		((ARegionPtr *)elem)->ptr->WriteReport(f, this, pGame->month, &(pGame->regions));
		((ARegionPtr *)elem)->ptr->WriteReportJSON(f, this, pGame->month, &(pGame->regions));
	}
	// LLS - maybe we don't want this -- I'll assume not, for now 
//f->PutStr("#end");
//	f->EndLine();
	f->EndArray();
	f->EndObject();
}

void Faction::FactionTypeStrJSON(AreportJSON *f)
{
	if (IsNPC())
	{
		f->String("NPC");
		return;
	}

	if (Globals->FACTION_LIMIT_TYPE == GameDefs::FACLIM_UNLIMITED) {
		f->String("Unlimited");
		return;
	}
	else if (Globals->FACTION_LIMIT_TYPE == GameDefs::FACLIM_MAGE_COUNT) {
		f->String("Normal");
		return;
	}
	else if (Globals->FACTION_LIMIT_TYPE == GameDefs::FACLIM_FACTION_TYPES) {
		f->StartObject();
		for (int i = 0; i < NFACTYPES; i++) {
			if (type[i]) {
				f->Key(FactionStrs[i]);
				f->Int(type[i]);
			}
		}
		f->EndObject();
	}
	return;
}

void mergeObjects(Writer<StringBuffer> *f, rapidjson::Value &srcObject)
{
	for (auto srcIt = srcObject.MemberBegin(); srcIt != srcObject.MemberEnd(); ++srcIt)
	{
		f->Key(srcIt->name.GetString());
		if (srcIt->value.IsArray())
		{
			f->StartArray();

			for (auto arrayIt = srcIt->value.Begin(); arrayIt != srcIt->value.End(); ++arrayIt)
			{
				auto obj = &arrayIt;
				//				dstIt->value.PushBack(*arrayIt, allocator);

				if (arrayIt->IsString())
					f->String(arrayIt->GetString());
				else if (arrayIt->IsInt())
					f->Int(arrayIt->GetInt());
				else if (arrayIt->IsBool())
					f->Bool(arrayIt->GetBool());
				else if (arrayIt->IsObject())
				{
					f->StartObject();
					mergeObjects(f, *arrayIt);
					f->EndObject();
				}
			}

			f->EndArray();
		}
		else if (srcIt->value.IsObject())
		{
			f->StartObject();

			mergeObjects(f, srcIt->value);

			f->EndObject();
		}
		else
		{
			if (srcIt->value.IsString())
				f->String(srcIt->value.GetString());
			else if (srcIt->value.IsInt())
				f->Int(srcIt->value.GetInt());
			else if (srcIt->value.IsBool())
				f->Bool(srcIt->value.GetBool());
		}
	}
}

void Battle::ReportJSON(AreportJSON * f, Faction * fac) {
/*	if (assassination == ASS_SUCC && fac != attacker) {
		f->String(*asstext);
//		f->PutStr(*asstext);
//		f->PutStr("");
		return;
	}
	forlist(&text) {
		AString * s = (AString *)elem;
		f->String(*s);
//		f->PutStr(*s);
	}
*/

	f->StartObject();

	Document d;
	d.Parse(s.GetString());
	mergeObjects(f->writer, d);

	delete jsonWriter;

	f->EndObject();
}

void ARegion::WagesForReportJSON(AreportJSON *f)
{
	Production *p = products.GetProd(I_SILVER, -1);
	if (p) {
		f->Key("wages");
		f->Double((p->productivity / 10));
		f->Key("maxWages");
		f->Double(p->amount);

		//		return AString("$") + (p->productivity / 10) +
		//			"." + (p->productivity % 10) + " (Max: $" + p->amount + ")";
	}
	else
	{
		f->Key("wages");
		f->Double(0);
	}
	//		return AString("$") + 0;
}

void ARegion::ShortPrintJSON(Writer<StringBuffer> *f, ARegionList *pRegs)
{
	f->Key("type");
	f->String(TerrainDefs[type].name);
	//	AString temp = TerrainDefs[type].name;

	f->Key("x");
	f->Int(xloc);
	f->Key("y");
	f->Int(yloc);

	//	temp += AString(" (") + xloc + "," + yloc;

	ARegionArray *pArr = pRegs->pRegionArrays[zloc];
	if (pArr->strName) {
		//		temp += ",";
		if (Globals->EASIER_UNDERWORLD &&
			(Globals->UNDERWORLD_LEVELS + Globals->UNDERDEEP_LEVELS > 1)) {
			//			temp += AString("") + zloc + " <";
			f->Key("z");
			f->Int(zloc);
		}
		else {
			f->Key("z");
			f->Int(zloc);
			// TODO
			/*			// add less explicit multilevel information about the underworld
						if (zloc > 2 && zloc < Globals->UNDERWORLD_LEVELS + 2) {
							for (int i = zloc; i > 3; i--) {
								temp += "very ";
							}
							temp += "deep ";
						}
						else if ((zloc > Globals->UNDERWORLD_LEVELS + 2) &&
							(zloc < Globals->UNDERWORLD_LEVELS +
								Globals->UNDERDEEP_LEVELS + 2)) {
							for (int i = zloc; i > Globals->UNDERWORLD_LEVELS + 3; i--) {
								temp += "very ";
							}
							temp += "deep ";
						}
			*/
		}
		f->Key("level");
		f->String((*pArr->strName).Str());
		//		temp += *pArr->strName;
		if (Globals->EASIER_UNDERWORLD &&
			(Globals->UNDERWORLD_LEVELS + Globals->UNDERDEEP_LEVELS > 1)) {
			//			temp += ">";
		}
	}
	//	temp += ")";

	f->Key("region");
	f->String((*name).Str());
	//	temp += AString(" in ") + *name;
}

void ARegion::PrintJSON(AreportJSON *f, ARegionList *pRegs)
{
	ShortPrintJSON(f->writer, pRegs);
	if (town) {
		f->Key("townName");
		f->String(*(town->name));
		f->Key("townType");
		f->String(TownString(town->TownType()));
		//		temp += AString(", contains ") + *(town->name) + " [" +
		//			TownString(town->TownType()) + "]";
	}
}

void ARegion::WriteProductsJSON(AreportJSON *f, Faction *fac, int present)
{
	f->Key("products");
	f->StartArray();
	//	AString temp = "Products: ";
	int has = 0;
	forlist((&products)) {
		Production *p = ((Production *)elem);
		if (ItemDefs[p->itemtype].type & IT_ADVANCED) {
			if (CanMakeAdv(fac, p->itemtype) || (fac->IsNPC())) {
				if (has) {
					//					temp += AString(", ") + p->WriteReport();
				}
				else {
					has = 1;
					//					temp += p->WriteReport();
					f->StartObject();
					p->WriteReportJSON(f);
					f->EndObject();
				}
			}
		}
		else {
			if (p->itemtype == I_SILVER) {
				if (p->skill == S_ENTERTAINMENT) {
					f->StartObject();
					if ((Globals->TRANSIT_REPORT &
						GameDefs::REPORT_SHOW_ENTERTAINMENT) || present) {
						//						f->PutStr(AString("Entertainment available: $") +
						//							p->amount + ".");
						f->Key("entertainment");
						f->Int(p->amount);
					}
					else {
						//						f->PutStr(AString("Entertainment available: $0."));
						f->Key("entertainment");
						f->Int(0);
					}
					f->EndObject();
				}
			}
			else {
				if (!present &&
					!(Globals->TRANSIT_REPORT &
						GameDefs::REPORT_SHOW_RESOURCES))
					continue;
				f->StartObject();
				if (has) {
					//					temp += AString(", ") + p->WriteReport();
				}
				else {
					has = 1;
					//					temp += p->WriteReport();
				}
				p->WriteReportJSON(f);
				f->EndObject();
			}
		}
	}

	//	if (has == 0) temp += "none";
	//	temp += ".";
	//	f->PutStr(temp);
	f->EndArray();
}

void ARegion::WriteMarketsJSON(AreportJSON *f, Faction *fac, int present)
{
	f->Key("wanted");
	f->StartArray();

	AString temp = "Wanted: ";
	int has = 0;
	forlist(&markets) {
		Market *m = (Market *)elem;
		if (!m->amount) continue;
		if (!present &&
			!(Globals->TRANSIT_REPORT & GameDefs::REPORT_SHOW_MARKETS))
			continue;
		if (m->type == M_SELL) {
			if (ItemDefs[m->item].type & IT_ADVANCED) {
				if (!Globals->MARKETS_SHOW_ADVANCED_ITEMS) {
					if (!HasItem(fac, m->item)) {
						continue;
					}
				}
			}
			if (has) {
				temp += ", ";
			}
			else {
				has = 1;
			}
			f->StartObject();
			m->ReportJSON(f);
			f->EndObject();
		}
	}
	//	if (!has) temp += "none";
	//	temp += ".";
		//f->PutStr(temp);
	f->EndArray();

	//	temp = "For Sale: ";
	has = 0;
	f->Key("forSale");
	f->StartArray();
	{
		forlist(&markets) {
			Market *m = (Market *)elem;
			if (!m->amount) continue;
			if (!present &&
				!(Globals->TRANSIT_REPORT & GameDefs::REPORT_SHOW_MARKETS))
				continue;
			if (m->type == M_BUY) {
				if (has) {
					temp += ", ";
				}
				else {
					has = 1;
				}
				//				temp += m->Report();
				f->StartObject();
				m->ReportJSON(f);
				f->EndObject();
			}
		}
	}
	//	if (!has) temp += "none";
	//	temp += ".";
	//	f->PutStr(temp);
	f->EndArray();
}

void ARegion::WriteEconomyJSON(AreportJSON *f, Faction *fac, int present)
{
	//	f->AddTab();
	f->Key("economy");
	f->StartObject();

	if ((Globals->TRANSIT_REPORT & GameDefs::REPORT_SHOW_WAGES) || present) {
		//		f->PutStr(AString("Wages: ") + WagesForReport() + ".");
		WagesForReport();
	}
	else {
		//		f->PutStr(AString("Wages: $0."));
		f->Key("Wages");
		f->Int(0);
	}

	WriteMarketsJSON(f, fac, present);
	WriteProductsJSON(f, fac, present);

	//	f->EndLine();
	//	f->DropTab();
	f->EndObject();
}

void ARegion::WriteExitsJSON(AreportJSON *f, ARegionList *pRegs, int *exits_seen)
{
	//	f->PutStr("Exits:");
	//	f->AddTab();
	int y = 0;
	for (int i = 0; i < NDIRS; i++) {
		ARegion *r = neighbors[i];
		if (r && exits_seen[i]) {
			f->Key(DirectionStrs[i]);
			f->StartObject();
			r->PrintJSON(f, pRegs);
			f->EndObject();
			//			f->PutStr(AString(DirectionStrs[i]) + " : " +
			//				r->Print(pRegs) + ".");
			y = 1;
		}
	}
	//	if (!y) f->PutStr("none");
	//	f->DropTab();
	//	f->EndLine();
}

void WriteUnitsJSON(AList *objects, AreportJSON *f, Faction *fac, int obs, int truesight,
	int detfac, int passobs, int passtrue, int passdetfac, int present) {
	forlist(objects) {
		Object *o = (Object *)elem;
		if (!o->IsFleet() && o->type == O_DUMMY) {
			o->ReportJSON(f, fac, obs, truesight, detfac,
				passobs, passtrue, passdetfac,
				present);
		}
	}
}

void WriteStructuresJSON(AList *objects, AreportJSON *f, Faction *fac, int obs, int truesight,
	int detfac, int passobs, int passtrue, int passdetfac, int present) {
	forlist(objects) {
		Object *o = (Object *)elem;
		if (o->type != O_DUMMY) {
			o->ReportJSON(f, fac, obs, truesight, detfac,
				passobs, passtrue, passdetfac,
				present);
		}
	}
}

void ARegion::WriteReportJSON(AreportJSON *f, Faction *fac, int month,
	ARegionList *pRegions)
{
	Farsight *farsight = GetFarsight(&farsees, fac);
	Farsight *passer = GetFarsight(&passers, fac);
	int present = Present(fac) || fac->IsNPC();

	if (farsight || passer || present) {
		f->StartObject();
		AString temp;
		//		AString temp = Print(pRegions);
		PrintJSON(f, pRegions);
		if (Population() &&
			(present || farsight ||
			(Globals->TRANSIT_REPORT & GameDefs::REPORT_SHOW_PEASANTS))) {
			f->Key("population");
			f->Int(Population());
			//			temp += AString(", ") + Population() + " peasants";
			if (Globals->RACES_EXIST) {
				//				temp += AString(" (") + ItemDefs[race].names + ")";
				f->Key("populationRace");
				f->String(ItemDefs[race].names);
			}
			if (present || farsight ||
				Globals->TRANSIT_REPORT & GameDefs::REPORT_SHOW_REGION_MONEY) {
				//				temp += AString(", $") + wealth;
				f->Key("wealth");
				f->Int(wealth);
			}
			else {
				f->Key("wealth");
				f->Int(0);
				//				temp += AString(", $0");
			}
		}
		//		temp += ".";
		//		f->PutStr(temp);
		//		f->PutStr("-------------------------------------------------"
		//			"-----------");

		//		f->AddTab();
		if (Globals->WEATHER_EXISTS) {
			temp = "It was ";
			if (clearskies) temp += "unnaturally clear ";
			else {
				if (weather == W_BLIZZARD) temp = "There was an unnatural ";
				else if (weather == W_NORMAL) temp = "The weather was ";
				temp += SeasonNames[weather];
			}
			temp += " last month; ";
			int nxtweather = pRegions->GetWeather(this, (month + 1) % 12);
			temp += "it will be ";
			temp += SeasonNames[nxtweather];
			temp += " next month.";
			//			f->PutStr(temp);
			f->Key("description");
			f->String(temp);
		}

#if 0
		f->PutStr("");
		temp = "Elevation is ";
		f->PutStr(temp + elevation);
		temp = "Humidity is ";
		f->PutStr(temp + humidity);
		temp = "Temperature is ";
		f->PutStr(temp + temperature);
#endif

		if (type == R_NEXUS) {
			int len = strlen(AC_STRING) + 2 * strlen(Globals->WORLD_NAME);
			char *nexus_desc = new char[len];
			sprintf(nexus_desc, AC_STRING, Globals->WORLD_NAME,
				Globals->WORLD_NAME);
			f->Key("description");
			f->String(nexus_desc);
			//			f->PutStr("");
			//			f->PutStr(nexus_desc);
			//			f->PutStr("");
			delete[] nexus_desc;
		}

		//		f->DropTab();

		//		WriteEconomy(f, fac, present || farsight);
		WriteEconomyJSON(f, fac, present || farsight);

		int exits_seen[NDIRS];
		if (present || farsight ||
			(Globals->TRANSIT_REPORT & GameDefs::REPORT_SHOW_ALL_EXITS)) {
			for (int i = 0; i < NDIRS; i++)
				exits_seen[i] = 1;
		}
		else {
			// This is just a transit report and we're not showing all
			// exits.   See if we are showing used exits.

			// Show none by default.
			int i;
			for (i = 0; i < NDIRS; i++)
				exits_seen[i] = 0;
			// Now, if we should, show the ones actually used.
			if (Globals->TRANSIT_REPORT & GameDefs::REPORT_SHOW_USED_EXITS) {
				forlist(&passers) {
					Farsight *p = (Farsight *)elem;
					if (p->faction == fac) {
						for (i = 0; i < NDIRS; i++) {
							exits_seen[i] |= p->exits_used[i];
						}
					}
				}
			}
		}

		f->Key("exits");
		f->StartObject();
		WriteExitsJSON(f, pRegions, exits_seen);

		if (Globals->GATES_EXIST && gate && gate != -1) {
			int sawgate = 0;
			if (fac->IsNPC())
				sawgate = 1;
			if (Globals->IMPROVED_FARSIGHT && farsight) {
				forlist(&farsees) {
					Farsight *watcher = (Farsight *)elem;
					if (watcher && watcher->faction == fac && watcher->unit) {
						if (watcher->unit->GetSkill(S_GATE_LORE)) {
							sawgate = 1;
						}
					}
				}
			}
			if (Globals->TRANSIT_REPORT & GameDefs::REPORT_USE_UNIT_SKILLS) {
				forlist(&passers) {
					Farsight *watcher = (Farsight *)elem;
					if (watcher && watcher->faction == fac && watcher->unit) {
						if (watcher->unit->GetSkill(S_GATE_LORE)) {
							sawgate = 1;
						}
					}
				}
			}
			forlist(&objects) {
				Object *o = (Object *)elem;
				forlist(&o->units) {
					Unit *u = (Unit *)elem;
					if (!sawgate &&
						((u->faction == fac) &&
							u->GetSkill(S_GATE_LORE))) {
						sawgate = 1;
					}
				}
			}
			if (sawgate) {
				f->Key("Gate");
				f->StartObject();
				if (gateopen) {

					f->Key("open");
					f->Bool(true);
					f->Key("num");
					f->Int(gate);

					//					AString temp;
					//					temp = "There is a Gate here (Gate ";
					//					temp += gate;
					if (!Globals->DISPERSE_GATE_NUMBERS) {
						//						temp += " of ";
						//						temp += pRegions->numberofgates;
						f->Key("numGates");
						f->Int(pRegions->numberofgates);
					}
					//					temp += ").";
					//					f->PutStr(temp);
					//					f->PutStr("");
				}
				else if (Globals->SHOW_CLOSED_GATES) {
					//					f->PutStr(AString("There is a closed Gate here."));
					//					f->PutStr("");
					f->Key("open");
					f->Bool(false);
				}
				f->EndObject();
			}
		}
		f->EndObject();

		int obs = GetObservation(fac, 0);
		int truesight = GetTrueSight(fac, 0);
		int detfac = 0;

		int passobs = GetObservation(fac, 1);
		int passtrue = GetTrueSight(fac, 1);
		int passdetfac = detfac;

		if (fac->IsNPC()) {
			obs = 10;
			passobs = 10;
		}

		forlist(&objects) {
			Object *o = (Object *)elem;
			forlist(&o->units) {
				Unit *u = (Unit *)elem;
				if (u->faction == fac && u->GetSkill(S_MIND_READING) > 2) {
					detfac = 1;
				}
			}
		}
		if (Globals->IMPROVED_FARSIGHT && farsight) {
			forlist(&farsees) {
				Farsight *watcher = (Farsight *)elem;
				if (watcher && watcher->faction == fac && watcher->unit) {
					if (watcher->unit->GetSkill(S_MIND_READING) > 2) {
						detfac = 1;
					}
				}
			}
		}

		if ((Globals->TRANSIT_REPORT & GameDefs::REPORT_USE_UNIT_SKILLS) &&
			(Globals->TRANSIT_REPORT & GameDefs::REPORT_SHOW_UNITS)) {
			forlist(&passers) {
				Farsight *watcher = (Farsight *)elem;
				if (watcher && watcher->faction == fac && watcher->unit) {
					if (watcher->unit->GetSkill(S_MIND_READING) > 2) {
						passdetfac = 1;
					}
				}
			}
		}

		if (objects.Num())
		{
			f->Key("units");
			f->StartArray();
			WriteUnitsJSON(&objects, f, fac, obs, truesight, detfac, passobs, passtrue, passdetfac, present || farsight);
			f->EndArray();

			f->Key("structures");
			f->StartArray();
			WriteStructuresJSON(&objects, f, fac, obs, truesight, detfac, passobs, passtrue, passdetfac, present || farsight);
			f->EndArray();
		}
		f->EndObject();
	}
}

void ItemStringJSON(Writer<StringBuffer> *f, int type, int num, int flags)
{
	if (num == 1) {
		if (flags & FULLNUM)
		{
			//			temp += AString(num) + " ";
			f->Key("amount");
			f->Int(num);
		}

		f->Key("type");
		(flags & ALWAYSPLURAL) ?
			f->String(ItemDefs[type].names) :
			f->String(ItemDefs[type].name);
		f->Key("abr");
		f->String(ItemDefs[type].abr);
		//		temp +=
		//			AString((flags & ALWAYSPLURAL) ?
		//				ItemDefs[type].names : ItemDefs[type].name) +
		//			" [" + ItemDefs[type].abr + "]";
	}
	else {
		if (num == -1) {
			f->Key("amount");
			f->String("unlimited");

			f->Key("type");
			f->String(ItemDefs[type].names);
			f->Key("abr");
			f->String(ItemDefs[type].abr);

			//			temp += AString("unlimited ") + ItemDefs[type].names + " [" +
			//				ItemDefs[type].abr + "]";
		}
		else {
			f->Key("amount");
			f->Int(num);

			f->Key("type");
			f->String(ItemDefs[type].names);
			f->Key("abr");
			f->String(ItemDefs[type].abr);

			//			temp += AString(num) + " " + ItemDefs[type].names + " [" +
			//				ItemDefs[type].abr + "]";
		}
	}
}

void Item::ReportJSON(Writer<StringBuffer>*f, int seeillusions)
{
	// special handling of the unfinished ship items
	if (ItemDefs[type].type & IT_SHIP) {
		f->Key("finished");
		f->Bool(false);

		f->Key("type");
		f->String(ItemDefs[type].name);
		f->Key("abr");
		f->String(ItemDefs[type].abr);
		f->Key("needs");
		f->Int(num);

		//		ret += AString("unfinished ") + ItemDefs[type].name +
		//			" [" + ItemDefs[type].abr + "] (needs " + num + ")";
	}
	else {
		//		ret += ItemString(type, num);
		ItemStringJSON(f, type, num);
	}
	if (seeillusions && (ItemDefs[type].type & IT_ILLUSION)) {
		f->Key("illusion");
		f->Bool(true);
		//		ret = ret + " (illusion)";
	}
}

void ItemList::ReportJSON(Writer<StringBuffer> *f, int obs, int seeillusions, int nofirstcomma)
{
	f->StartArray();
	UncheckAll();
	AString temp;
	for (int s = 0; s < 7; s++) {
		//		temp += ReportByType(s, obs, seeillusions, nofirstcomma);
		ReportByTypeJSON(f, s, obs, seeillusions, nofirstcomma);
		//		if (temp.Len()) nofirstcomma = 0;
	}
	f->EndArray();
}

void ItemList::BattleReportJSON(Writer<StringBuffer> *jsonWriter)
{
	forlist(this) {
		Item *i = (Item *)elem;
		if (ItemDefs[i->type].combat) {
			jsonWriter->StartObject();
			//			temp += ", ";
			//			temp += i->Report(0);
			i->ReportJSON(jsonWriter, 0);
			if (ItemDefs[i->type].type & IT_MONSTER) {
				MonType *mp = FindMonster(ItemDefs[i->type].abr,
					(ItemDefs[i->type].type & IT_ILLUSION));
				jsonWriter->Key("monster");
				jsonWriter->StartObject();
				jsonWriter->Key("attack");
				jsonWriter->Int(mp->attackLevel);
				jsonWriter->Key("defense");
				jsonWriter->Int(mp->defense[ATTACK_COMBAT]);
				jsonWriter->Key("attacks");
				jsonWriter->Int(mp->numAttacks);
				jsonWriter->Key("hits");
				jsonWriter->Int(mp->hits);
				jsonWriter->Key("tactics");
				jsonWriter->Int(mp->tactics);

				//				temp += AString(" (Combat ") + mp->attackLevel +
				//					"/" + mp->defense[ATTACK_COMBAT] + ", Attacks " +
				//					mp->numAttacks + ", Hits " + mp->hits +
				//					", Tactics " + mp->tactics + ")";
				jsonWriter->EndObject();
			}
			jsonWriter->EndObject();
		}
	}
}

void ItemList::ReportByTypeJSON(Writer<StringBuffer> *f, int type, int obs, int seeillusions,
	int nofirstcomma)
{
	//	AString temp;
	forlist(this) {
		int report = 0;
		Item *i = (Item *)elem;
		if (i->checked) continue;
		switch (type) {
		case 0:
			if (ItemDefs[i->type].type & IT_MAN)
				report = 1;
			break;
		case 1:
			if (ItemDefs[i->type].type & IT_MONSTER)
				report = 1;
			break;
		case 2:
			if ((ItemDefs[i->type].type & IT_WEAPON) ||
				(ItemDefs[i->type].type & IT_BATTLE) ||
				(ItemDefs[i->type].type & IT_ARMOR) ||
				(ItemDefs[i->type].type & IT_MAGIC))
				report = 1;
			break;
		case 3:
			if (ItemDefs[i->type].type & IT_MOUNT)
				report = 1;
			break;
		case 4:
			if ((i->type == I_WAGON) || (i->type == I_MWAGON))
				report = 1;
			break;
		case 5:
			report = 1;
			if (ItemDefs[i->type].type & IT_MAN)
				report = 0;
			if (ItemDefs[i->type].type & IT_MONSTER)
				report = 0;
			if (i->type == I_SILVER)
				report = 0;
			if ((ItemDefs[i->type].type & IT_WEAPON) ||
				(ItemDefs[i->type].type & IT_BATTLE) ||
				(ItemDefs[i->type].type & IT_ARMOR) ||
				(ItemDefs[i->type].type & IT_MAGIC))
				report = 0;
			if (ItemDefs[i->type].type & IT_MOUNT)
				report = 0;
			if ((i->type == I_WAGON) ||
				(i->type == I_MWAGON))
				report = 0;
			break;
		case 6:
			if (i->type == I_SILVER)
				report = 1;
		}
		if (report) {

			if (obs == 2) {
				if (nofirstcomma) nofirstcomma = 0;
				//				else temp += ", ";
				//				temp += i->Report(seeillusions);
				f->StartObject();
				i->ReportJSON(f, seeillusions);
				f->EndObject();
			}
			else {
				if (ItemDefs[i->type].weight) {
					if (nofirstcomma) nofirstcomma = 0;
					//					else temp += ", ";
					//					temp += i->Report(seeillusions);
					f->StartObject();
					i->ReportJSON(f, seeillusions);
					f->EndObject();
				}
			}
			i->checked = 1;
		}
	}
}

void Market::ReportJSON(AreportJSON *f)
{
	ItemStringJSON(f->writer, item, amount);

	f->Key("price");
	f->Int(price);
}

void WriteUnitToJSON(AreportJSON *f, Faction *fac, Unit *u, int obs, int truesight,
	int detfac, int passobs, int passtrue, int passdetfac, int present, int outdoorUnit) {
	int attitude = fac->GetAttitude(u->faction->num);
	if (u->faction == fac) {
		u->WriteReportJSON(f, -1, 1, 1, 1, attitude, fac->showunitattitudes);
	}
	else {
		if (present) {
			u->WriteReportJSON(f, obs, truesight, detfac, !outdoorUnit, attitude, fac->showunitattitudes);
		}
		else {
			// show outdoor units in tranist report
			if ((outdoorUnit && (Globals->TRANSIT_REPORT & GameDefs::REPORT_SHOW_OUTDOOR_UNITS))
				// show indro units in tranit report
				|| (!outdoorUnit && (Globals->TRANSIT_REPORT & GameDefs::REPORT_SHOW_INDOOR_UNITS))
				// show guards in tranist report
				|| ((u->guard == GUARD_GUARD) && (Globals->TRANSIT_REPORT & GameDefs::REPORT_SHOW_GUARDS))) {
				u->WriteReportJSON(f, passobs, passtrue, passdetfac,
					!outdoorUnit, attitude, fac->showunitattitudes);
			}
		}
	}
}

void WriteObjectUnitsJSON(AreportJSON *f, Faction *fac, Object *o, int obs, int truesight,
	int detfac, int passobs, int passtrue, int passdetfac, int present) {

	if (o->units.Num()) {
		forlist((&(o->units))) {
			Unit *u = (Unit *)elem;
			WriteUnitToJSON(f, fac, u, obs, truesight, detfac, passobs, passtrue, passdetfac, present, o->type == O_DUMMY);
		}
	}
}

void WriteBuildingPropsToJSON(AreportJSON *f, Faction *fac, ObjectType *ob, Object *o, int obs, int truesight,
	int detfac, int passobs, int passtrue, int passdetfac, int present) {

	AString *cleanName = GetCleanName(o->name, o->num);

	f->Key("name");
	f->String(*cleanName);
	delete cleanName;

	f->Key("num");
	f->Int(o->num);

	f->Key("type");
	f->String(ob->name);

	if (o->describe) {
		f->Key("description");
		f->String(*(o->describe));
	}

	int incomplete = 0;
	int willDecay = 0;
	int maintenance = 0;

	if (o->incomplete > 0) {
		f->Key("needs");
		f->Int(o->incomplete);
		incomplete = 1;
	}
	else if (Globals->DECAY && !(ob->flags & ObjectType::NEVERDECAY) && o->incomplete < 1) {
		if (o->incomplete > (0 - ob->maxMonthlyDecay)) {
			willDecay = 1;
		}
		else if (o->incomplete > (0 - ob->maxMaintenance / 2)) {
			maintenance = 1;
		}
	}

	f->Key("flags");
	f->StartArray();
	//if (incomplete) {
	//    f->String("incomplete");
	//}

	if (willDecay) {
		f->String("about to decay");
	}

	if (maintenance) {
		f->String("needs maintenance");
	}

	if (o->inner != -1) {
		f->String("contains an inner location");
	}

	if (o->runes) {
		f->String("engraved with Runes of Warding");
	}

	if (!(ob->flags & ObjectType::CANENTER)) {
		f->String("closed to player units");
	}
	f->EndArray();


	f->Key("units");
	f->StartArray();
	WriteObjectUnitsJSON(f, fac, o, obs, truesight, detfac, passobs, passtrue, passdetfac, present);
	f->EndArray();
}

void WriteFleetTypeAndStructureJSON(AreportJSON *f, Object *o) {
	int shiptype = -1;
	int num = 0;

	for (int i = 0; i < NITEMS; i++) {
		if (ItemDefs[i].type & IT_SHIP) {
			int sn = o->GetNumShips(i);
			if (sn > 0) {
				num += sn;
				shiptype = i;
			}
		}
	}

	f->Key("type");
	if (num == 1) {
		f->String(ItemDefs[shiptype].name);
	}
	else {
		f->String(ObjectDefs[o->type].name);
	}

	f->Key("contents");
	f->StartArray();
	for (int item = 0; item < NITEMS; item++) {
		num = o->GetNumShips(item);
		if (num <= 0) continue;

		f->StartObject();

		f->Key("name");
		if (num > 1) {
			f->String(ItemDefs[item].names);
		}
		else {
			f->String(ItemDefs[item].name);
		}

		f->Key("type");
		f->String(ItemDefs[item].name);

		f->Key("count");
		f->Int(num);

		f->EndObject();
	}
	f->EndArray();
}

void WriteFleetPropsToJSON(AreportJSON *f, Faction *fac, ObjectType *ob, Object *o, int obs, int truesight,
	int detfac, int passobs, int passtrue, int passdetfac, int present) {

	AString *cleanName = GetCleanName(o->name, o->num);

	f->Key("name");
	f->String(*cleanName);
	delete cleanName;

	f->Key("num");
	f->Int(o->num);

	WriteFleetTypeAndStructureJSON(f, o);

	if (o->describe) {
		f->Key("description");
		f->String(*(o->describe));
	}

	if ((o->GetOwner() && fac == o->GetOwner()->faction) || (obs > 9)) {
		if (o->incomplete > 0) {
			//f->Key("flags");
			//f->StartArray();
			//    f->String("incomplete");
			//f->EndArray();

			f->Key("needs");
			f->Int(o->incomplete);
		}

		f->Key("load");
		f->Int(o->FleetLoad());

		f->Key("capacity");
		f->Int(o->FleetCapacity());

		f->Key("sailors");
		f->StartObject();
		f->Key("avaliable");
		f->Int(o->FleetSailingSkill(1));

		f->Key("required");
		f->Int(o->GetFleetSize());
		f->EndObject();

		f->Key("maxSpeed");
		f->Int(o->GetFleetSpeed(1));
	}

	if ((Globals->PREVENT_SAIL_THROUGH) && (!Globals->ALLOW_TRIVIAL_PORTAGE)) {
		if ((o->flying < 1) && (TerrainDefs[o->region->type].similar_type != R_OCEAN)) {
			int dir = 0;
			int first = 1;
			f->Key("sailDirections");
			f->StartArray();
			for (dir = 0; dir < NDIRS; dir++) {
				if (o->SailThroughCheck(dir) == 1) {
					f->String(DirectionAbrs[dir]);
				}
			}
			f->EndArray();
		}
	}

	f->Key("units");
	f->StartArray();
	WriteObjectUnitsJSON(f, fac, o, obs, truesight, detfac, passobs, passtrue, passdetfac, present);
	f->EndArray();
}

void Object::ReportJSON(AreportJSON *f, Faction *fac, int obs, int truesight,
	int detfac, int passobs, int passtrue, int passdetfac, int present)
{
	ObjectType *ob = &ObjectDefs[type];

	if ((type != O_DUMMY) && !present) {
		if (IsFleet() &&
			!(Globals->TRANSIT_REPORT & GameDefs::REPORT_SHOW_SHIPS)) {
			// This is a ship and we don't see ships in transit
			return;
		}
		if (IsBuilding() &&
			!(Globals->TRANSIT_REPORT & GameDefs::REPORT_SHOW_BUILDINGS)) {
			// This is a building and we don't see buildings in transit
			return;
		}
		if (IsRoad() &&
			!(Globals->TRANSIT_REPORT & GameDefs::REPORT_SHOW_ROADS)) {
			// This is a road and we don't see roads in transit
			return;
		}
	}

	if (IsFleet()) {
		f->StartObject();
		WriteFleetPropsToJSON(f, fac, ob, this, obs, truesight, detfac, passobs, passtrue, passdetfac, present);
		f->EndObject();

		return;
	}

	if (type != O_DUMMY) {
		f->StartObject();
		WriteBuildingPropsToJSON(f, fac, ob, this, obs, truesight, detfac, passobs, passtrue, passdetfac, present);
		f->EndObject();

		return;
	}

	WriteObjectUnitsJSON(f, fac, this, obs, truesight, detfac, passobs, passtrue, passdetfac, present);
}

void Production::WriteReportJSON(AreportJSON *f)
{
	ItemStringJSON(f->writer, itemtype, amount);
	//	AString temp = ItemString(itemtype, amount);
	//	return temp;
}

void Unit::MageReportJSON(AreportJSON *f)
{
	if (combat != -1) {
		f->Key("combatSpell");
		f->StartObject();
		WriteSkillPropsToJSON(f, combat);
		f->EndObject();
	}
}

void Unit::ReadyItemJSON(AreportJSON *f)
{
	int i, ready;

	f->Key("ready");
	f->StartObject();
	f->Key("weapon");
	f->StartArray();
	for (i = 0; i < MAX_READY; ++i) {
		ready = readyWeapon[i];
		if (ready != -1) {
			ItemStringJSON(f->writer, ready, 1);
		}
	}
	f->EndArray();

	f->Key("armor");
	f->StartArray();
	for (i = 0; i < MAX_READY; ++i) {
		ready = readyArmor[i];
		if (ready != -1) {
			ItemStringJSON(f->writer, ready, 1);
		}
	}
	f->EndArray();

	if (readyItem != -1) {
		f->Key("item");
		ItemStringJSON(f->writer, readyItem, 1);
	}
	f->EndObject();
}

void Unit::StudyableSkillsJSON(AreportJSON *f)
{
	f->Key("canStudy");
	f->StartArray();
	for (int i = 0; i < NSKILLS; i++) {
		if (SkillDefs[i].depends[0].skill != NULL) {
			if (CanStudy(i)) {
				f->StartObject();
				WriteSkillPropsToJSON(f, i);
				f->EndObject();
			}
		}
	}
	f->EndArray();
}

void Unit::GetNameJSON(Writer<StringBuffer> *f, int obs)
{
	f->Key("name");
	f->String((*name).Str());
	//	AString ret = *name;
	int stealth = GetAttribute("stealth");
	if (reveal == REVEAL_FACTION || obs > stealth) {
		//		ret += ", ";
		//		ret += *faction->name;
		f->Key("faction");
		f->String((*faction->name).Str());
	}
}

void Unit::SpoilsReportJSON(AreportJSON *f) {
	if (GetFlag(FLAG_NOSPOILS) ||
		GetFlag(FLAG_FLYSPOILS) ||
		GetFlag(FLAG_SAILSPOILS) ||
		GetFlag(FLAG_WALKSPOILS) ||
		GetFlag(FLAG_RIDESPOILS))
	{
		f->Key("spoils");
		f->StartArray();
		if (GetFlag(FLAG_NOSPOILS)) {
			//temp = ", weightless battle spoils";
			f->String("weightless");
		}
		else if (GetFlag(FLAG_FLYSPOILS)) {
			//temp = ", flying battle spoils";
			f->String("flying");
		}
		else if (GetFlag(FLAG_WALKSPOILS)) {
			//		temp = ", walking battle spoils";
			f->String("walking");
		}
		else if (GetFlag(FLAG_RIDESPOILS)) {
			//temp = ", riding battle spoils";
			f->String("riding");
		}
		else if (GetFlag(FLAG_SAILSPOILS)) {
			//		temp = ", sailing battle spoils";
			f->String("sailing");
		}
		//	return temp;
		f->EndArray();
	}
}

void Unit::WriteReportJSON(AreportJSON *f, int obs, int truesight, int detfac,
	int autosee, int attitude, int showattitudes)
{
	int stealth = GetAttribute("stealth");

	if (obs == -1) {
		/* The unit belongs to the Faction writing the report */
		obs = 2;
	}
	else {
		if (obs < stealth) {
			/* The unit cannot be seen */
			if (reveal == REVEAL_FACTION) {
				obs = 1;
			}
			else {
				if (guard == GUARD_GUARD || reveal == REVEAL_UNIT || autosee) {
					obs = 0;
				}
				else {
					return;
				}
			}
		}
		else {
			if (obs == stealth) {
				/* Can see unit, but not Faction */
				if (reveal == REVEAL_FACTION) {
					obs = 1;
				}
				else {
					obs = 0;
				}
			}
			else {
				/* Can see unit and Faction */
				obs = 1;
			}
		}
	}

	/* Setup True Sight */
	if (obs == 2) {
		truesight = 1;
	}
	else {
		if (GetSkill(S_ILLUSION) > truesight) {
			truesight = 0;
		}
		else {
			truesight = 1;
		}
	}

	if (detfac && obs != 2) obs = 1;

	/* Write the report */
	f->StartObject();

	AString *cleanName = GetCleanName(name, num);

	f->Key("name");
	f->String(*cleanName);

	delete cleanName;

	f->Key("num");
	f->Int(num);

	if (obs > 0) {
		f->Key("faction");
		f->StartObject();
		f->Key("name");
		f->String(*faction->name);

		f->Key("num");
		f->Int(faction->num);
		f->EndObject();
	}

	if (describe) {
		f->Key("description");
		f->String(*describe);
	}

	f->Key("own");
	f->Bool(obs == 2);

	if (obs != 2 && showattitudes) {
		f->Key("attitude");
		switch (attitude) {
		case A_ALLY:
			f->String("ally");
			break;
		case A_FRIENDLY:
			f->String("friendly");
			break;
		case A_NEUTRAL:
			f->String("neutal");
			break;
		case A_UNFRIENDLY:
			f->String("unfriendly");
			break;
		case A_HOSTILE:
			f->String("hostile");
			break;
		}
	}

	if (obs == 2) {
		f->Key("revealing");
		switch (reveal)
		{
		case REVEAL_UNIT:
			f->String("unit");
			break;

		case REVEAL_FACTION:
			f->String("faction");
			break;

		default:
			f->Bool(0);
			break;
		}

		f->Key("consume");
		if (GetFlag(FLAG_CONSUMING_UNIT)) {
			f->String("consuming unit's food");
		}
		else if (GetFlag(FLAG_CONSUMING_FACTION)) {
			f->String("consuming faction's food");
		}
		else {
			f->String("silver");
		}

		f->Key("weight");
		f->Int(items.Weight());

		f->Key("capacity");
		f->StartObject();
		f->Key("flying");
		f->Int(FlyingCapacity());

		f->Key("riding");
		f->Int(RidingCapacity());

		f->Key("walking");
		f->Int(WalkingCapacity());

		f->Key("swimming");
		f->Int(SwimmingCapacity());
		f->EndObject();

		f->Key("skills");
		f->StartArray();
		skills.ReportJSON(f, GetMen());
		f->EndArray();

		SpoilsReportJSON(f);
	}

	f->Key("flags");
	f->StartArray();
	if (guard == GUARD_GUARD) {
		f->String("on guard");
	}

	if (obs > 0) {
		if (guard == GUARD_AVOID) {
			f->String("avoiding");
		}

		if (GetFlag(FLAG_BEHIND)) {
			f->String("behind");
		}
	}

	if (obs == 2) {
		if (GetFlag(FLAG_HOLDING)) {
			f->String("holding");
		}
		if (GetFlag(FLAG_AUTOTAX)) {
			f->String("taxing");
		}
		if (GetFlag(FLAG_NOAID)) {
			f->String("receiving no aid");
		}
		if (GetFlag(FLAG_SHARING)) {
			f->String("sharing");
		}
		if (GetFlag(FLAG_NOCROSS_WATER)) {
			f->String("won't cross water");
		}
	}
	f->EndArray();

	f->Key("items");
	items.ReportJSON(f->writer, obs, truesight, 0);

	if (obs == 2 && (type == U_MAGE || type == U_GUARDMAGE)) {
		MageReportJSON(f);
	}

	if (obs == 2) {
		ReadyItemJSON(f);
		StudyableSkillsJSON(f);
	}

	f->EndObject();
}

void Unit::BattleReportJSON(Writer<StringBuffer> *jsonWriter, int obs)
{
	jsonWriter->StartObject();

	AString *temp = new AString("");
	if (Globals->BATTLE_FACTION_INFO)
	{
		GetNameJSON(jsonWriter, obs);
		//		jsonWriter->String(GetNameJSON(obs).Str ());
		//		*temp += GetName(obs);
	}
	else
	{
		jsonWriter->Key("name");
		jsonWriter->String((*name).Str());
		//		*temp += *name;
	}

	if (GetFlag(FLAG_BEHIND))
	{
		jsonWriter->Key("behind");
		jsonWriter->Bool(true);
	}

	jsonWriter->Key("items");
	jsonWriter->StartArray();

	items.BattleReportJSON(jsonWriter);

	jsonWriter->EndArray();

	jsonWriter->Key("skills");
	jsonWriter->StartArray();
	forlist(&skills) {
		Skill *s = (Skill *)elem;
		if (SkillDefs[s->type].flags & SkillType::BATTLEREP) {
			int lvl = GetAvailSkill(s->type);
			if (lvl) {
				//				*temp += ", ";
				//				*temp += SkillDefs[s->type].name;
				//				*temp += " ";
				//				*temp += lvl;

				jsonWriter->StartObject();
				jsonWriter->Key(SkillDefs[s->type].name);
				jsonWriter->Int(lvl);
				jsonWriter->EndObject();
			}
		}
	}
	jsonWriter->EndArray();

	if (describe) {
		jsonWriter->Key("descrition");
		jsonWriter->String((*describe).Str());

		//		*temp += "; ";
		//		*temp += *describe;
	}

	jsonWriter->EndObject();
}

void WriteSkillPropsToJSON(AreportJSON *f, int i)
{
	f->Key("name");
	f->String(SkillDefs[i].name);

	f->Key("abbr");
	f->String(SkillDefs[i].abbr);
}

void SkillList::ReportJSON(AreportJSON *f, int nummen)
{
	AString temp;
	if (!Num()) {
		//		temp += "none";
		//		return temp;
		return;
	}
	int i = 0;
	int displayed = 0;
	forlist(this) {
		Skill *s = (Skill *)elem;
		if (s->days == 0) continue;
		displayed++;
		if (i) {
			temp += ", ";
		}
		else {
			i = 1;
		}
		f->StartObject();
		WriteSkillPropsToJSON(f, s->type);
		//		temp += SkillStrs(s->type);

		f->Key("level");
		f->Int(GetLevelByDays(s->days / nummen));

		f->Key("days");
		f->Int(s->days / nummen);

		//		temp += AString(" ") + GetLevelByDays(s->days / nummen) +
		//			AString(" (") + AString(s->days / nummen);
		if (Globals->REQUIRED_EXPERIENCE) {
			//			temp += AString("+") + AString(GetStudyRate(s->type, nummen));
			f->Key("study");
			f->Int(GetStudyRate(s->type, nummen));
		}
		//		temp += AString(")");
		f->EndObject();
	}
	//if (!displayed) temp += "none";
}

AreportJSON::AreportJSON()
{
	writer = new Writer<StringBuffer>(s);
}

AreportJSON::~AreportJSON()
{
	delete writer;
}

void AreportJSON::Close()
{
	json.PutStr(s.GetString());
	json.Close();
}

void AreportJSON::Open(const AString &s)
{
	AString *name = getfilename(s);
	int i = json.OpenByName(name->Str());

	if (i != -1)
	{
	}
}

int AreportJSON::OpenByName(const AString &s)
{
	AString name = s;
	int i = json.OpenByName(name.Str());

	if (i != -1)
	{
	}

	return 0;
}

void AreportJSON::StartObject()
{
	writer->StartObject();
}

void AreportJSON::EndObject()
{
	writer->EndObject();
}

void AreportJSON::StartArray()
{
	writer->StartArray();
}

void AreportJSON::EndArray()
{
	writer->EndArray();
}

void AreportJSON::Key(const AString &s)
{
	AString temp = s;
	writer->Key(temp.Str());
}

void AreportJSON::String(const AString &s)
{
	AString temp = s;
	writer->String(temp.Str());
}

void AreportJSON::Bool(const bool &b)
{
	writer->Bool(b);
}

void AreportJSON::Int(const int &i)
{
	writer->Int(i);
}

void AreportJSON::Double(const double &i)
{
	writer->Double(i);
}

void AreportJSON::Null()
{
	writer->Null();
}

#endif
