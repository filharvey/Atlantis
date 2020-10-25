// START A3HEADER
//
// This source file is part of the Atlantis PBM game program.
// Copyright (C) 1995-1999 Geoff Dunbar
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program, in the file license.txt. If not, write
// to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
// Boston, MA 02111-1307, USA.
//
// See the Atlantis Project web page for details:
// http://www.prankster.com/project
//
// END A3HEADER
// MODIFICATIONS
// Date        Person          Comments
// ----        ------          --------
// 2000/MAR/14 Larry Stanbery  Added a unit:faction map capability.
#include "gamedefs.h"
#include "game.h"
#include "items.h"
#include "skills.h"
#include "gamedata.h"

void usage()
{
	Awrite("atlantis new");
	Awrite("atlantis run");
	Awrite("atlantis edit");
	Awrite("");
	Awrite("atlantis map <geo|wmon|lair|gate> <mapfile>");
	Awrite("atlantis mapunits");
	Awrite("atlantis genrules <introfile> <cssfile> <rules-outputfile>");
	Awrite("");
	Awrite("atlantis check <orderfile> <checkfile>");
	Awrite("");
	Awrite("atlantis battle <jsonfile> <count>");
}

int main(int argc, char *argv[])
{
	Game game;
	int retval = 1;

	initIO();

	Awrite(AString("Atlantis Engine Version: ") +
			ATL_VER_STRING(CURRENT_ATL_VER));
	Awrite(AString(Globals->RULESET_NAME) + ", Version: " +
			ATL_VER_STRING(Globals->RULESET_VERSION));
	Awrite("");

	if (argc == 1) {
		usage();
		doneIO();
		return 0;
	}

	game.ModifyTablesPerRuleset();

	do {
		if (AString(argv[1]) == "new") {
			if (!game.NewGame()) {
				Awrite( "Couldn't make the new game!" );
				break;
			}

			if ( !game.SaveGame() ) {
				Awrite( "Couldn't save the game!" );
				break;
			}

			if ( !game.WritePlayers() ) {
				Awrite( "Couldn't write the players file!" );
				break;
			}
		} else if (AString(argv[1]) == "map") {
			if (argc != 4) {
				usage();
				break;
			}

			if ( !game.OpenGame() ) {
				Awrite( "Couldn't open the game file!" );
				break;
			}

			if ( !game.ViewMap( argv[2], argv[3] )) {
				Awrite( "Couldn't write the map file!" );
				break;
			}
		} else if (AString(argv[1]) == "run") {
			if ( !game.OpenGame() ) {
				Awrite( "Couldn't open the game file!" );
				break;
			}

			if ( !game.RunGame() ) {
				Awrite( "Couldn't run the game!" );
				break;
			}

			if ( !game.SaveGame() ) {
				Awrite( "Couldn't save the game!" );
				break;
			}
		} else if (AString(argv[1]) == "edit") {
			if ( !game.OpenGame() ) {
				Awrite( "Couldn't open the game file!" );
				break;
			}

			int saveGame = 0;
			if ( !game.EditGame( &saveGame ) ) {
				Awrite( "Couldn't edit the game!" );
				break;
			}

			if ( saveGame ) {
				if ( !game.SaveGame() ) {
					Awrite( "Couldn't save the game!" );
					break;
				}
			}
		} else if ( AString( argv[1] ) == "check" ) {
			if (argc != 4) {
				usage();
				break;
			}

			game.DummyGame();
			if ( !game.DoOrdersCheck( argv[ 2 ], argv[ 3 ] )) {
				Awrite( "Couldn't check the orders!" );
				break;
			}
		} else if ( AString( argv[1] ) == "mapunits" ) {
			if ( !game.OpenGame() ) {
				Awrite( "Couldn't open the game file!" );
				break;
			}
			game.UnitFactionMap();
		} else if (AString(argv[1])== "genrules") {
			if (argc != 5) {
				usage();
				break;
			}
			if (!game.GenRules(argv[4], argv[3], argv[2])) {
				Awrite("Unable to generate rules!");
				break;
			}
		}
#if EXPORT_JSON
		else if (AString(argv[1]) == "battle") {
			if (argc != 3 && argc != 4) {
				usage();
				break;
			}

			int rounds = 1;
			if (argc == 4)
			{
				rounds = atoi(argv[3]);
			}

			int wins = 0;
			int loses = 0;

			seedrandomrandom();

			for (int a = 0; a < rounds; a++)
			{
				int result = game.SimulateBattle(argv[2]);

				if (result == BATTLE_WON) 
					wins++;
				else if (result == BATTLE_LOST)
					loses++;
			}

			float ratio = (wins / (float)rounds) * 100;

			cout << "Battles " << rounds << " wins " << wins << " loses " << loses << " ratio: " << ratio << "%" << endl;
		}
		else if (AString(argv[1]) == "exportdata") {

			if (!game.ExportGameData()) {
				Awrite("Unable to generate battle report!");
				break;
			}
		}
#endif
		else {
			Awrite(AString("Unknown option: ") + argv[1]);
			break;
		}
		retval = 0;
	} while( 0 );

	doneIO();
	return retval;
}
