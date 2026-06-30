#include "MainMenuState.h"
#include <sstream>
#include "../version.h"
#include "../Engine/Game.h"
#include "../Mod/Mod.h"
#include "../Engine/LocalizedText.h"
#include "../Engine/Logger.h"
#include "../Engine/Screen.h"
#include "../Interface/TextButton.h"
#include "../Interface/Window.h"
#include "../Interface/Text.h"
#include "NewGameState.h"
#include "NewBattleState.h"
#include "ListLoadState.h"
#include "OptionsVideoState.h"
#include "ModListState.h"
#include "../Engine/Options.h"

namespace OpenXcom
{

GoToMainMenuState::GoToMainMenuState() {}
GoToMainMenuState::~GoToMainMenuState() {}

void GoToMainMenuState::init()
{
	Screen::updateScale(Options::geoscapeScale, Options::baseXGeoscape, Options::baseYGeoscape, true);
	_game->getScreen()->resetDisplay(false);
	_game->setState(new MainMenuState());
}

MainMenuState::MainMenuState()
{
	_window = new Window(this, 256, 160, 32, 20, POPUP_BOTH);
	_btnNewGame = new TextButton(92, 20, 64, 90);
	_btnNewBattle = new TextButton(92, 20, 164, 90);
	_btnLoad = new TextButton(92, 20, 64, 118);
	_btnOptions = new TextButton(92, 20, 164, 118);
	_btnMods = new TextButton(92, 20, 64, 146);
	_btnQuit = new TextButton(92, 20, 164, 146);
	_txtUpdateInfo = new Text(320, 17, 0, 11);
	_txtTitle = new Text(256, 30, 32, 45);

	setInterface("mainMenu");

	add(_window, "window", "mainMenu");
	add(_btnNewGame, "button", "mainMenu");
	add(_btnNewBattle, "button", "mainMenu");
	add(_btnLoad, "button", "mainMenu");
	add(_btnOptions, "button", "mainMenu");
	add(_btnMods, "button", "mainMenu");
	add(_btnQuit, "button", "mainMenu");
	add(_txtUpdateInfo, "text", "mainMenu");
	add(_txtTitle, "text", "mainMenu");

	centerAllSurfaces();
	setWindowBackground(_window, "mainMenu");

	_btnNewGame->setText(tr("STR_NEW_GAME"));
	_btnNewGame->onMouseClick((ActionHandler)&MainMenuState::btnNewGameClick);
	_btnNewBattle->setText(tr("STR_NEW_BATTLE"));
	_btnNewBattle->onMouseClick((ActionHandler)&MainMenuState::btnNewBattleClick);
	_btnLoad->setText(tr("STR_LOAD_SAVED_GAME"));
	_btnLoad->onMouseClick((ActionHandler)&MainMenuState::btnLoadClick);
	_btnOptions->setText(tr("STR_OPTIONS"));
	_btnOptions->onMouseClick((ActionHandler)&MainMenuState::btnOptionsClick);
	_btnMods->setText(tr("STR_MODS"));
	_btnMods->onMouseClick((ActionHandler)&MainMenuState::btnModsClick);
	_btnQuit->setText(tr("STR_QUIT"));
	_btnQuit->onMouseClick((ActionHandler)&MainMenuState::btnQuitClick);

	_txtUpdateInfo->setAlign(ALIGN_CENTER);
	_txtUpdateInfo->setWordWrap(true);
	_txtUpdateInfo->setVisible(false);
	if (Options::isModifiedContentLoaded())
	{
		_txtUpdateInfo->setText(tr("STR_FTA_MODIFIED_GAME_ACHIEVEMENTS_DISABLED"));
		_txtUpdateInfo->setColor(81);
		_txtUpdateInfo->setVisible(true);
	}

	_txtTitle->setAlign(ALIGN_CENTER);
	_txtTitle->setBig();
	std::ostringstream title;
	title << tr("STR_OPENXCOM").arg(Options::getActiveMasterInfo()->getVersionDisplay()) << Unicode::TOK_NL_SMALL;
	title << "OpenXcom " << OPENXCOM_FTA_VERSION_SHORT << OPENXCOM_FTA_VERSION_GIT;
	_txtTitle->setText(title.str());
}

void MainMenuState::init()
{
	State::init();
	_game->setModifiedContentLoaded(Options::isModifiedContentLoaded());
	if (Options::getLoadLastSave() && !Options::getLoadThisSave().empty())
	{
		Log(LOG_INFO) << "Loading saved game passed as parameter";
		btnLoadClick(NULL);
	}
	else if (Options::getLoadLastSave() && _game->getSavedGame()->getList(_game->getLanguage(), true).size() > 0)
	{
		Log(LOG_INFO) << "Loading last saved game";
		btnLoadClick(NULL);
	}
}

MainMenuState::~MainMenuState() {}

void MainMenuState::btnNewGameClick(Action *) { _game->pushState(new NewGameState); }
void MainMenuState::btnNewBattleClick(Action *) { _game->pushState(new NewBattleState); }
void MainMenuState::btnLoadClick(Action *) { _game->pushState(new ListLoadState(OPT_MENU)); }
void MainMenuState::btnOptionsClick(Action *)
{
	Options::backupDisplay();
	_game->pushState(new OptionsVideoState(OPT_MENU));
}
void MainMenuState::btnModsClick(Action *) { _game->pushState(new ModListState); }
void MainMenuState::btnQuitClick(Action *) { _game->quit(); }

void MainMenuState::resize(int &dX, int &dY)
{
	dX = Options::baseXResolution;
	dY = Options::baseYResolution;
	Screen::updateScale(Options::geoscapeScale, Options::baseXGeoscape, Options::baseYGeoscape, true);
	dX = Options::baseXResolution - dX;
	dY = Options::baseYResolution - dY;
	State::resize(dX, dY);
}

}
