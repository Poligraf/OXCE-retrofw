/*
 * Copyright 2010-2016 OpenXcom Developers.
 *
 * This file is part of OpenXcom.
 *
 * OpenXcom is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * OpenXcom is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with OpenXcom.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "GeoscapeState.h"
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <climits>
#include <functional>
#include "../Engine/RNG.h"
#include "../Engine/Game.h"
#include "../Engine/Action.h"
#include "../Mod/Mod.h"
#include "../Engine/LocalizedText.h"
#include "../Engine/Screen.h"
#include "../Engine/Sound.h"
#include "../Engine/Surface.h"
#include "../Engine/Options.h"
#include "../Engine/Collections.h"
#include "../Engine/Unicode.h"
#include "Globe.h"
#include "../Interface/ComboBox.h"
#include "../Interface/Text.h"
#include "../Interface/TextButton.h"
#include "../Engine/Timer.h"
#include "../Savegame/GameTime.h"
#include "../Savegame/SavedGame.h"
#include "../Savegame/Base.h"
#include "../Savegame/BaseFacility.h"
#include "../Mod/RuleBaseFacility.h"
#include "../Savegame/Craft.h"
#include "../Mod/RuleCraft.h"
#include "../Savegame/Ufo.h"
#include "../Mod/RuleUfo.h"
#include "../Mod/RuleArcScript.h"
#include "../Mod/RuleEventScript.h"
#include "../Mod/RuleEvent.h"
#include "../Mod/RuleMissionScript.h"
#include "../Savegame/Waypoint.h"
#include "../Savegame/Transfer.h"
#include "../Savegame/Soldier.h"
#include "../Savegame/SoldierDiary.h"
#include "../Menu/PauseState.h"
#include "SelectMusicTrackState.h"
#include "UfoTrackerState.h"
#include "InterceptState.h"
#include "../Basescape/BasescapeState.h"
#include "../Basescape/SellState.h"
#include "../Basescape/ManageAlienContainmentState.h"
#include "../Basescape/TechTreeViewerState.h"
#include "../Basescape/GlobalManufactureState.h"
#include "../Basescape/GlobalResearchState.h"
#include "../Menu/CutsceneState.h"
#include "../Menu/ErrorMessageState.h"
#include "GraphsState.h"
#include "FundingState.h"
#include "MonthlyReportState.h"
#include "ProductionCompleteState.h"
#include "UfoDetectedState.h"
#include "GeoscapeCraftState.h"
#include "DogfightState.h"
#include "UfoLostState.h"
#include "CraftPatrolState.h"
#include "LowFuelState.h"
#include "MultipleTargetsState.h"
#include "ConfirmLandingState.h"
#include "ItemsArrivingState.h"
#include "CraftErrorState.h"
#include "DogfightErrorState.h"
#include "DogfightExperienceState.h"
#include "../Ufopaedia/Ufopaedia.h"
#include "../Savegame/ResearchProject.h"
#include "ResearchCompleteState.h"
#include "../Mod/RuleResearch.h"
#include "ResearchRequiredState.h"
#include "NewPossibleResearchState.h"
#include "NewPossibleManufactureState.h"
#include "NewPossiblePurchaseState.h"
#include "NewPossibleCraftState.h"
#include "NewPossibleFacilityState.h"
#include "TrainingFinishedState.h"
#include "../Savegame/Production.h"
#include "../Mod/RuleManufacture.h"
#include "../Savegame/ItemContainer.h"
#include "../Savegame/MissionSite.h"
#include "../Savegame/AlienBase.h"
#include "../Mod/RuleRegion.h"
#include "MissionDetectedState.h"
#include "AlienBaseState.h"
#include "../Savegame/Region.h"
#include "../Savegame/Country.h"
#include "../Mod/RuleCountry.h"
#include "../Mod/RuleAlienMission.h"
#include "../Savegame/AlienStrategy.h"
#include "../Savegame/AlienMission.h"
#include "../Savegame/GeoscapeEvent.h"
#include "GeoscapeEventState.h"
#include "../Savegame/SavedBattleGame.h"
#include "../Battlescape/BattlescapeGenerator.h"
#include "../Battlescape/BriefingState.h"
#include "../Mod/UfoTrajectory.h"
#include "../Mod/Armor.h"
#include "BaseDefenseState.h"
#include "BaseDestroyedState.h"
#include "../Menu/LoadGameState.h"
#include "../Menu/SaveGameState.h"
#include "../Menu/ListSaveState.h"
#include "../Mod/RuleGlobe.h"
#include "../Engine/Exception.h"
#include "../Mod/AlienDeployment.h"
#include "../Mod/RuleInterface.h"
#include "../Mod/RuleVideo.h"
#include "../fmath.h"
#include "../fallthrough.h"

namespace OpenXcom
{

/**
 * Initializes all the elements in the Geoscape screen.
 * @param game Pointer to the core game.
 */
GeoscapeState::GeoscapeState() : _pause(false), _zoomInEffectDone(false), _zoomOutEffectDone(false), _minimizedDogfights(0), _slowdownCounter(0)
{
	int screenWidth = Options::baseXGeoscape;
	int screenHeight = Options::baseYGeoscape;

	// Create objects
	Surface *hd = _game->getMod()->getSurface("ALTGEOBORD.SCR");
	_bg = new Surface(hd->getWidth(), hd->getHeight(), 0, 0);
	_sideLine = new Surface(64, screenHeight, screenWidth - 64, 0);
	_sidebar = new Surface(64, 200, screenWidth - 64, screenHeight / 2 - 100);

	_globe = new Globe(_game, (screenWidth-64)/2, screenHeight/2, screenWidth-64, screenHeight, 0, 0);
	_bg->setX((_globe->getWidth() - _bg->getWidth()) / 2);
	_bg->setY((_globe->getHeight() - _bg->getHeight()) / 2);

	_btnIntercept = new TextButton(63, 11, screenWidth-63, screenHeight/2-100);
	_btnBases = new TextButton(63, 11, screenWidth-63, screenHeight/2-88);
	_btnGraphs = new TextButton(63, 11, screenWidth-63, screenHeight/2-76);
	_btnUfopaedia = new TextButton(63, 11, screenWidth-63, screenHeight/2-64);
	_btnOptions = new TextButton(63, 11, screenWidth-63, screenHeight/2-52);
	_btnFunding = new TextButton(63, 11, screenWidth-63, screenHeight/2-40);

	_btn5Secs = new TextButton(31, 13, screenWidth-63, screenHeight/2+12);
	_btn1Min = new TextButton(31, 13, screenWidth-31, screenHeight/2+12);
	_btn5Mins = new TextButton(31, 13, screenWidth-63, screenHeight/2+26);
	_btn30Mins = new TextButton(31, 13, screenWidth-31, screenHeight/2+26);
	_btn1Hour = new TextButton(31, 13, screenWidth-63, screenHeight/2+40);
	_btn1Day = new TextButton(31, 13, screenWidth-31, screenHeight/2+40);

	_btnRotateLeft = new InteractiveSurface(12, 10, screenWidth-61, screenHeight/2+76);
	_btnRotateRight = new InteractiveSurface(12, 10, screenWidth-37, screenHeight/2+76);
	_btnRotateUp = new InteractiveSurface(13, 12, screenWidth-49, screenHeight/2+62);
	_btnRotateDown = new InteractiveSurface(13, 12, screenWidth-49, screenHeight/2+87);
	_btnZoomIn = new InteractiveSurface(23, 23, screenWidth-25, screenHeight/2+56);
	_btnZoomOut = new InteractiveSurface(13, 17, screenWidth-20, screenHeight/2+82);

	int height = (screenHeight - Screen::ORIGINAL_HEIGHT) / 2 + 10;
	_sideTop = new TextButton(63, height, screenWidth-63, _sidebar->getY() - height - 1);
	_sideBottom = new TextButton(63, height, screenWidth-63, _sidebar->getY() + _sidebar->getHeight() + 1);

	_txtHour = new Text(20, 16, screenWidth-61, screenHeight/2-26);
	_txtHourSep = new Text(4, 16, screenWidth-41, screenHeight/2-26);
	_txtMin = new Text(20, 16, screenWidth-37, screenHeight/2-26);
	_txtMinSep = new Text(4, 16, screenWidth-17, screenHeight/2-26);
	_txtSec = new Text(11, 8, screenWidth-13, screenHeight/2-20);
	_txtWeekday = new Text(59, 8, screenWidth-61, screenHeight/2-13);
	_txtDay = new Text(29, 8, screenWidth-61, screenHeight/2-6);
	_txtMonth = new Text(29, 8, screenWidth-32, screenHeight/2-6);
	_txtYear = new Text(59, 8, screenWidth-61, screenHeight/2+1);
	_txtFunds = new Text(59, 8, screenWidth-61, screenHeight/2-27);

	int slackingIndicatorOffset = _game->getMod()->getInterface("geoscape")->getElement("slackingIndicator")->custom;
	_txtSlacking = new Text(59, 17, screenWidth - 61, screenHeight / 2 - 100 + slackingIndicatorOffset);

	_timeSpeed = _btn5Secs;
	_gameTimer = new Timer(Options::geoClockSpeed);

	_zoomInEffectTimer = new Timer(Options::dogfightSpeed);
	_zoomOutEffectTimer = new Timer(Options::dogfightSpeed);
	_dogfightStartTimer = new Timer(Options::dogfightSpeed);
	_dogfightTimer = new Timer(Options::dogfightSpeed);

	_txtDebug = new Text(200, 32, 0, 0);
	_cbxRegion = new ComboBox(this, 150, 16, 0, 36);
	_cbxZone = new ComboBox(this, 100, 16, 154, 36);

	// Set palette
	setInterface("geoscape");

	add(_bg);
	add(_sideLine);
	add(_sidebar);
	add(_globe);

	add(_btnIntercept, "button", "geoscape");
	add(_btnBases, "button", "geoscape");
	add(_btnGraphs, "button", "geoscape");
	add(_btnUfopaedia, "button", "geoscape");
	add(_btnOptions, "button", "geoscape");
	add(_btnFunding, "button", "geoscape");

	add(_btn5Secs, "button", "geoscape");
	add(_btn1Min, "button", "geoscape");
	add(_btn5Mins, "button", "geoscape");
	add(_btn30Mins, "button", "geoscape");
	add(_btn1Hour, "button", "geoscape");
	add(_btn1Day, "button", "geoscape");

	add(_btnRotateLeft);
	add(_btnRotateRight);
	add(_btnRotateUp);
	add(_btnRotateDown);
	add(_btnZoomIn);
	add(_btnZoomOut);

	add(_sideTop, "button", "geoscape");
	add(_sideBottom, "button", "geoscape");

	add(_txtFunds, "text", "geoscape");
	add(_txtHour, "text", "geoscape");
	add(_txtHourSep, "text", "geoscape");
	add(_txtMin, "text", "geoscape");
	add(_txtMinSep, "text", "geoscape");
	add(_txtSec, "text", "geoscape");
	add(_txtWeekday, "text", "geoscape");
	add(_txtDay, "text", "geoscape");
	add(_txtMonth, "text", "geoscape");
	add(_txtYear, "text", "geoscape");
	add(_txtSlacking, "slackingIndicator", "geoscape");

	add(_txtDebug, "text", "geoscape");
	add(_cbxRegion, "button", "geoscape");
	add(_cbxZone, "button", "geoscape");

	// Set up objects
	Surface *geobord = _game->getMod()->getSurface("GEOBORD.SCR");
	geobord->setX(_sidebar->getX() - geobord->getWidth() + _sidebar->getWidth());
	geobord->setY(_sidebar->getY());
	_sidebar->copy(geobord);
	_game->getMod()->getSurface("ALTGEOBORD.SCR")->blitNShade(_bg, 0, 0);

	_sideLine->drawRect(0, 0, _sideLine->getWidth(), _sideLine->getHeight(), 15);

	_btnIntercept->initText(_game->getMod()->getFont("FONT_GEO_BIG"), _game->getMod()->getFont("FONT_GEO_SMALL"), _game->getLanguage());
	_btnIntercept->setText(tr("STR_INTERCEPT"));
	_btnIntercept->onMouseClick((ActionHandler)&GeoscapeState::btnInterceptClick);
	_btnIntercept->onKeyboardPress((ActionHandler)&GeoscapeState::btnInterceptClick, Options::keyGeoIntercept);
	_btnIntercept->onKeyboardPress((ActionHandler)&GeoscapeState::btnUfoTrackerClick, Options::keyGeoUfoTracker);
	_btnIntercept->onKeyboardPress((ActionHandler)&GeoscapeState::btnTechTreeViewerClick, Options::keyGeoTechTreeViewer);
	_btnIntercept->onKeyboardPress((ActionHandler)&GeoscapeState::btnSelectMusicTrackClick, Options::keySelectMusicTrack);
	_btnIntercept->onKeyboardPress((ActionHandler)&GeoscapeState::btnGlobalProductionClick, Options::keyGeoGlobalProduction);
	_btnIntercept->onKeyboardPress((ActionHandler)&GeoscapeState::btnGlobalResearchClick, Options::keyGeoGlobalResearch);
	_btnIntercept->onKeyboardPress((ActionHandler)&GeoscapeState::btnDogfightExperienceClick, Options::keyGeoDailyPilotExperience);
	_btnIntercept->setGeoscapeButton(true);

	_btnBases->initText(_game->getMod()->getFont("FONT_GEO_BIG"), _game->getMod()->getFont("FONT_GEO_SMALL"), _game->getLanguage());
	_btnBases->setText(tr("STR_BASES"));
	_btnBases->onMouseClick((ActionHandler)&GeoscapeState::btnBasesClick);
	_btnBases->onKeyboardPress((ActionHandler)&GeoscapeState::btnBasesClick, Options::keyGeoBases);
	_btnBases->setGeoscapeButton(true);

	_btnGraphs->initText(_game->getMod()->getFont("FONT_GEO_BIG"), _game->getMod()->getFont("FONT_GEO_SMALL"), _game->getLanguage());
	_btnGraphs->setText(tr("STR_GRAPHS"));
	_btnGraphs->onMouseClick((ActionHandler)&GeoscapeState::btnGraphsClick);
	_btnGraphs->onKeyboardPress((ActionHandler)&GeoscapeState::btnGraphsClick, Options::keyGeoGraphs);
	_btnGraphs->setGeoscapeButton(true);

	_btnUfopaedia->initText(_game->getMod()->getFont("FONT_GEO_BIG"), _game->getMod()->getFont("FONT_GEO_SMALL"), _game->getLanguage());
	_btnUfopaedia->setText(tr("STR_UFOPAEDIA_UC"));
	_btnUfopaedia->onMouseClick((ActionHandler)&GeoscapeState::btnUfopaediaClick);
	_btnUfopaedia->onKeyboardPress((ActionHandler)&GeoscapeState::btnUfopaediaClick, Options::keyGeoUfopedia);
	_btnUfopaedia->setGeoscapeButton(true);

	_btnOptions->initText(_game->getMod()->getFont("FONT_GEO_BIG"), _game->getMod()->getFont("FONT_GEO_SMALL"), _game->getLanguage());
	_btnOptions->setText(tr("STR_OPTIONS_UC"));
	_btnOptions->onMouseClick((ActionHandler)&GeoscapeState::btnOptionsClick);
	_btnOptions->onKeyboardPress((ActionHandler)&GeoscapeState::btnOptionsClick, Options::keyGeoOptions);
	_btnOptions->setGeoscapeButton(true);

	_btnFunding->initText(_game->getMod()->getFont("FONT_GEO_BIG"), _game->getMod()->getFont("FONT_GEO_SMALL"), _game->getLanguage());
	_btnFunding->setText(tr("STR_FUNDING_UC"));
	_btnFunding->onMouseClick((ActionHandler)&GeoscapeState::btnFundingClick);
	_btnFunding->onKeyboardPress((ActionHandler)&GeoscapeState::btnFundingClick, Options::keyGeoFunding);
	_btnFunding->setGeoscapeButton(true);

	_btn5Secs->initText(_game->getMod()->getFont("FONT_GEO_BIG"), _game->getMod()->getFont("FONT_GEO_SMALL"), _game->getLanguage());
	_btn5Secs->setBig();
	_btn5Secs->setText(tr("STR_5_SECONDS"));
	_btn5Secs->setGroup(&_timeSpeed);
	_btn5Secs->onKeyboardPress((ActionHandler)&GeoscapeState::btnTimerClick, Options::keyGeoSpeed1);
	_btn5Secs->setGeoscapeButton(true);

	_btn1Min->initText(_game->getMod()->getFont("FONT_GEO_BIG"), _game->getMod()->getFont("FONT_GEO_SMALL"), _game->getLanguage());
	_btn1Min->setBig();
	_btn1Min->setText(tr("STR_1_MINUTE"));
	_btn1Min->setGroup(&_timeSpeed);
	_btn1Min->onKeyboardPress((ActionHandler)&GeoscapeState::btnTimerClick, Options::keyGeoSpeed2);
	_btn1Min->setGeoscapeButton(true);

	_btn5Mins->initText(_game->getMod()->getFont("FONT_GEO_BIG"), _game->getMod()->getFont("FONT_GEO_SMALL"), _game->getLanguage());
	_btn5Mins->setBig();
	_btn5Mins->setText(tr("STR_5_MINUTES"));
	_btn5Mins->setGroup(&_timeSpeed);
	_btn5Mins->onKeyboardPress((ActionHandler)&GeoscapeState::btnTimerClick, Options::keyGeoSpeed3);
	_btn5Mins->setGeoscapeButton(true);

	_btn30Mins->initText(_game->getMod()->getFont("FONT_GEO_BIG"), _game->getMod()->getFont("FONT_GEO_SMALL"), _game->getLanguage());
	_btn30Mins->setBig();
	_btn30Mins->setText(tr("STR_30_MINUTES"));
	_btn30Mins->setGroup(&_timeSpeed);
	_btn30Mins->onKeyboardPress((ActionHandler)&GeoscapeState::btnTimerClick, Options::keyGeoSpeed4);
	_btn30Mins->setGeoscapeButton(true);

	_btn1Hour->initText(_game->getMod()->getFont("FONT_GEO_BIG"), _game->getMod()->getFont("FONT_GEO_SMALL"), _game->getLanguage());
	_btn1Hour->setBig();
	_btn1Hour->setText(tr("STR_1_HOUR"));
	_btn1Hour->setGroup(&_timeSpeed);
	_btn1Hour->onKeyboardPress((ActionHandler)&GeoscapeState::btnTimerClick, Options::keyGeoSpeed5);
	_btn1Hour->setGeoscapeButton(true);

	_btn1Day->initText(_game->getMod()->getFont("FONT_GEO_BIG"), _game->getMod()->getFont("FONT_GEO_SMALL"), _game->getLanguage());
	_btn1Day->setBig();
	_btn1Day->setText(tr("STR_1_DAY"));
	_btn1Day->setGroup(&_timeSpeed);
	_btn1Day->onKeyboardPress((ActionHandler)&GeoscapeState::btnTimerClick, Options::keyGeoSpeed6);
	_btn1Day->setGeoscapeButton(true);

	_sideBottom->setGeoscapeButton(true);
	_sideTop->setGeoscapeButton(true);

	_btnRotateLeft->onMousePress((ActionHandler)&GeoscapeState::btnRotateLeftPress);
	_btnRotateLeft->onMouseRelease((ActionHandler)&GeoscapeState::btnRotateLeftRelease);
	_btnRotateLeft->onKeyboardPress((ActionHandler)&GeoscapeState::btnRotateLeftPress, Options::keyGeoLeft);
	_btnRotateLeft->onKeyboardRelease((ActionHandler)&GeoscapeState::btnRotateLeftRelease, Options::keyGeoLeft);

	_btnRotateRight->onMousePress((ActionHandler)&GeoscapeState::btnRotateRightPress);
	_btnRotateRight->onMouseRelease((ActionHandler)&GeoscapeState::btnRotateRightRelease);
	_btnRotateRight->onKeyboardPress((ActionHandler)&GeoscapeState::btnRotateRightPress, Options::keyGeoRight);
	_btnRotateRight->onKeyboardRelease((ActionHandler)&GeoscapeState::btnRotateRightRelease, Options::keyGeoRight);

	_btnRotateUp->onMousePress((ActionHandler)&GeoscapeState::btnRotateUpPress);
	_btnRotateUp->onMouseRelease((ActionHandler)&GeoscapeState::btnRotateUpRelease);
	_btnRotateUp->onKeyboardPress((ActionHandler)&GeoscapeState::btnRotateUpPress, Options::keyGeoUp);
	_btnRotateUp->onKeyboardRelease((ActionHandler)&GeoscapeState::btnRotateUpRelease, Options::keyGeoUp);

	_btnRotateDown->onMousePress((ActionHandler)&GeoscapeState::btnRotateDownPress);
	_btnRotateDown->onMouseRelease((ActionHandler)&GeoscapeState::btnRotateDownRelease);
	_btnRotateDown->onKeyboardPress((ActionHandler)&GeoscapeState::btnRotateDownPress, Options::keyGeoDown);
	_btnRotateDown->onKeyboardRelease((ActionHandler)&GeoscapeState::btnRotateDownRelease, Options::keyGeoDown);

	_btnZoomIn->onMouseClick((ActionHandler)&GeoscapeState::btnZoomInLeftClick, SDL_BUTTON_LEFT);
	_btnZoomIn->onMouseClick((ActionHandler)&GeoscapeState::btnZoomInRightClick, SDL_BUTTON_RIGHT);
	_btnZoomIn->onKeyboardPress((ActionHandler)&GeoscapeState::btnZoomInLeftClick, Options::keyGeoZoomIn);

	_btnZoomOut->onMouseClick((ActionHandler)&GeoscapeState::btnZoomOutLeftClick, SDL_BUTTON_LEFT);
	_btnZoomOut->onMouseClick((ActionHandler)&GeoscapeState::btnZoomOutRightClick, SDL_BUTTON_RIGHT);
	_btnZoomOut->onKeyboardPress((ActionHandler)&GeoscapeState::btnZoomOutLeftClick, Options::keyGeoZoomOut);

	_txtFunds->setAlign(ALIGN_CENTER);
	_txtFunds->setVisible(Options::showFundsOnGeoscape);

	_txtHour->setBig();
	_txtHour->setAlign(ALIGN_RIGHT);

	_txtHourSep->setBig();
	_txtHourSep->setText(":");

	_txtMin->setBig();

	_txtMinSep->setBig();
	_txtMinSep->setText(":");

	_txtWeekday->setAlign(ALIGN_CENTER);

	_txtDay->setAlign(ALIGN_CENTER);

	_txtMonth->setAlign(ALIGN_CENTER);

	_txtYear->setAlign(ALIGN_CENTER);

	_txtSlacking->setAlign(ALIGN_RIGHT);

	if (Options::showFundsOnGeoscape)
	{
		_txtHour->setY(_txtHour->getY()+6);
		_txtHour->setSmall();
		_txtHourSep->setY(_txtHourSep->getY()+6);
		_txtHourSep->setSmall();
		_txtMin->setY(_txtMin->getY()+6);
		_txtMin->setSmall();
		_txtMinSep->setX(_txtMinSep->getX()-10);
		_txtMinSep->setY(_txtMinSep->getY()+6);
		_txtMinSep->setSmall();
		_txtSec->setX(_txtSec->getX()-10);
	}

	_gameTimer->onTimer((StateHandler)&GeoscapeState::timeAdvance);
	_gameTimer->start();

	_zoomInEffectTimer->onTimer((StateHandler)&GeoscapeState::zoomInEffect);
	_zoomOutEffectTimer->onTimer((StateHandler)&GeoscapeState::zoomOutEffect);
	_dogfightStartTimer->onTimer((StateHandler)&GeoscapeState::startDogfight);
	_dogfightTimer->onTimer((StateHandler)&GeoscapeState::handleDogfights);

	// debug helpers
	{
		std::vector<std::string> regionList;
		regionList.push_back("All regions");
		for (auto r : *_game->getSavedGame()->getRegions())
		{
			regionList.push_back(r->getRules()->getType());
		}
		_cbxRegion->setOptions(regionList, false);
		_cbxRegion->setVisible(false);
		_cbxRegion->onChange((ActionHandler)&GeoscapeState::cbxRegionChange);

		std::vector<std::string> zoneList;
		zoneList.push_back("All zones");
		for (int z = 0; z < 20; ++z)
		{
			zoneList.push_back(std::to_string(z));
		}
		_cbxZone->setOptions(zoneList, false);
		_cbxZone->setVisible(false);
		_cbxZone->onChange((ActionHandler)&GeoscapeState::cbxZoneChange);
	}

	timeDisplay();
}

/**
 * Deletes timers.
 */
GeoscapeState::~GeoscapeState()
{
	delete _gameTimer;
	delete _zoomInEffectTimer;
	delete _zoomOutEffectTimer;
	delete _dogfightStartTimer;
	delete _dogfightTimer;

	std::list<DogfightState*>::iterator it = _dogfights.begin();
	for (; it != _dogfights.end();)
	{
		delete *it;
		it = _dogfights.erase(it);
	}
	for (it = _dogfightsToBeStarted.begin(); it != _dogfightsToBeStarted.end();)
	{
		delete *it;
		it = _dogfightsToBeStarted.erase(it);
	}
}

/**
 * Handle blitting of Geoscape and Dogfights.
 */
void GeoscapeState::blit()
{
	State::blit();
	for (std::list<DogfightState*>::iterator it = _dogfights.begin(); it != _dogfights.end(); ++it)
	{
		(*it)->blit();
	}
}

/**
 * Handle key shortcuts.
 * @param action Pointer to an action.
 */
void GeoscapeState::handle(Action *action)
{
	if (_dogfights.size() == _minimizedDogfights)
	{
		State::handle(action);
	}

	if (action->getDetails()->type == SDL_KEYDOWN)
	{
		// "ctrl-d" - enable debug mode
		if (Options::debug && action->getDetails()->key.keysym.sym == SDLK_d && (SDL_GetModState() & KMOD_CTRL) != 0)
		{
			_game->getSavedGame()->setDebugMode();
			if (_game->getSavedGame()->getDebugMode())
			{
				_txtDebug->setText("DEBUG MODE");
			}
			else
			{
				_txtDebug->setText("");
			}
			_cbxRegion->setVisible(_game->getSavedGame()->getDebugMode());
			_cbxZone->setVisible(_game->getSavedGame()->getDebugMode());
		}
		if (Options::debug && _game->getSavedGame()->getDebugMode() && (SDL_GetModState() & KMOD_CTRL) != 0)
		{
			// "ctrl-1"
			if (action->getDetails()->key.keysym.sym == SDLK_1)
			{
				_txtDebug->setText("I'M A BILLIONAIRE! ALMOST...");
				_game->getSavedGame()->setFunds(999999999);
			}
			// "ctrl-2"
			if (action->getDetails()->key.keysym.sym == SDLK_2)
			{
				_txtDebug->setText("ALL FACILITY CONSTRUCTION COMPLETED");
				for (auto& base : *_game->getSavedGame()->getBases())
				{
					for (auto& facility : *base->getFacilities())
					{
						facility->setBuildTime(0);
						facility->setIfHadPreviousFacility(false);
					}
				}
			}
			// "ctrl-3"
			if (action->getDetails()->key.keysym.sym == SDLK_3)
			{
				_txtDebug->setText("+50 SCIENTISTS/ENGINEERS");
				for (auto& base : *_game->getSavedGame()->getBases())
				{
					base->setScientists(base->getScientists() + 50);
					base->setEngineers(base->getEngineers() + 50);
				}
			}
			// "ctrl-4"
			if (action->getDetails()->key.keysym.sym == SDLK_4)
			{
				_txtDebug->setText("+2 ALL ITEMS");
				for (auto& base : *_game->getSavedGame()->getBases())
				{
					for (auto& itemRule : _game->getMod()->getItemsList())
					{
						auto *item = _game->getMod()->getItem(itemRule);
						if (item && item->isRecoverable() && !item->isAlien() && item->getSellCost() > 0)
						{
							base->getStorageItems()->addItem(itemRule, 2);
						}
					}
				}
			}
			// "ctrl-5"
			if (action->getDetails()->key.keysym.sym == SDLK_5)
			{
				_txtDebug->setText("+2 ALL LIVE ALIENS");
				for (auto& base : *_game->getSavedGame()->getBases())
				{
					for (auto& itemRule : _game->getMod()->getItemsList())
					{
						auto *item = _game->getMod()->getItem(itemRule);
						if (item && item->isRecoverable() && item->isAlien() && item->getSellCost() > 0)
						{
							base->getStorageItems()->addItem(itemRule, 2);
						}
					}
				}
			}
			// "ctrl-6"
			if (action->getDetails()->key.keysym.sym == SDLK_6)
			{
				_txtDebug->setText("XCOM/ALIEN ACTIVITY FOR THIS MONTH RESET");
				size_t invertedEntry = _game->getSavedGame()->getFundsList().size() - 1;
				for (auto& region : *_game->getSavedGame()->getRegions())
				{
					region->getActivityXcom().at(invertedEntry) = 0;
					region->getActivityAlien().at(invertedEntry) = 0;
				}
				for (auto& country : *_game->getSavedGame()->getCountries())
				{
					country->getActivityXcom().at(invertedEntry) = 0;
					country->getActivityAlien().at(invertedEntry) = 0;
				}
			}
			// "ctrl-7"
			if (action->getDetails()->key.keysym.sym == SDLK_7)
			{
				_txtDebug->setText("BIG BROTHER SEES ALL");
				for (auto& ufo : *_game->getSavedGame()->getUfos())
				{
					ufo->setDetected(true);
					ufo->setHyperDetected(true);
				}
				for (auto& ab : *_game->getSavedGame()->getAlienBases())
				{
					ab->setDiscovered(true);
				}
			}
			// "ctrl-a"
			if (action->getDetails()->key.keysym.sym == SDLK_a)
			{
				_txtDebug->setText("SOLDIER DIARIES DELETED");
				for (auto& base : *_game->getSavedGame()->getBases())
				{
					for (auto& soldier : *base->getSoldiers())
					{
						soldier->resetDiary();
					}
				}
			}
			// "ctrl-c"
			if (action->getDetails()->key.keysym.sym == SDLK_c)
			{
				_txtDebug->setText("SOLDIER COMMENDATIONS DELETED");
				for (auto& base : *_game->getSavedGame()->getBases())
				{
					for (auto& soldier : *base->getSoldiers())
					{
						for (auto& commendation : *soldier->getDiary()->getSoldierCommendations())
						{
							delete commendation;
						}
						soldier->getDiary()->getSoldierCommendations()->clear();
					}
				}
			}
		}
		// quick save and quick load
		if (!_game->getSavedGame()->isIronman())
		{
			if (action->getDetails()->key.keysym.sym == Options::keyQuickSave)
			{
				popup(new SaveGameState(OPT_GEOSCAPE, SAVE_QUICK, _palette));
			}
			else if (action->getDetails()->key.keysym.sym == Options::keyQuickLoad)
			{
				popup(new LoadGameState(OPT_GEOSCAPE, SAVE_QUICK, _palette));
			}
		}
	}
	if (!_dogfights.empty())
	{
		for (std::list<DogfightState*>::iterator it = _dogfights.begin(); it != _dogfights.end(); ++it)
		{
			(*it)->handle(action);
		}
		_minimizedDogfights = minimizedDogfightsCount();
	}
}

/**
 * Updates the timer display and resets the palette
 * since it's bound to change on other screens.
 */
void GeoscapeState::init()
{
	State::init();
	timeDisplay();
	updateSlackingIndicator();

	_globe->onMouseClick((ActionHandler)&GeoscapeState::globeClick);
	_globe->onMouseOver(0);
	_globe->rotateStop();
	_globe->setFocus(true);
	_globe->draw();

	// Pop up save screen if it's a new ironman game
	if (_game->getSavedGame()->isIronman() && _game->getSavedGame()->getName().empty())
	{
		popup(new ListSaveState(OPT_GEOSCAPE));
	}

	// Set music if it's not already playing
	if (_dogfights.empty() && !_dogfightStartTimer->isRunning())
	{
		if (_game->getSavedGame()->getMonthsPassed() == -1)
		{
			_game->getMod()->playMusic("GMGEO", 1);
		}
		else
		{
			_game->getMod()->playMusic("GMGEO");
		}
	}
	else
	{
		_game->getMod()->playMusic("GMINTER");
	}
	_globe->setNewBaseHover(false);

		// run once
	if (_game->getSavedGame()->getMonthsPassed() == -1 &&
		// as long as there's a base
		!_game->getSavedGame()->getBases()->empty() &&
		// and it has a name (THIS prevents it from running prior to the base being placed.)
		!_game->getSavedGame()->getBases()->front()->getName().empty())
	{
		_game->getSavedGame()->addMonth();
		determineAlienMissions();
		_game->getSavedGame()->setFunds(_game->getSavedGame()->getFunds() - (_game->getSavedGame()->getBaseMaintenance() - _game->getSavedGame()->getBases()->front()->getPersonnelMaintenance()));
	}
}

/**
 * Runs the game timer and handles popups.
 */
void GeoscapeState::think()
{
	State::think();

	_zoomInEffectTimer->think(this, 0);
	_zoomOutEffectTimer->think(this, 0);
	_dogfightStartTimer->think(this, 0);

	if (_popups.empty() && _dogfights.empty() && (!_zoomInEffectTimer->isRunning() || _zoomInEffectDone) && (!_zoomOutEffectTimer->isRunning() || _zoomOutEffectDone))
	{
		// Handle timers
		_gameTimer->think(this, 0);
	}
	else
	{
		if (!_dogfights.empty() || _minimizedDogfights != 0)
		{
			// If all dogfights are minimized rotate the globe, etc.
			if (_dogfights.size() == _minimizedDogfights)
			{
				_pause = false;
				_gameTimer->think(this, 0);
			}
			_dogfightTimer->think(this, 0);
		}
		if (!_popups.empty())
		{
			// Handle popups
			_globe->rotateStop();
			_game->pushState(_popups.front());
			_popups.erase(_popups.begin());
		}
	}
}

/**
 * Updates the Geoscape clock with the latest
 * game time and date in human-readable format. (+Funds)
 */
void GeoscapeState::timeDisplay()
{
	if (Options::showFundsOnGeoscape)
	{
		_txtFunds->setText(Unicode::formatFunding(_game->getSavedGame()->getFunds()));
	}

	std::ostringstream ss;
	ss << std::setfill('0') << std::setw(2) << _game->getSavedGame()->getTime()->getSecond();
	_txtSec->setText(ss.str());

	std::ostringstream ss2;
	ss2 << std::setfill('0') << std::setw(2) << _game->getSavedGame()->getTime()->getMinute();
	_txtMin->setText(ss2.str());

	std::ostringstream ss3;
	ss3 << _game->getSavedGame()->getTime()->getHour();
	_txtHour->setText(ss3.str());

	std::ostringstream ss4;
	ss4 << _game->getSavedGame()->getTime()->getDayString(_game->getLanguage());
	_txtDay->setText(ss4.str());

	_txtWeekday->setText(tr(_game->getSavedGame()->getTime()->getWeekdayString()));

	_txtMonth->setText(tr(_game->getSavedGame()->getTime()->getMonthString()));

	std::ostringstream ss5;
	ss5 << _game->getSavedGame()->getTime()->getYear();
	_txtYear->setText(ss5.str());
}

/**
 * Advances the game timer according to
 * the timer speed set, and calls the respective
 * triggers. The timer always advances in "5 secs"
 * cycles, regardless of the speed, otherwise it might
 * skip important steps. Instead, it just keeps advancing
 * the timer until the next speed step (eg. the next day
 * on 1 Day speed) or until an event occurs, since updating
 * the screen on each step would become cumbersomely slow.
 */
void GeoscapeState::timeAdvance()
{
	int timeSpan = 0;
	if (_timeSpeed == _btn5Secs)
	{
		if (Options::oxceGeoSlowdownFactor > 1)
		{
			_slowdownCounter--;
			if (_slowdownCounter > 0)
			{
				// wait
				_globe->draw();
				return;
			}
			else
			{
				_slowdownCounter = Clamp(Options::oxceGeoSlowdownFactor, 2, 100);
			}
		}
		timeSpan = 1;
	}
	else if (_timeSpeed == _btn1Min)
	{
		timeSpan = 12;
	}
	else if (_timeSpeed == _btn5Mins)
	{
		timeSpan = 12 * 5;
	}
	else if (_timeSpeed == _btn30Mins)
	{
		timeSpan = 12 * 5 * 6;
	}
	else if (_timeSpeed == _btn1Hour)
	{
		timeSpan = 12 * 5 * 6 * 2;
	}
	else if (_timeSpeed == _btn1Day)
	{
		timeSpan = 12 * 5 * 6 * 2 * 24;
	}


	for (int i = 0; i < timeSpan && !_pause; ++i)
	{
		TimeTrigger trigger;
		trigger = _game->getSavedGame()->getTime()->advance();
		switch (trigger)
		{
		case TIME_1MONTH:
			time1Month();
			FALLTHROUGH;
		case TIME_1DAY:
			time1Day();
			FALLTHROUGH;
		case TIME_1HOUR:
			time1Hour();
			FALLTHROUGH;
		case TIME_30MIN:
			time30Minutes();
			FALLTHROUGH;
		case TIME_10MIN:
			time10Minutes();
			FALLTHROUGH;
		case TIME_5SEC:
			time5Seconds();
		}
	}

	_pause = !_dogfightsToBeStarted.empty() || _zoomInEffectTimer->isRunning() || _zoomOutEffectTimer->isRunning();

	timeDisplay();
	_globe->draw();
}

/**
 * Update list of active crafts.
 * @return Const pointer to updated list.
 */
const std::vector<Craft*>* GeoscapeState::updateActiveCrafts()
{
	_activeCrafts.clear();
	for (auto base : *_game->getSavedGame()->getBases())
	{
		for (auto craft : *base->getCrafts())
		{
			if (craft->getStatus() == "STR_OUT" && !craft->isDestroyed())
			{
				_activeCrafts.push_back(craft);
			}
		}
	}
	return &_activeCrafts;
}

/**
 * Takes care of any game logic that has to
 * run every game second, like craft movement.
 */
void GeoscapeState::time5Seconds()
{
	// If in "slow mode", handle UFO hunting and escorting logic every 5 seconds, not only every 10 minutes
	if ((_timeSpeed == _btn5Secs || _timeSpeed == _btn1Min) && _game->getMod()->getHunterKillerFastRetarget())
	{
		ufoHuntingAndEscorting();
	}

	// Game over if there are no more bases.
	if (_game->getSavedGame()->getBases()->empty())
	{
		_game->getSavedGame()->setEnding(END_LOSE);
	}
	if (_game->getSavedGame()->getEnding() == END_LOSE)
	{
		_game->pushState(new CutsceneState(_game->getMod()->getLoseDefeatCutscene()));
		if (_game->getSavedGame()->isIronman())
		{
			_game->pushState(new SaveGameState(OPT_GEOSCAPE, SAVE_IRONMAN, _palette));
		}
		return;
	}

	auto crafts = updateActiveCrafts();

	// Handle UFO logic
	bool ufoIsAttacking = false;
	for (std::vector<Ufo*>::iterator i = _game->getSavedGame()->getUfos()->begin(); i != _game->getSavedGame()->getUfos()->end(); ++i)
	{
		switch ((*i)->getStatus())
		{
		case Ufo::FLYING:
			(*i)->think();
			if ((*i)->reachedDestination() && !(*i)->isEscorting())
			{
				Craft* c = dynamic_cast<Craft*>((*i)->getDestination());
				if (c != 0 && !c->isDestroyed() && (*i)->isHunting())
				{
					// Check if some other HK already attacked before us (at the very same moment)
					int hkDogfights = 0;
					for (auto f : _dogfights) if (f->isUfoAttacking()) { hkDogfights++; }
					for (auto g : _dogfightsToBeStarted) if (g->isUfoAttacking()) { hkDogfights++; }

					// If yes, wait... not more than 1 HK interception allowed at a time.
					if (hkDogfights >= 1)
					{
						continue;
					}

					// If not, interrupt all other (regular) interceptions to prevent a dead-lock (and other possible side effects)
					Collections::deleteAll(_dogfights);
					Collections::deleteAll(_dogfightsToBeStarted);
					_minimizedDogfights = 0;

					// Start the dogfight
					{
						// Main target
						_dogfightsToBeStarted.push_back(new DogfightState(this, c, *i, true));

						// Start fighting escorts and other craft as well (if they are in escort range)
						if (_game->getMod()->getEscortsJoinFightAgainstHK())
						{
							int secondaryTargets = 0;
							for (auto craft : *crafts)
							{
								// craft is close enough and has at least one loaded weapon
								if (craft != c && craft->getNumWeapons(true) > 0 && craft->getDistance(c) < Nautical(_game->getMod()->getEscortRange()))
								{
									// only up to 4 dogfights = 1 main + 3 secondary
									if (secondaryTargets < 3)
									{
										// Note: push_front() is used so that main target is attacked first
										_dogfightsToBeStarted.push_front(new DogfightState(this, craft, *i, true));
										secondaryTargets++;
									}
								}
							}
						}

						if (!_dogfightStartTimer->isRunning())
						{
							_pause = true;
							timerReset();
							_globe->center(c->getLongitude(), c->getLatitude());
							startDogfight();
							_dogfightStartTimer->start();
						}
						_game->getMod()->playMusic("GMINTER");

						// Don't process certain craft logic (moving and reaching destination)
						ufoIsAttacking = true;
					}
					// Don't handle other logic for this UFO, just continue with the next one
					continue;
				}

				size_t count = _game->getSavedGame()->getMissionSites()->size();
				AlienMission *mission = (*i)->getMission();
				bool detected = (*i)->getDetected();
				mission->ufoReachedWaypoint(**i, *_game, *_globe);
				if (Options::oxceUfoLandingAlert && (*i)->getStatus() == Ufo::LANDED && (*i)->getDetected() && (*i)->getLandId() != 0)
				{
					std::string msg = tr("STR_UFO_HAS_LANDED").arg((*i)->getName(_game->getLanguage()));
					popup(new CraftErrorState(this, msg));
				}
				if (detected != (*i)->getDetected() && !(*i)->getFollowers()->empty())
				{
					if (!((*i)->getTrajectory().getID() == UfoTrajectory::RETALIATION_ASSAULT_RUN && (*i)->getStatus() == Ufo::LANDED))
						popup(new UfoLostState((*i)->getName(_game->getLanguage())));
				}
				if (count < _game->getSavedGame()->getMissionSites()->size())
				{
					MissionSite *site = _game->getSavedGame()->getMissionSites()->back();
					site->setDetected(true);
					popup(new MissionDetectedState(site, this));
				}
				// If UFO was destroyed, don't spawn missions
				if ((*i)->getStatus() == Ufo::DESTROYED)
					return;
				if (Base *base = dynamic_cast<Base*>((*i)->getDestination()))
				{
					mission->setWaveCountdown(30 * (RNG::generate(0, 400) + 48));
					(*i)->setDestination(0);
					base->setupDefenses();
					timerReset();
					if (!base->getDefenses()->empty())
					{
						popup(new BaseDefenseState(base, *i, this));
						return; // don't allow multiple simultaneous attacks in the same game tick
					}
					else
					{
						handleBaseDefense(base, *i);
						return;
					}
				}
			}
			// Init UFO shields
			if ((*i)->getShield() == -1)
			{
				(*i)->setShield((*i)->getCraftStats().shieldCapacity);
			}
			// Recharge UFO shields
			else if ((*i)->getShield() < (*i)->getCraftStats().shieldCapacity)
			{
				int shieldRechargeInGeoscape = (*i)->getCraftStats().shieldRechargeInGeoscape;
				if (shieldRechargeInGeoscape == -1)
				{
					(*i)->setShield((*i)->getCraftStats().shieldCapacity);
				}
				else if (shieldRechargeInGeoscape > 0)
				{
					int total = shieldRechargeInGeoscape / 100;
					if (RNG::percent(shieldRechargeInGeoscape % 100))
						total++;
					(*i)->setShield((*i)->getShield() + total);
				}
			}
			break;
		case Ufo::LANDED:
			(*i)->think();
			if ((*i)->getSecondsRemaining() == 0)
			{
				AlienMission *mission = (*i)->getMission();
				bool detected = (*i)->getDetected();
				mission->ufoLifting(**i, *_game->getSavedGame());
				if (detected != (*i)->getDetected() && !(*i)->getFollowers()->empty())
				{
					popup(new UfoLostState((*i)->getName(_game->getLanguage())));
				}
			}
			break;
		case Ufo::CRASHED:
			(*i)->think();
			if ((*i)->getSecondsRemaining() == 0)
			{
				(*i)->setDetected(false);
				(*i)->setStatus(Ufo::DESTROYED);
			}
			break;
		case Ufo::DESTROYED:
			// Nothing to do
			break;
		}
	}

	// Handle craft logic
	for (std::vector<Base*>::iterator i = _game->getSavedGame()->getBases()->begin(); i != _game->getSavedGame()->getBases()->end(); ++i)
	{
		for (std::vector<Craft*>::iterator j = (*i)->getCrafts()->begin(); j != (*i)->getCrafts()->end();)
		{
			if ((*j)->isDestroyed())
			{
				for (std::vector<Country*>::iterator country = _game->getSavedGame()->getCountries()->begin(); country != _game->getSavedGame()->getCountries()->end(); ++country)
				{
					if ((*country)->getRules()->insideCountry((*j)->getLongitude(), (*j)->getLatitude()))
					{
						(*country)->addActivityXcom(-(*j)->getRules()->getScore());
						break;
					}
				}
				for (std::vector<Region*>::iterator region = _game->getSavedGame()->getRegions()->begin(); region != _game->getSavedGame()->getRegions()->end(); ++region)
				{
					if ((*region)->getRules()->insideRegion((*j)->getLongitude(), (*j)->getLatitude()))
					{
						(*region)->addActivityXcom(-(*j)->getRules()->getScore());
						break;
					}
				}
				// if a transport craft has been shot down, kill all the soldiers on board.
				if ((*j)->getRules()->getSoldiers() > 0)
				{
					for (std::vector<Soldier*>::iterator k = (*i)->getSoldiers()->begin(); k != (*i)->getSoldiers()->end();)
					{
						if ((*k)->getCraft() == (*j))
						{
							k = _game->getSavedGame()->killSoldier(*k);
						}
						else
						{
							++k;
						}
					}
				}
				_game->getSavedGame()->stopHuntingXcomCraft((*j)); // craft destroyed in dogfight
				Craft *craft = *j;
				j = (*i)->removeCraft(craft, false);
				delete craft;
				continue;
			}
			if ((*j)->getDestination() != 0)
			{
				Ufo* u = dynamic_cast<Ufo*>((*j)->getDestination());
				if (u != 0)
				{
					if (!u->getDetected())
					{
						if (u->getTrajectory().getID() == UfoTrajectory::RETALIATION_ASSAULT_RUN && (u->getStatus() == Ufo::LANDED || u->getStatus() == Ufo::DESTROYED))
						{
							(*j)->returnToBase();
						}
						else
						{
							Waypoint *w = new Waypoint();
							w->setLongitude(u->getLongitude());
							w->setLatitude(u->getLatitude());
							w->setId(u->getId());
							(*j)->setDestination(0);
							popup(new GeoscapeCraftState((*j), _globe, w));
						}
					}
					if (u->getStatus() == Ufo::LANDED && (*j)->isInDogfight())
					{
						(*j)->setInDogfight(false);
					}
					else if (u->getStatus() == Ufo::DESTROYED)
					{
						(*j)->returnToBase();
					}
				}
				else
				{
					if ((*j)->isInDogfight())
					{
						(*j)->setInDogfight(false);
					}
				}
			}

			if (!ufoIsAttacking)
			{
				bool returnedToBase = (*j)->think();
				if (returnedToBase)
				{
					_game->getSavedGame()->stopHuntingXcomCraft((*j)); // hiding in the base is good enough, obviously
				}
			}

			// Handle craft shield recharge
			if (!ufoIsAttacking && (*j)->getShield() < (*j)->getCraftStats().shieldCapacity)
			{
				int shieldRechargeInGeoscape = (*j)->getCraftStats().shieldRechargeInGeoscape;
				if (shieldRechargeInGeoscape == -1)
				{
					(*j)->setShield((*j)->getCraftStats().shieldCapacity);
				}
				else if (shieldRechargeInGeoscape > 0)
				{
					int total = (*j)->getCraftStats().shieldRechargeInGeoscape / 100;
					if (RNG::percent((*j)->getCraftStats().shieldRechargeInGeoscape % 100))
						total++;
					(*j)->setShield((*j)->getShield() + total);
				}
			}

			if (!ufoIsAttacking && (*j)->reachedDestination())
			{
				Ufo* u = dynamic_cast<Ufo*>((*j)->getDestination());
				Waypoint *w = dynamic_cast<Waypoint*>((*j)->getDestination());
				MissionSite* m = dynamic_cast<MissionSite*>((*j)->getDestination());
				AlienBase* b = dynamic_cast<AlienBase*>((*j)->getDestination());
				Craft* x = dynamic_cast<Craft*>((*j)->getDestination());
				if (u != 0)
				{
					switch (u->getStatus())
					{
					case Ufo::FLYING:
						// Not more than 4 interceptions at a time... but hunter-killers are always allowed
						if (!u->isHunterKiller() && _dogfights.size() + _dogfightsToBeStarted.size() >= 4)
						{
							++j;
							continue;
						}
						// Can we actually fight it
						if (!(*j)->isInDogfight() && u->getSpeed() <= (*j)->getCraftStats().speedMax)
						{
							if (u->isHunterKiller())
							{
								// Check if some other HK already attacked before us (at the very same moment)
								int hkDogfights = 0;
								for (auto f : _dogfights) if (f->isUfoAttacking()) { hkDogfights++; }
								for (auto g : _dogfightsToBeStarted) if (g->isUfoAttacking()) { hkDogfights++; }

								// If yes, wait... not more than 1 HK interception allowed at a time.
								if (hkDogfights >= 1)
								{
									++j;
									continue;
								}

								// If not, interrupt all other (regular) interceptions to prevent a dead-lock (and other possible side effects)
								Collections::deleteAll(_dogfights);
								Collections::deleteAll(_dogfightsToBeStarted);
								_minimizedDogfights = 0;

								// Don't process certain craft logic (moving and reaching destination)
								ufoIsAttacking = true;
							}

							// Main target
							DogfightState* dogfight = new DogfightState(this, (*j), u, u->isHunterKiller());
							_dogfightsToBeStarted.push_back(dogfight);

							if (u->isHunterKiller() && _game->getMod()->getEscortsJoinFightAgainstHK())
							{
								// Start fighting escorts and other craft as well (if they are in escort range)
								int secondaryTargets = 0;
								for (auto craft : *crafts)
								{
									// craft is flying (i.e. not in base)
									if (craft != (*j))
									{
										// craft is close enough and has at least one loaded weapon
										if (craft->getNumWeapons(true) > 0 && craft->getDistance((*j)) < Nautical(_game->getMod()->getEscortRange()))
										{
											// only up to 4 dogfights = 1 main + 3 secondary
											if (secondaryTargets < 3)
											{
												// Note: push_front() is used so that main target is attacked first
												_dogfightsToBeStarted.push_front(new DogfightState(this, craft, u, u->isHunterKiller()));
												secondaryTargets++;
											}
										}
									}
								}
							}

							// Ignore these restrictions when fighting against a HK, otherwise it's very easy to avoid being attacked
							if (!u->isHunterKiller())
							{
								if ((*j)->getRules()->isWaterOnly() && u->getAltitudeInt() > (*j)->getRules()->getMaxAltitude())
								{
									popup(new DogfightErrorState((*j), tr("STR_UNABLE_TO_ENGAGE_DEPTH")));
									dogfight->setMinimized(true);
									dogfight->setWaitForAltitude(true);
								}
								else if ((*j)->getRules()->isWaterOnly() && !_globe->insideLand((*j)->getLongitude(), (*j)->getLatitude()))
								{
									popup(new DogfightErrorState((*j), tr("STR_UNABLE_TO_ENGAGE_AIRBORNE")));
									dogfight->setMinimized(true);
									dogfight->setWaitForPoly(true);
								}
							}

							if (!_dogfightStartTimer->isRunning())
							{
								_pause = true;
								timerReset();
								_globe->center((*j)->getLongitude(), (*j)->getLatitude());
								startDogfight();
								_dogfightStartTimer->start();
							}
							_game->getMod()->playMusic("GMINTER");
						}
						break;
					case Ufo::LANDED:
					case Ufo::CRASHED:
					case Ufo::DESTROYED: // Just before expiration
						if (((*j)->getNumSoldiers() > 0 || (*j)->getNumVehicles() > 0) && (*j)->getRules()->getAllowLanding())
						{
							if (!(*j)->isInDogfight())
							{
								// look up polygons texture
								int texture, shade;
								_globe->getPolygonTextureAndShade(u->getLongitude(), u->getLatitude(), &texture, &shade);
								timerReset();
								auto globeTexture = _game->getMod()->getGlobe()->getTexture(texture);
								popup(new ConfirmLandingState(*j, globeTexture, globeTexture, shade));
							}
						}
						else if (u->getStatus() != Ufo::LANDED)
						{
							(*j)->returnToBase();
						}
						break;
					}
				}
				else if (w != 0)
				{
					if (!(*j)->getIsAutoPatrolling())
					{
						popup(new CraftPatrolState((*j), _globe));
					}
					(*j)->setDestination(0);
				}
				else if (m != 0)
				{
					if (((*j)->getNumSoldiers() > 0 || (*j)->getNumVehicles() > 0) && (*j)->getRules()->getAllowLanding())
					{
						// look up polygons texture
						int texture, shade;
						_globe->getPolygonTextureAndShade(m->getLongitude(), m->getLatitude(), &texture, &shade);
						timerReset();
						auto globeTexture = _game->getMod()->getGlobe()->getTexture(texture);
						auto missionTexture = _game->getMod()->getGlobe()->getTexture(m->getTexture());
						if (!missionTexture)
						{
							missionTexture = globeTexture;
						}
						popup(new ConfirmLandingState(*j, missionTexture, globeTexture, shade));
					}
					else
					{
						(*j)->returnToBase();
					}
				}
				else if (b != 0)
				{
					if (b->isDiscovered())
					{
						if (((*j)->getNumSoldiers() > 0 || (*j)->getNumVehicles() > 0) && (*j)->getRules()->getAllowLanding())
						{
							int texture, shade;
							_globe->getPolygonTextureAndShade(b->getLongitude(), b->getLatitude(), &texture, &shade);
							timerReset();
							auto globeTexture = _game->getMod()->getGlobe()->getTexture(texture);
							popup(new ConfirmLandingState(*j, globeTexture, globeTexture, shade));
						}
						else
						{
							(*j)->returnToBase();
						}
					}
				}
				else if (x != 0)
				{
					if (x->getStatus() != "STR_OUT" || x->isDestroyed())
					{
						(*j)->returnToBase();
					}
				}
			}
			 ++j;
		}
	}

	// Clean up dead UFOs and end dogfights which were minimized.
	Collections::deleteIf(*_game->getSavedGame()->getUfos(), _game->getSavedGame()->getUfos()->size(),
		[&](Ufo* ufo)
		{
			if (ufo->getStatus() == Ufo::DESTROYED)
			{
				Collections::deleteIf(_dogfights, _dogfights.size(),
					[&](DogfightState* dogfight)
					{
						return dogfight->getUfo() == ufo;
					}
				);
				return true;
			}
			else
			{
				return false;
			}
		}
	);

	// Check any dogfights waiting to open
	for (std::list<DogfightState*>::iterator d = _dogfights.begin(); d != _dogfights.end(); ++d)
	{
		if ((*d)->isMinimized())
		{
			if (((*d)->getWaitForPoly() && _globe->insideLand((*d)->getUfo()->getLongitude(), (*d)->getUfo()->getLatitude())) ||
				((*d)->getWaitForAltitude() && (*d)->getUfo()->getAltitudeInt() <= (*d)->getCraft()->getRules()->getMaxAltitude()))
			{
				_pause = true; // the USO reached the sea during this interval period, stop the timer and let handleDogfights() take it from there.
				timerReset();
			}
		}
	}

	// Clean up unused waypoints
	Collections::deleteIf(*_game->getSavedGame()->getWaypoints(), _game->getSavedGame()->getWaypoints()->size(),
		[&](Waypoint* way)
		{
			return way->getFollowers()->empty();
		}
	);
}

/**
 * Functor that attempt to detect an XCOM base.
 */
class DetectXCOMBase
{
	typedef Ufo* argument_type;
	typedef bool result_type;

public:
	/// Create a detector for the given base.
	DetectXCOMBase(const Base &base) : _base(base) { /* Empty by design.  */ }
	/// Attempt detection
	bool operator()(const Ufo *ufo) const;
private:
	const Base &_base;	//!< The target base.
};

/**
 * Only UFOs within detection range of the base have a chance to detect it.
 * @param ufo Pointer to the UFO attempting detection.
 * @return If the base is detected by @a ufo.
 */
bool DetectXCOMBase::operator()(const Ufo *ufo) const
{
	if (ufo->getTrajectoryPoint() <= 1) return false;
	if (ufo->getTrajectory().getZone(ufo->getTrajectoryPoint()) == 5) return false;
	if ((ufo->getMission()->getRules().getObjective() != OBJECTIVE_RETALIATION && !Options::aggressiveRetaliation) ||	// only UFOs on retaliation missions actively scan for bases
		ufo->getTrajectory().getID() == UfoTrajectory::RETALIATION_ASSAULT_RUN || 										// UFOs attacking a base don't detect!
		ufo->isCrashed() ||																								// Crashed UFOs don't detect!
		_base.getDistance(ufo) >= Nautical(ufo->getCraftStats().sightRange))											// UFOs have a detection range of 80 XCOM units. - we use a great circle formula and nautical miles.
	{
		return false;
	}
	return RNG::percent(_base.getDetectionChance());
}

/**
 * Functor that marks an XCOM base for retaliation.
 * This is required because of the iterator type.
 */
struct SetRetaliationTarget
{
	typedef std::map<const Region*, Base*>::value_type argument_type;
	typedef void result_type;

	/// Mark as a valid retaliation target.
	void operator()(const argument_type &iter) const { iter.second->setRetaliationTarget(true); }
};

/**
 * Takes care of any game logic that has to
 * run every game ten minutes, like fuel consumption.
 */
void GeoscapeState::time10Minutes()
{
	for (std::vector<Base*>::iterator i = _game->getSavedGame()->getBases()->begin(); i != _game->getSavedGame()->getBases()->end(); ++i)
	{
		// Fuel consumption for XCOM craft.
		for (std::vector<Craft*>::iterator j = (*i)->getCrafts()->begin(); j != (*i)->getCrafts()->end(); ++j)
		{
			if ((*j)->getStatus() == "STR_OUT")
			{
				int escortSpeed = 0;
				{
					Craft *escortee = dynamic_cast<Craft*>((*j)->getDestination());
					if (escortee != 0)
					{
						if ((*j)->getDistance(escortee) < Nautical(_game->getMod()->getEscortRange()))
						{
							escortSpeed = escortee->getSpeed();
						}
					}
				}
				(*j)->consumeFuel(escortSpeed);
				if (!(*j)->getLowFuel() && (*j)->getFuel() <= (*j)->getFuelLimit())
				{
					(*j)->setLowFuel(true);
					(*j)->returnToBase();
					if (!(*j)->getIsAutoPatrolling())
					{
						popup(new LowFuelState((*j), this));
					}
				}

				if ((*j)->getDestination() == 0 && (*j)->getCraftStats().sightRange > 0)
				{
					double range = Nautical((*j)->getCraftStats().sightRange);
					for (std::vector<AlienBase*>::iterator b = _game->getSavedGame()->getAlienBases()->begin(); b != _game->getSavedGame()->getAlienBases()->end(); ++b)
					{
						if ((*j)->getDistance(*b) <= range)
						{
							if (RNG::percent(50-((*j)->getDistance(*b) / range) * 50) && !(*b)->isDiscovered())
							{
								(*b)->setDiscovered(true);
							}
						}
					}
				}
			}
		}
	}
	if (Options::aggressiveRetaliation)
	{
		// Detect as many bases as possible.
		for (std::vector<Base*>::iterator iBase = _game->getSavedGame()->getBases()->begin(); iBase != _game->getSavedGame()->getBases()->end(); ++iBase)
		{
			// Find a UFO that detected this base, if any.
			std::vector<Ufo*>::const_iterator uu = std::find_if (_game->getSavedGame()->getUfos()->begin(), _game->getSavedGame()->getUfos()->end(), DetectXCOMBase(**iBase));
			if (uu != _game->getSavedGame()->getUfos()->end())
			{
				// Base found
				(*iBase)->setRetaliationTarget(true);
			}
		}
	}
	else
	{
		// Only remember last base in each region.
		std::map<const Region *, Base *> discovered;
		for (std::vector<Base*>::iterator iBase = _game->getSavedGame()->getBases()->begin(); iBase != _game->getSavedGame()->getBases()->end(); ++iBase)
		{
			// Find a UFO that detected this base, if any.
			std::vector<Ufo*>::const_iterator uu = std::find_if (_game->getSavedGame()->getUfos()->begin(), _game->getSavedGame()->getUfos()->end(), DetectXCOMBase(**iBase));
			if (uu != _game->getSavedGame()->getUfos()->end())
			{
				discovered[_game->getSavedGame()->locateRegion(**iBase)] = *iBase;
			}
		}
		// Now mark the bases as discovered.
		std::for_each(discovered.begin(), discovered.end(), SetRetaliationTarget());
	}

	// Handle alien bases detecting xcom craft and generating hunt missions
	baseHunting();

	// Handle UFO re-targeting (i.e. hunting and escorting) logic
	ufoHuntingAndEscorting();
}

void GeoscapeState::ufoHuntingAndEscorting()
{
	auto crafts = updateActiveCrafts();

	for (std::vector<Ufo*>::iterator ufo = _game->getSavedGame()->getUfos()->begin(); ufo != _game->getSavedGame()->getUfos()->end(); ++ufo)
	{
		if ((*ufo)->isHunterKiller() && (*ufo)->getStatus() == Ufo::FLYING)
		{
			// current target and attraction
			int newAttraction = INT_MAX;
			Craft *newTarget = 0;
			Craft *originalTarget = 0;
			if ((*ufo)->isHunting())
			{
				originalTarget = (*ufo)->getTargetedXcomCraft();
			}
			if (originalTarget && !originalTarget->getMissionComplete())
			{
				if ((*ufo)->insideRadarRange(originalTarget))
				{
					newTarget = originalTarget;
					newAttraction = newTarget->getHunterKillerAttraction((*ufo)->getHuntMode());
				}
			}

			// look for more attractive target
			for (auto craft : *crafts)
			{
				if (!craft->getMissionComplete() && !craft->getRules()->isUndetectable())
				{
					int tmpAttraction = craft->getHunterKillerAttraction((*ufo)->getHuntMode());
					if (tmpAttraction < newAttraction && (*ufo)->insideRadarRange(craft))
					{
						newTarget = craft;
						newAttraction = tmpAttraction;
					}
				}
			}

			if (newTarget)
			{
				if (newTarget != originalTarget)
				{
					// set new target
					(*ufo)->setTargetedXcomCraft(newTarget);
					// TODO: rethink: always reveal the hunting UFO (even outside of radar range?)
					(*ufo)->setDetected(true);
					if ((*ufo)->getId() == 0)
					{
						(*ufo)->setId(_game->getSavedGame()->getId("STR_UFO"));
					}
					// inform the player
					if ((*ufo)->getRules()->getHuntAlertSound() > -1)
					{
						_game->getMod()->getSound("GEO.CAT", (*ufo)->getRules()->getHuntAlertSound())->play();
					}
					std::string msg = tr("STR_UFO_STARTED_HUNTING")
						.arg((*ufo)->getName(_game->getLanguage()))
						.arg(newTarget->getName(_game->getLanguage()));
					popup(new CraftErrorState(this, msg));
				}
			}
			else if (originalTarget)
			{
				// stop hunting
				(*ufo)->resetOriginalDestination(originalTarget);
			}

			// If we are not preoccupied by hunting, let's see if there is still anyone left to escort
			if ((*ufo)->isEscort() && !(*ufo)->isHunting() && !(*ufo)->isEscorting())
			{
				// Find a UFO to escort
				for (std::vector<Ufo*>::const_iterator t = _game->getSavedGame()->getUfos()->begin(); t != _game->getSavedGame()->getUfos()->end(); ++t)
				{
					// From the same mission
					if ((*t)->getMission()->getId() == (*ufo)->getMission()->getId())
					{
						// But not another hunter-killer, we escort only normal UFOs
						if (!(*t)->isHunterKiller())
						{
							(*ufo)->setEscortedUfo((*t));
							break;
						}
					}
				}
			}
		}
	}
}

void GeoscapeState::baseHunting()
{
	auto crafts = updateActiveCrafts();

	for (std::vector<AlienBase*>::iterator ab = _game->getSavedGame()->getAlienBases()->begin(); ab != _game->getSavedGame()->getAlienBases()->end(); ++ab)
	{
		if ((*ab)->getDeployment()->getBaseDetectionRange() > 0)
		{
			// Increase counter by 10 minutes
			(*ab)->setMinutesSinceLastHuntMissionGeneration((*ab)->getMinutesSinceLastHuntMissionGeneration() + 10);

			// Check counter
			if ((*ab)->getMinutesSinceLastHuntMissionGeneration() >= (*ab)->getDeployment()->getHuntMissionMaxFrequency())
			{
				// Look for nearby craft
				bool started = false;
				for (auto craft : *crafts)
				{
					// Craft is flying (i.e. not in base)
					if (craft->getStatus() == "STR_OUT" && !craft->isDestroyed() && !craft->getRules()->isUndetectable())
					{
						// Craft is close enough and RNG is in our favour
						if (craft->getDistance((*ab)) < Nautical((*ab)->getDeployment()->getBaseDetectionRange()) && RNG::percent((*ab)->getDeployment()->getBaseDetectionChance()))
						{
							// Generate a hunt mission
							auto huntMission = (*ab)->getDeployment()->generateHuntMission(_game->getSavedGame()->getMonthsPassed());
							if (_game->getMod()->getAlienMission(huntMission))
							{
								// Spawn hunt mission for this base.
								const RuleAlienMission &rule = *_game->getMod()->getAlienMission(huntMission);
								AlienMission *mission = new AlienMission(rule);
								mission->setRegion(_game->getSavedGame()->locateRegion(*(*ab))->getRules()->getType(), *_game->getMod());
								mission->setId(_game->getSavedGame()->getId("ALIEN_MISSIONS"));
								mission->setRace((*ab)->getAlienRace());
								mission->setAlienBase((*ab));
								int targetZone = -1;
								if (mission->getRules().getObjective() == OBJECTIVE_SITE)
								{
									int missionZone = mission->getRules().getSpawnZone();
									RuleRegion *regionRules = _game->getMod()->getRegion(mission->getRegion());
									const std::vector<MissionArea> areas = regionRules->getMissionZones().at(missionZone).areas;
									if (!areas.empty())
									{
										targetZone = RNG::generate(0, areas.size() - 1);
									}
								}
								mission->setMissionSiteZone(targetZone);
								mission->start(*_game, *_globe);
								_game->getSavedGame()->getAlienMissions().push_back(mission);

								// Start immediately
								mission->think(*_game, *_globe);

								// Reset counter
								(*ab)->setMinutesSinceLastHuntMissionGeneration(0);
								started = true;
								break;
							}
							else if (huntMission != "")
							{
								throw Exception("Alien Base tried to generate undefined hunt mission: " + huntMission);
							}
						}
					}
					if (started) break;
				}
			}
		}
	}
}

/** @brief Process a MissionSite.
 * This function object will count down towards expiring a MissionSite, and handle expired MissionSites.
 * @param ts Pointer to mission site.
 * @return Has mission site expired?
 */
bool GeoscapeState::processMissionSite(MissionSite *site)
{
	bool removeSite = site->getSecondsRemaining() < 30 * 60;
	if (!removeSite)
	{
		site->setSecondsRemaining(site->getSecondsRemaining() - 30 * 60);
	}
	else
	{
		bool noFollowers = site->getFollowers()->empty();
		if (site->getRules()->despawnEvenIfTargeted())
		{
			std::vector<Craft*> followers = site->getCraftFollowers();
			for (std::vector<Craft*>::iterator k = followers.begin(); k != followers.end(); ++k)
			{
				(*k)->returnToBase();
			}
			if (!noFollowers)
			{
				popup(new UfoLostState(site->getName(_game->getLanguage())));
			}
		}
		else
		{
			removeSite = noFollowers; // CHEEKY EXPLOIT
		}
	}

	int score = removeSite ? site->getDeployment()->getDespawnPenalty() : site->getDeployment()->getPoints();

	Region *region = _game->getSavedGame()->locateRegion(*site);
	if (region)
	{
		region->addActivityAlien(score);
	}
	for (std::vector<Country*>::iterator k = _game->getSavedGame()->getCountries()->begin(); k != _game->getSavedGame()->getCountries()->end(); ++k)
	{
		if ((*k)->getRules()->insideCountry(site->getLongitude(), site->getLatitude()))
		{
			(*k)->addActivityAlien(score);
			break;
		}
	}

	return removeSite;
}

/**
 * Takes care of any game logic that has to
 * run every game half hour, like UFO detection.
 */
void GeoscapeState::time30Minutes()
{
	// Decrease mission countdowns
	for (auto am : _game->getSavedGame()->getAlienMissions())
	{
		am->think(*_game, *_globe);
	}

	// Remove finished missions
	Collections::deleteIf(
		_game->getSavedGame()->getAlienMissions(),
		[](AlienMission* am)
		{
			return am->isOver();
		}
	);

	// Handle crashed UFOs expiration
	for(auto ufo : *_game->getSavedGame()->getUfos())
	{
		if (ufo->getStatus() == Ufo::CRASHED)
		{
			if (ufo->getSecondsRemaining() >= 30 * 60)
			{
				ufo->setSecondsRemaining(ufo->getSecondsRemaining() - 30 * 60);
				continue;
			}
			// Marked expired UFOs for removal.
			ufo->setStatus(Ufo::DESTROYED);
		}
	}

	// Handle craft maintenance and alien base detection
	for (auto base : *_game->getSavedGame()->getBases())
	{
		for (auto craft : *base->getCrafts())
		{
			if (craft->getStatus() == "STR_REFUELLING")
			{
				std::string item = craft->getRules()->getRefuelItem();
				if (item.empty())
				{
					craft->refuel();
					// notification
					if (craft->getStatus() == "STR_READY" && craft->getRules()->notifyWhenRefueled())
					{
						std::string msg = tr("STR_CRAFT_IS_READY").arg(craft->getName(_game->getLanguage())).arg(base->getName());
						popup(new CraftErrorState(this, msg));
					}
					// auto-patrol
					if (craft->getStatus() == "STR_READY" && craft->getRules()->canAutoPatrol())
					{
						if (craft->getIsAutoPatrolling())
						{
							Waypoint *w = new Waypoint();
							w->setLongitude(craft->getLongitudeAuto());
							w->setLatitude(craft->getLatitudeAuto());
							if (w != 0 && w->getId() == 0)
							{
								w->setId(_game->getSavedGame()->getId("STR_WAY_POINT"));
								_game->getSavedGame()->getWaypoints()->push_back(w);
							}
							craft->setDestination(w);
							craft->setStatus("STR_OUT");
						}
					}
				}
				else
				{
					if (base->getStorageItems()->getItem(item) > 0)
					{
						base->getStorageItems()->removeItem(item);
						craft->refuel();
						craft->setLowFuel(false);
						// notification
						if (craft->getStatus() == "STR_READY" && craft->getRules()->notifyWhenRefueled())
						{
							std::string msg = tr("STR_CRAFT_IS_READY").arg(craft->getName(_game->getLanguage())).arg(base->getName());
							popup(new CraftErrorState(this, msg));
						}
						// auto-patrol
						if (craft->getStatus() == "STR_READY" && craft->getRules()->canAutoPatrol())
						{
							if (craft->getIsAutoPatrolling())
							{
								Waypoint *w = new Waypoint();
								w->setLongitude(craft->getLongitudeAuto());
								w->setLatitude(craft->getLatitudeAuto());
								if (w != 0 && w->getId() == 0)
								{
									w->setId(_game->getSavedGame()->getId("STR_WAY_POINT"));
									_game->getSavedGame()->getWaypoints()->push_back(w);
								}
								craft->setDestination(w);
								craft->setStatus("STR_OUT");
							}
						}
					}
					else if (!craft->getLowFuel())
					{
						std::string msg = tr("STR_NOT_ENOUGH_ITEM_TO_REFUEL_CRAFT_AT_BASE")
										   .arg(tr(item))
										   .arg(craft->getName(_game->getLanguage()))
										   .arg(base->getName());
						popup(new CraftErrorState(this, msg));
						if (craft->getFuel() > 0)
						{
							craft->setStatus("STR_READY");
						}
						else
						{
							craft->setLowFuel(true);
						}
					}
				}
			}
		}
	}

	// can be updated by previous loop
	auto crafts = updateActiveCrafts();

	// Handle UFO detection and give aliens points
	for (auto ufo : *_game->getSavedGame()->getUfos())
	{
		int points = ufo->getRules()->getMissionScore(); //one point per UFO in-flight per half hour
		switch (ufo->getStatus())
		{
		case Ufo::LANDED:
			points *= 2;
			FALLTHROUGH;
		case Ufo::FLYING:
			// Get area
			for (auto region : *_game->getSavedGame()->getRegions())
			{
				if (region->getRules()->insideRegion(ufo->getLongitude(), ufo->getLatitude()))
				{
					region->addActivityAlien(points);
					break;
				}
			}
			// Get country
			for (auto country : *_game->getSavedGame()->getCountries())
			{
				if (country->getRules()->insideCountry(ufo->getLongitude(), ufo->getLatitude()))
				{
					country->addActivityAlien(points);
					break;
				}
			}

			// Detection ufo state
			{
				auto maskTest = [](UfoDetection value, UfoDetection mask)
				{
					return (value & mask) == mask;
				};
				auto maskBitOr = [](UfoDetection value, UfoDetection mask)
				{
					return (UfoDetection)(value | mask);
				};

				auto detected = DETECTION_NONE;
				auto alreadyTracked = ufo->getDetected();

				for (auto base : *_game->getSavedGame()->getBases())
				{
					detected = maskBitOr(detected, base->detect(ufo, alreadyTracked));
				}

				for (auto craft : *crafts)
				{
					detected = maskBitOr(detected, craft->detect(ufo, alreadyTracked));
				}

				if (!alreadyTracked)
				{
					if (maskTest(detected, DETECTION_RADAR))
					{
						if (maskTest(detected, DETECTION_HYPERWAVE))
						{
							ufo->setHyperDetected(true);
						}
						ufo->setDetected(true);
						// don't show if player said he doesn't want to see this UFO anymore
						if (!_game->getSavedGame()->isUfoOnIgnoreList(ufo->getId()))
						{
							popup(new UfoDetectedState(ufo, this, true, ufo->getHyperDetected()));
						}
					}
				}
				else
				{
					if (maskTest(detected, DETECTION_HYPERWAVE))
					{
						ufo->setHyperDetected(true);
					}
					// TODO: rethink: hunting UFOs stay visible even outside of radar range?
					if (!maskTest(detected, DETECTION_RADAR) && !ufo->isHunting())
					{
						ufo->setDetected(false);
						ufo->setHyperDetected(false);
						if (!ufo->getFollowers()->empty())
						{
							popup(new UfoLostState(ufo->getName(_game->getLanguage())));
						}
					}
				}
			}

			break;
		case Ufo::CRASHED:
		case Ufo::DESTROYED:
			break;
		}
	}

	// Processes MissionSites
	Collections::deleteIf(
		*_game->getSavedGame()->getMissionSites(),
		[&](MissionSite* site)
		{
			return processMissionSite(site);
		}
	);

	// Decrease event countdowns and pop up if needed
	for (auto ge : _game->getSavedGame()->getGeoscapeEvents())
	{
		ge->think();

		if (ge->isOver())
		{
			bool interrupted = false;
			if (!ge->getRules().getInterruptResearch().empty())
			{
				if (_game->getSavedGame()->isResearched(ge->getRules().getInterruptResearch(), false))
				{
					interrupted = true;
				}
			}
			if (!interrupted)
			{
				timerReset();
				popup(new GeoscapeEventState(ge));
			}
		}
	}

	// Remove finished events
	Collections::deleteIf(
		_game->getSavedGame()->getGeoscapeEvents(),
		[](GeoscapeEvent *ge)
		{
			return ge->isOver();
		}
	);
}

/**
 * Takes care of any game logic that has to
 * run every game hour, like transfers.
 */
void GeoscapeState::time1Hour()
{
	// Handle craft maintenance
	for (std::vector<Base*>::iterator i = _game->getSavedGame()->getBases()->begin(); i != _game->getSavedGame()->getBases()->end(); ++i)
	{
		for (std::vector<Craft*>::iterator j = (*i)->getCrafts()->begin(); j != (*i)->getCrafts()->end(); ++j)
		{
			if ((*j)->getStatus() == "STR_REPAIRS")
			{
				(*j)->repair();
			}
			else if ((*j)->getStatus() == "STR_REARMING")
			{
				auto s = (*j)->rearm();
				if (s)
				{
					std::string msg = tr("STR_NOT_ENOUGH_ITEM_TO_REARM_CRAFT_AT_BASE")
									   .arg(tr(s->getType()))
									   .arg((*j)->getName(_game->getLanguage()))
									   .arg((*i)->getName());
					popup(new CraftErrorState(this, msg));
				}
			}
			if ((*j)->getShieldCapacity() > 0 && (*j)->getStatus() != "STR_OUT")
			{
				// Recharge craft shields in parallel (no wait for repair/rearm/refuel)
				(*j)->setShield((*j)->getShield() + (*j)->getRules()->getShieldRechargeAtBase());
			}
		}
	}

	// Handle transfers
	bool window = false;
	for (std::vector<Base*>::iterator i = _game->getSavedGame()->getBases()->begin(); i != _game->getSavedGame()->getBases()->end(); ++i)
	{
		for (std::vector<Transfer*>::iterator j = (*i)->getTransfers()->begin(); j != (*i)->getTransfers()->end(); ++j)
		{
			(*j)->advance(*i);
			if (!window && (*j)->getHours() <= 0)
			{
				window = true;
			}
		}
	}
	if (window)
	{
		popup(new ItemsArrivingState(this));
	}
	// Handle Production
	for (std::vector<Base*>::iterator i = _game->getSavedGame()->getBases()->begin(); i != _game->getSavedGame()->getBases()->end(); ++i)
	{
		std::map<Production*, productionProgress_e> toRemove;
		for (std::vector<Production*>::const_iterator j = (*i)->getProductions().begin(); j != (*i)->getProductions().end(); ++j)
		{
			toRemove[(*j)] = (*j)->step((*i), _game->getSavedGame(), _game->getMod(), _game->getLanguage());
		}
		for (std::map<Production*, productionProgress_e>::iterator j = toRemove.begin(); j != toRemove.end(); ++j)
		{
			if (j->second > PROGRESS_NOT_COMPLETE)
			{
				popup(new ProductionCompleteState((*i),  tr(j->first->getRules()->getName()), this, j->second, j->first));
				(*i)->removeProduction(j->first);
			}
		}

		if (Options::storageLimitsEnforced)
		{
			if ((*i)->storesOverfull())
			{
				timerReset();
				popup(new ErrorMessageState(tr("STR_STORAGE_EXCEEDED").arg((*i)->getName()), _palette, _game->getMod()->getInterface("geoscape")->getElement("errorMessage")->color, "BACK13.SCR", _game->getMod()->getInterface("geoscape")->getElement("errorPalette")->color));
				popup(new SellState((*i), 0));
			}
			else if (!_game->getSavedGame()->getAlienContainmentChecked())
			{
				_game->getSavedGame()->setAlienContainmentChecked(true);
				std::map<int, int> prisonTypes;
				RuleItem *rule = nullptr;
				for (auto &item : *(*i)->getStorageItems()->getContents())
				{
					rule = _game->getMod()->getItem(item.first, true);
					if (rule->isAlien())
					{
						prisonTypes[rule->getPrisonType()] += 1;
					}
				}
				for (auto &p : prisonTypes)
				{
					int prisonType = p.first;
					if ((*i)->getUsedContainment(prisonType) > (*i)->getAvailableContainment(prisonType))
					{
						_game->getSavedGame()->setAlienContainmentChecked(false);
						timerReset();
						popup(new ErrorMessageState(
							trAlt("STR_CONTAINMENT_EXCEEDED", prisonType).arg((*i)->getName()),
							_palette,
							_game->getMod()->getInterface("geoscape")->getElement("errorMessage")->color,
							"BACK01.SCR",
							_game->getMod()->getInterface("geoscape")->getElement("errorPalette")->color));
						popup(new ManageAlienContainmentState((*i), prisonType, OPT_GEOSCAPE));
						break;
					}
				}
			}
		}
	}
	for (std::vector<MissionSite*>::iterator i = _game->getSavedGame()->getMissionSites()->begin(); i != _game->getSavedGame()->getMissionSites()->end(); ++i)
	{
		if (!(*i)->getDetected())
		{
			(*i)->setDetected(true);
			popup(new MissionDetectedState(*i, this));
			break;
		}
	}

	updateSlackingIndicator();
}

/**
 * This class will attempt to generate a supply mission for a base.
 * Each alien base has a 6/101 chance to generate a supply mission.
 */
class GenerateSupplyMission
{
	typedef const AlienBase* argument_type;
	typedef void result_type;

public:
	/// Store rules and game data references for later use.
	GenerateSupplyMission(Game &engine, const Globe &globe) : _engine(engine), _globe(globe) { /* Empty by design */ }
	/// Check and spawn mission.
	void operator()(AlienBase *base) const;
private:
	Game &_engine;
	const Globe &_globe;
};

/**
 * Check and create supply mission for the given base.
 * There is a 6/101 chance of the mission spawning.
 * @param base A pointer to the alien base.
 */
void GenerateSupplyMission::operator()(AlienBase *base) const
{
	const Mod &_mod = *_engine.getMod();
	SavedGame &_save = *_engine.getSavedGame();

	std::string missionName = base->getDeployment()->chooseGenMissionType();
	if (_mod.getAlienMission(missionName))
	{
		if (base->getGenMissionCount() < base->getDeployment()->getGenMissionLimit() && RNG::percent(base->getDeployment()->getGenMissionFrequency()))
		{
			//Spawn supply mission for this base.
			const RuleAlienMission &rule = *_mod.getAlienMission(missionName);
			AlienMission *mission = new AlienMission(rule);
			std::string targetRegion;
			if (RNG::percent(rule.getTargetBaseOdds()))
			{
				// 1. target a random xcom base region
				std::vector<std::string> regionsWithXcomBases;
				for (std::vector<Base*>::const_iterator i = _save.getBases()->begin(); i != _save.getBases()->end(); ++i)
				{
					regionsWithXcomBases.push_back(_save.locateRegion(*(*i))->getRules()->getType());
				}
				targetRegion = regionsWithXcomBases[RNG::generate(0, regionsWithXcomBases.size() - 1)];
			}
			else if (rule.hasRegionWeights())
			{
				// 2. target one of the defined (weighted) regions
				targetRegion = rule.generateRegion(_save.getMonthsPassed());
			}
			else
			{
				// 3. target the region of the alien base (vanilla default)
				targetRegion = _save.locateRegion(*base)->getRules()->getType();
			}
			mission->setRegion(targetRegion, _mod);
			mission->setId(_save.getId("ALIEN_MISSIONS"));
			mission->setRace(base->getAlienRace());
			mission->setAlienBase(base);
			int targetZone = -1;
			if (mission->getRules().getObjective() == OBJECTIVE_SITE)
			{
				int missionZone = mission->getRules().getSpawnZone();
				RuleRegion *regionRules = _mod.getRegion(mission->getRegion());
				const std::vector<MissionArea> areas = regionRules->getMissionZones().at(missionZone).areas;
				if (!areas.empty())
				{
					targetZone = RNG::generate(0, areas.size() - 1);
				}
			}
			mission->setMissionSiteZone(targetZone);
			mission->start(_engine, _globe);
			base->setGenMissionCount(base->getGenMissionCount() + 1); // increase counter, used to check mission limit
			_save.getAlienMissions().push_back(mission);
		}
	}
	else if (!missionName.empty())
	{
		throw Exception("Alien Base tried to generate undefined mission: " + missionName);
	}
}

/**
 * Takes care of any game logic that has to
 * run every game day, like constructions.
 */
void GeoscapeState::time1Day()
{
	SavedGame *saveGame = _game->getSavedGame();
	Mod *mod = _game->getMod();
	bool psiStrengthEval = (Options::psiStrengthEval && saveGame->isResearched(mod->getPsiRequirements()));
	for (Base *base : *_game->getSavedGame()->getBases())
	{
		// Handle facility construction
		std::map<const RuleBaseFacility*, int> finishedFacilities;
		for (BaseFacility *facility : *base->getFacilities())
		{
			if (facility->getBuildTime() > 0)
			{
				facility->build();
				if (facility->getBuildTime() == 0)
				{
					finishedFacilities[facility->getRules()] += 1;
				}
			}
		}
		for (auto& f : finishedFacilities)
		{
			if (f.second > 1)
			{
				std::ostringstream ssf;
				ssf << tr(f.first->getType()) << " (x" << f.second << ")";
				popup(new ProductionCompleteState(base, ssf.str(), this, PROGRESS_CONSTRUCTION));
			}
			else
			{
				popup(new ProductionCompleteState(base, tr(f.first->getType()), this, PROGRESS_CONSTRUCTION));
			}
		}

		// Handle science project
		// 1. gather finished research
		std::vector<ResearchProject*> finished;
		for (ResearchProject *project : base->getResearch())
		{
			if (project->step())
			{
				finished.push_back(project);
			}
		}
		// 2. remember available research before adding new finished research
		std::vector<RuleResearch*> before;
		if (!finished.empty())
		{
			saveGame->getAvailableResearchProjects(before, mod, base);
		}
		// 3. add finished research, including lookups and getonefrees (up to 4x)
		for (ResearchProject *project : finished)
		{
			const RuleResearch *bonus = 0;
			const RuleResearch *research = project->getRules();

			// 3a. remove finished research from the base where it was researched
			base->removeResearch(project);
			project = nullptr;

			// 3b. handle interrogation and spawned items/events
			if (Options::retainCorpses && research->destroyItem())
			{
				auto ruleUnit = mod->getUnit(research->getName(), false);
				if (ruleUnit)
				{
					auto ruleCorpse = ruleUnit->getArmor()->getCorpseGeoscape();
					if (ruleCorpse && ruleCorpse->isRecoverable() && ruleCorpse->isCorpseRecoverable())
					{
						base->getStorageItems()->addItem(ruleCorpse->getType());
					}
				}
			}
			RuleItem *spawnedItem = _game->getMod()->getItem(research->getSpawnedItem());
			if (spawnedItem)
			{
				Transfer *t = new Transfer(1);
				t->setItems(research->getSpawnedItem());
				base->getTransfers()->push_back(t);
			}
			RuleEvent* spawnedEventRule = _game->getMod()->getEvent(research->getSpawnedEvent());
			if (spawnedEventRule)
			{
				GeoscapeEvent* newEvent = new GeoscapeEvent(*spawnedEventRule);
				int minutes = (spawnedEventRule->getTimer() + (RNG::generate(0, spawnedEventRule->getTimerRandom()))) / 30 * 30;
				if (minutes < 60) minutes = 60; // just in case
				newEvent->setSpawnCountdown(minutes);
				saveGame->getGeoscapeEvents().push_back(newEvent);

				// remember that it has been generated
				saveGame->addGeneratedEvent(spawnedEventRule);
			}
			// 3c. handle getonefrees (topic+lookup)
			if ((bonus = saveGame->selectGetOneFree(research)))
			{
				saveGame->addFinishedResearch(bonus, mod, base);
				if (!bonus->getLookup().empty())
				{
					saveGame->addFinishedResearch(mod->getResearch(bonus->getLookup(), true), mod, base);
				}
			}
			// 3d. determine and remember if the ufopedia article should pop up again or not
			// Note: because different topics may lead to the same lookup
			const RuleResearch *newResearch = research;
			std::string name = research->getLookup().empty() ? research->getName() : research->getLookup();
			if (saveGame->isResearched(name, false))
			{
				newResearch = 0;
			}
			// 3e. handle core research (topic+lookup)
			saveGame->addFinishedResearch(research, mod, base);
			if (!research->getLookup().empty())
			{
				saveGame->addFinishedResearch(mod->getResearch(research->getLookup(), true), mod, base);
			}
			// 3e. handle cutscene
			if (!research->getCutscene().empty())
			{
				popup(new CutsceneState(research->getCutscene()));
				if (saveGame->getEnding() == END_NONE)
				{
					const RuleVideo* videoRule = _game->getMod()->getVideo(research->getCutscene(), true);
					if (videoRule->getWinGame()) saveGame->setEnding(END_WIN);
					if (videoRule->getLoseGame()) saveGame->setEnding(END_LOSE);
				}
			}
			if (bonus && !bonus->getCutscene().empty())
			{
				popup(new CutsceneState(bonus->getCutscene()));
				if (saveGame->getEnding() == END_NONE)
				{
					const RuleVideo* videoRule = _game->getMod()->getVideo(bonus->getCutscene(), true);
					if (videoRule->getWinGame()) saveGame->setEnding(END_WIN);
					if (videoRule->getLoseGame()) saveGame->setEnding(END_LOSE);
				}
			}
			// 3e. handle research complete popup + ufopedia article popups (topic+bonus)
			popup(new ResearchCompleteState(newResearch, bonus, research));
			// 3f. reset timer
			timerReset();
			// 3g. warning if weapon is researched before its clip
			if (newResearch)
			{
				RuleItem *item = mod->getItem(newResearch->getName());
				if (item && item->getBattleType() == BT_FIREARM && !item->getPrimaryCompatibleAmmo()->empty())
				{
					RuleManufacture *man = mod->getManufacture(item->getType());
					if (man && !man->getRequirements().empty())
					{
						const auto &req = man->getRequirements();
						const RuleItem *ammo = item->getPrimaryCompatibleAmmo()->front();
						if (std::find_if(req.begin(), req.end(), [&](const RuleResearch* r){ return r->getName() == ammo->getType(); }) != req.end() && !saveGame->isResearched(req, true))
						{
							popup(new ResearchRequiredState(item));
						}
					}
				}
			}
			// 3h. inform about new possible research
			std::vector<RuleResearch *> after;
			saveGame->getAvailableResearchProjects(after, mod, base);
			std::vector<RuleResearch *> newPossibleResearch;
			saveGame->getNewlyAvailableResearchProjects(before, after, newPossibleResearch);
			popup(new NewPossibleResearchState(base, newPossibleResearch));
			// 3i. inform about new possible manufacture, purchase, craft and facilities
			std::vector<RuleManufacture *> newPossibleManufacture;
			saveGame->getDependableManufacture(newPossibleManufacture, research, mod, base);
			if (bonus)
			{
				saveGame->getDependableManufacture(newPossibleManufacture, bonus, mod, base);
			}
			if (!newPossibleManufacture.empty())
			{
				popup(new NewPossibleManufactureState(base, newPossibleManufacture));
			}
			std::vector<RuleItem *> newPossiblePurchase;
			_game->getSavedGame()->getDependablePurchase(newPossiblePurchase, research, _game->getMod());
			if (bonus)
			{
				_game->getSavedGame()->getDependablePurchase(newPossiblePurchase, bonus, _game->getMod());
			}
			if (!newPossiblePurchase.empty())
			{
				popup(new NewPossiblePurchaseState(base, newPossiblePurchase));
			}
			std::vector<RuleCraft *> newPossibleCraft;
			_game->getSavedGame()->getDependableCraft(newPossibleCraft, research, _game->getMod());
			if (bonus)
			{
				_game->getSavedGame()->getDependableCraft(newPossibleCraft, bonus, _game->getMod());
			}
			if (!newPossibleCraft.empty())
			{
				popup(new NewPossibleCraftState(base, newPossibleCraft));
			}
			std::vector<RuleBaseFacility *> newPossibleFacilities;
			_game->getSavedGame()->getDependableFacilities(newPossibleFacilities, research, _game->getMod());
			if (bonus)
			{
				_game->getSavedGame()->getDependableFacilities(newPossibleFacilities, bonus, _game->getMod());
			}
			if (!newPossibleFacilities.empty())
			{
				popup(new NewPossibleFacilityState(base, _globe, newPossibleFacilities));
			}
			// 3j. now iterate through all the bases and remove this project from their labs (unless it can still yield more stuff!)
			for (Base *otherBase : *saveGame->getBases())
			{
				for (ResearchProject* otherProject : otherBase->getResearch())
				{
					if (research->getName() == otherProject->getRules()->getName())
					{
						if (saveGame->hasUndiscoveredGetOneFree(research, true))
						{
							// This research topic still has some more undiscovered non-disabled and *AVAILABLE* "getOneFree" topics, keep it!
						}
						else if (saveGame->hasUndiscoveredProtectedUnlock(research, mod))
						{
							// This research topic still has one or more undiscovered non-disabled "protected unlocks", keep it!
						}
						else
						{
							// This topic can't give you anything else anymore, remove it!
							otherBase->removeResearch(otherProject);
							break;
						}
					}
				}
			}
		}

		// Handle soldier wounds and martial training
		auto recovery = base->getSumRecoveryPerDay();
		std::vector<Soldier *> trainingFinishedList;
		for (std::vector<Soldier*>::iterator j = base->getSoldiers()->begin(); j != base->getSoldiers()->end(); ++j)
		{
			(*j)->replenishStats(recovery);

			if ((*j)->isInTraining())
			{
				(*j)->trainPhys(_game->getMod()->getCustomTrainingFactor());
				(*j)->calcStatString(_game->getMod()->getStatStrings(), psiStrengthEval);
				if ((*j)->isFullyTrained())
				{
					(*j)->setTraining(false);
					trainingFinishedList.push_back(*j);
				}
			}
			else
			{
				if ((*j)->getReturnToTrainingWhenHealed() && !(*j)->isWounded())
				{
					if (!(*j)->isFullyTrained() && base->getFreeTrainingSpace() > 0)
					{
						(*j)->setTraining(true);
					}
					// only ever try to return once
					(*j)->setReturnToTrainingWhenHealed(false);
				}
			}
		}
		if (!trainingFinishedList.empty())
		{
			popup(new TrainingFinishedState(base, trainingFinishedList, false));
		}
		// Handle psionic training
		if (base->getAvailablePsiLabs() > 0 && Options::anytimePsiTraining)
		{
			std::vector<Soldier*> psiTrainingFinishedList;
			for (std::vector<Soldier*>::const_iterator s = base->getSoldiers()->begin(); s != base->getSoldiers()->end(); ++s)
			{
				(*s)->trainPsi1Day();
				(*s)->calcStatString(_game->getMod()->getStatStrings(), psiStrengthEval);
				if ((*s)->isInPsiTraining() && (*s)->isFullyPsiTrained())
				{
					(*s)->setPsiTraining(false);
					psiTrainingFinishedList.push_back(*s);
				}
			}
			if (!psiTrainingFinishedList.empty())
			{
				popup(new TrainingFinishedState(base, psiTrainingFinishedList, true));
			}
		}
	}

	// check and remove disabled projects from ongoing research
	for (std::vector<Base*>::iterator i = _game->getSavedGame()->getBases()->begin(); i != _game->getSavedGame()->getBases()->end(); ++i)
	{
		std::vector<ResearchProject*> obsolete;
		for (std::vector<ResearchProject*>::const_iterator iter = (*i)->getResearch().begin(); iter != (*i)->getResearch().end(); ++iter)
		{
			if (_game->getSavedGame()->isResearchRuleStatusDisabled((*iter)->getRules()->getName()))
			{
				obsolete.push_back(*iter);
			}
		}
		for (std::vector<ResearchProject*>::const_iterator iter = obsolete.begin(); iter != obsolete.end(); ++iter)
		{
			(*i)->removeResearch(*iter);
		}
	}

	// check and interrupt alien missions if necessary (based on discovered research)
	for (auto am : saveGame->getAlienMissions())
	{
		auto researchName = am->getRules().getInterruptResearch();
		if (!researchName.empty())
		{
			auto research = mod->getResearch(researchName, true);
			if (saveGame->isResearched(research, false)) // ignore debug mode
			{
				am->setInterrupted(true);
			}
		}
	}

	// check and self-destruct alien bases if necessary (based on discovered research)
	std::vector<AlienBase*>::iterator ab = saveGame->getAlienBases()->begin();
	while (ab != saveGame->getAlienBases()->end())
	{
		auto selfDestructCode = (*ab)->getDeployment()->getBaseSelfDestructCode();
		if (!selfDestructCode.empty())
		{
			auto research = mod->getResearch(selfDestructCode, true);
			if (saveGame->isResearched(research, false)) // ignore debug mode
			{
				saveGame->clearLinksForAlienBase((*ab), _game->getMod());
				delete (*ab);
				ab = saveGame->getAlienBases()->erase(ab);
			}
			else
			{
				++ab;
			}
		}
		else
		{
			++ab;
		}
	}

	// handle regional and country points for alien bases
	for (std::vector<AlienBase*>::const_iterator b = saveGame->getAlienBases()->begin(); b != saveGame->getAlienBases()->end(); ++b)
	{
		for (std::vector<Region*>::iterator k = saveGame->getRegions()->begin(); k != saveGame->getRegions()->end(); ++k)
		{
			if ((*k)->getRules()->insideRegion((*b)->getLongitude(), (*b)->getLatitude()))
			{
				(*k)->addActivityAlien((*b)->getDeployment()->getPoints());
				break;
			}
		}
		for (std::vector<Country*>::iterator k = saveGame->getCountries()->begin(); k != saveGame->getCountries()->end(); ++k)
		{
			if ((*k)->getRules()->insideCountry((*b)->getLongitude(), (*b)->getLatitude()))
			{
				(*k)->addActivityAlien((*b)->getDeployment()->getPoints());
				break;
			}
		}
	}

	// Handle resupply of alien bases.
	std::for_each(saveGame->getAlienBases()->begin(), saveGame->getAlienBases()->end(),
			  GenerateSupplyMission(*_game, *_globe));

	// Handle alien base detection (by xcom base facilities).
	for (auto alienBase : *_game->getSavedGame()->getAlienBases())
	{
		if (alienBase->isDiscovered()) continue;
		for (auto xcomBase : *_game->getSavedGame()->getBases())
		{
			int distance = XcomDistance(xcomBase->getDistance(alienBase));
			for (auto facility : *xcomBase->getFacilities())
			{
				if (facility->getBuildTime() == 0 && facility->getRules()->getSightRange() > distance)
				{
					int chanceToDetect = facility->getRules()->getSightChance(); // static % defined by the modder
					if (chanceToDetect == 0)
					{
						chanceToDetect = 50 - (distance * 50 / facility->getRules()->getSightRange()); // dynamic 0-50% based on relative distance
					}
					if (RNG::percent(chanceToDetect))
					{
						alienBase->setDiscovered(true);
					}
				}
			}
		}
	}

	// clear the daily dogfight experience cache
	for (auto* base : *saveGame->getBases())
	{
		for (auto* soldier : *base->getSoldiers())
		{
			soldier->resetDailyDogfightExperienceCache();
		}
	}

	// Autosave 3 times a month
	int day = saveGame->getTime()->getDay();
	if (day == 10 || day == 20)
	{
		if (saveGame->isIronman())
		{
			popup(new SaveGameState(OPT_GEOSCAPE, SAVE_IRONMAN, _palette));
		}
		else if (Options::autosave)
		{
			popup(new SaveGameState(OPT_GEOSCAPE, SAVE_AUTO_GEOSCAPE, _palette));
		}
	}
	else if (saveGame->getEnding() != END_NONE && saveGame->isIronman())
	{
		_game->pushState(new SaveGameState(OPT_GEOSCAPE, SAVE_IRONMAN, _palette));
	}

	// pay attention to your maintenance player!
	if (_game->getSavedGame()->getTime()->isLastDayOfMonth())
	{
		int month = _game->getSavedGame()->getMonthsPassed();
		int currentScore = _game->getSavedGame()->getCurrentScore(month + 1);
		int performanceBonus = currentScore * mod->getPerformanceBonusFactor();
		if (performanceBonus < 0)
		{
			performanceBonus = 0; // bonus only, no malus
		}

		int64_t funds = _game->getSavedGame()->getFunds();
		int64_t income = _game->getSavedGame()->getCountryFunding() + performanceBonus;
		int64_t maintenance = _game->getSavedGame()->getBaseMaintenance();
		int64_t projection = funds + income - maintenance;
		if (projection < 0)
		{
			projection = std::abs(projection);
			projection = ((projection / 100000) + 1) * 100000; // round up to 100k
			std::string msg = tr("STR_ECONOMY_WARNING")
				.arg(Unicode::formatFunding(funds))
				.arg(Unicode::formatFunding(income))
				.arg(Unicode::formatFunding(maintenance))
				.arg(Unicode::formatFunding(projection));
			popup(new CraftErrorState(this, msg, false));
		}
	}
}

/**
 * Takes care of any game logic that has to
 * run every game month, like funding.
 */
void GeoscapeState::time1Month()
{
	_game->getSavedGame()->addMonth();

	// Determine alien mission for this month.
	determineAlienMissions();

	// Handle Psi-Training and initiate a new retaliation mission, if applicable
	if (!Options::anytimePsiTraining)
	{
		bool psiStrengthEval = (Options::psiStrengthEval && _game->getSavedGame()->isResearched(_game->getMod()->getPsiRequirements()));
		for (std::vector<Base*>::const_iterator b = _game->getSavedGame()->getBases()->begin(); b != _game->getSavedGame()->getBases()->end(); ++b)
		{
			if ((*b)->getAvailablePsiLabs() > 0)
			{
				for (std::vector<Soldier*>::const_iterator s = (*b)->getSoldiers()->begin(); s != (*b)->getSoldiers()->end(); ++s)
				{
					if ((*s)->isInPsiTraining())
					{
						(*s)->trainPsi();
						(*s)->calcStatString(_game->getMod()->getStatStrings(), psiStrengthEval);
					}
				}
			}
		}
	}

	// Handle funding
	timerReset();
	_game->getSavedGame()->monthlyFunding();
	popup(new MonthlyReportState(_globe));

	// Handle Xcom Operatives discovering bases
	if (!_game->getSavedGame()->getAlienBases()->empty() && RNG::percent(20))
	{
		for (std::vector<AlienBase*>::const_iterator b = _game->getSavedGame()->getAlienBases()->begin(); b != _game->getSavedGame()->getAlienBases()->end(); ++b)
		{
			if (!(*b)->isDiscovered())
			{
				(*b)->setDiscovered(true);
				popup(new AlienBaseState(*b, this));
				break;
			}
		}
	}
}

/**
 * Slows down the timer back to minimum speed,
 * for when important events occur.
 */
void GeoscapeState::timerReset()
{
	SDL_Event ev;
	ev.button.button = SDL_BUTTON_LEFT;
	Action act(&ev, _game->getScreen()->getXScale(), _game->getScreen()->getYScale(), _game->getScreen()->getCursorTopBlackBand(), _game->getScreen()->getCursorLeftBlackBand());
	_btn5Secs->mousePress(&act, this);
}

/**
 * Adds a new popup window to the queue
 * (this prevents popups from overlapping)
 * and pauses the game timer respectively.
 * @param state Pointer to popup state.
 */
void GeoscapeState::popup(State *state)
{
	_pause = true;
	_popups.push_back(state);
}

/**
 * Returns a pointer to the Geoscape globe for
 * access by other substates.
 * @return Pointer to globe.
 */
Globe *GeoscapeState::getGlobe() const
{
	return _globe;
}

/**
 * Processes any left-clicks on globe markers,
 * or right-clicks to scroll the globe.
 * @param action Pointer to an action.
 */

void GeoscapeState::globeClick(Action *action)
{
	int mouseX = (int)floor(action->getAbsoluteXMouse()), mouseY = (int)floor(action->getAbsoluteYMouse());

	// Clicking markers on the globe
	if (action->getDetails()->button.button == SDL_BUTTON_LEFT)
	{
		std::vector<Target*> v = _globe->getTargets(mouseX, mouseY, false, 0);
		if (!v.empty())
		{
			_game->pushState(new MultipleTargetsState(v, 0, this));
		}
	}

	if (_game->getSavedGame()->getDebugMode())
	{
		double lon, lat;
		int texture, shade;
		_globe->cartToPolar(mouseX, mouseY, &lon, &lat);
		double lonDeg = lon / M_PI * 180, latDeg = lat / M_PI * 180;
		_globe->getPolygonTextureAndShade(lon, lat, &texture, &shade);
		std::ostringstream ss;
		ss << "rad: " << lon << ", " << lat << std::endl;
		ss << "deg: " << lonDeg << ", " << latDeg << std::endl;
		ss << "texture: " << texture << ", shade: " << shade << std::endl;

		_txtDebug->setText(ss.str());
	}
}

/**
 * Opens the Intercept window.
 * @param action Pointer to an action.
 */
void GeoscapeState::btnInterceptClick(Action *)
{
	if (buttonsDisabled())
	{
		return;
	}
	_game->pushState(new InterceptState(_globe));
}

/**
* Opens the UFO Tracker window.
* @param action Pointer to an action.
*/
void GeoscapeState::btnUfoTrackerClick(Action *)
{
	_game->pushState(new UfoTrackerState(this, _globe));
}

/**
* Opens the TechTreeViewer window.
* @param action Pointer to an action.
*/
void GeoscapeState::btnTechTreeViewerClick(Action *)
{
	_game->pushState(new TechTreeViewerState());
}

/**
 * Opens the jukebox.
 * @param action Pointer to an action.
 */
void GeoscapeState::btnSelectMusicTrackClick(Action *)
{
	_game->pushState(new SelectMusicTrackState(SMT_GEOSCAPE));
}

/**
 * Opens the Current Global Production.
 * @param action Pointer to an action.
 */
void GeoscapeState::btnGlobalProductionClick(Action *)
{
	_game->pushState(new GlobalManufactureState(false));
}

/**
 * Opens the Current Global Research.
 * @param action Pointer to an action.
 */
void GeoscapeState::btnGlobalResearchClick(Action *)
{
	_game->pushState(new GlobalResearchState(false));
}

/**
 * Opens the Dogfight Experience screen.
 * @param action Pointer to an action.
 */
void GeoscapeState::btnDogfightExperienceClick(Action *)
{
	_game->pushState(new DogfightExperienceState());
}

/**
 * Goes to the Basescape screen.
 * @param action Pointer to an action.
 */
void GeoscapeState::btnBasesClick(Action *)
{
	if (buttonsDisabled())
	{
		return;
	}
	timerReset();
	if (!_game->getSavedGame()->getBases()->empty())
	{
		_game->pushState(new BasescapeState(_game->getSavedGame()->getSelectedBase(), _globe));
	}
	else
	{
		_game->pushState(new BasescapeState(0, _globe));
	}
}

/**
 * Goes to the Graphs screen.
 * @param action Pointer to an action.
 */
void GeoscapeState::btnGraphsClick(Action *)
{
	if (buttonsDisabled())
	{
		return;
	}
	_game->pushState(new GraphsState);
}

/**
 * Goes to the Ufopaedia window.
 * @param action Pointer to an action.
 */
void GeoscapeState::btnUfopaediaClick(Action *)
{
	if (buttonsDisabled())
	{
		return;
	}
	Ufopaedia::open(_game);
}

/**
 * Opens the Options window.
 * @param action Pointer to an action.
 */
void GeoscapeState::btnOptionsClick(Action *)
{
	if (buttonsDisabled())
	{
		return;
	}
	_game->pushState(new PauseState(OPT_GEOSCAPE));
}

/**
 * Goes to the Funding screen.
 * @param action Pointer to an action.
 */
void GeoscapeState::btnFundingClick(Action *)
{
	if (buttonsDisabled())
	{
		return;
	}
	_game->pushState(new FundingState);
}

/**
 * Starts rotating the globe to the left.
 * @param action Pointer to an action.
 */
void GeoscapeState::btnRotateLeftPress(Action *)
{
	_globe->rotateLeft();
}

/**
 * Stops rotating the globe to the left.
 * @param action Pointer to an action.
 */
void GeoscapeState::btnRotateLeftRelease(Action *)
{
	_globe->rotateStopLon();
}

/**
 * Starts rotating the globe to the right.
 * @param action Pointer to an action.
 */
void GeoscapeState::btnRotateRightPress(Action *)
{
	_globe->rotateRight();
}

/**
 * Stops rotating the globe to the right.
 * @param action Pointer to an action.
 */
void GeoscapeState::btnRotateRightRelease(Action *)
{
	_globe->rotateStopLon();
}

/**
 * Starts rotating the globe upwards.
 * @param action Pointer to an action.
 */
void GeoscapeState::btnRotateUpPress(Action *)
{
	_globe->rotateUp();
}

/**
 * Stops rotating the globe upwards.
 * @param action Pointer to an action.
 */
void GeoscapeState::btnRotateUpRelease(Action *)
{
	_globe->rotateStopLat();
}

/**
 * Starts rotating the globe downwards.
 * @param action Pointer to an action.
 */
void GeoscapeState::btnRotateDownPress(Action *)
{
	_globe->rotateDown();
}

/**
 * Stops rotating the globe downwards.
 * @param action Pointer to an action.
 */
void GeoscapeState::btnRotateDownRelease(Action *)
{
	_globe->rotateStopLat();
}

/**
 * Zooms into the globe.
 * @param action Pointer to an action.
 */
void GeoscapeState::btnZoomInLeftClick(Action *)
{
	_globe->zoomIn();
}

/**
 * Zooms the globe maximum.
 * @param action Pointer to an action.
 */
void GeoscapeState::btnZoomInRightClick(Action *)
{
	_globe->zoomMax();
}

/**
 * Zooms out of the globe.
 * @param action Pointer to an action.
 */
void GeoscapeState::btnZoomOutLeftClick(Action *)
{
	_globe->zoomOut();
}

/**
 * Zooms the globe minimum.
 * @param action Pointer to an action.
 */
void GeoscapeState::btnZoomOutRightClick(Action *)
{
	_globe->zoomMin();
}

/**
 * Zoom in effect for dogfights.
 */
void GeoscapeState::zoomInEffect()
{
	if (_globe->zoomDogfightIn())
	{
		_zoomInEffectDone = true;
		_zoomInEffectTimer->stop();
	}
}

/**
 * Zoom out effect for dogfights.
 */
void GeoscapeState::zoomOutEffect()
{
	if (_globe->zoomDogfightOut())
	{
		_zoomOutEffectDone = true;
		_zoomOutEffectTimer->stop();
		init();
	}
}

/**
 * Dogfight logic. Moved here to have the code clean.
 */
void GeoscapeState::handleDogfights()
{
	// Handle dogfights logic.
	_minimizedDogfights = 0;

	std::list<DogfightState*>::iterator d = _dogfights.begin();
	for (; d != _dogfights.end(); ++d)
	{
		(*d)->getUfo()->setInterceptionProcessed(false);
	}
	d = _dogfights.begin();
	while (d != _dogfights.end())
	{
		if ((*d)->isMinimized())
		{
			if ((*d)->getWaitForPoly() && _globe->insideLand((*d)->getUfo()->getLongitude(), (*d)->getUfo()->getLatitude()))
			{
				(*d)->setMinimized(false);
				(*d)->setWaitForPoly(false);
			}
			else if ((*d)->getWaitForAltitude() && (*d)->getUfo()->getAltitudeInt() <= (*d)->getCraft()->getRules()->getMaxAltitude())
			{
				(*d)->setMinimized(false);
				(*d)->setWaitForAltitude(false);
			}
			else
			{
				_minimizedDogfights++;
			}
		}
		else
		{
			_globe->rotateStop();
		}
		(*d)->think();
		if ((*d)->dogfightEnded())
		{
			if ((*d)->isMinimized())
			{
				_minimizedDogfights--;
			}
			delete *d;
			d = _dogfights.erase(d);
		}
		else
		{
			++d;
		}
	}
	if (_dogfights.empty())
	{
		_dogfightTimer->stop();
		_zoomOutEffectTimer->start();
	}
}

/**
 * Goes through all active dogfight instances and tries to perform the same action.
 * @param button Action to perform.
 */
void GeoscapeState::handleDogfightMultiAction(int button)
{
	for (auto d : _dogfights)
	{
		if (d->isMinimized())
			continue;

		SDL_Event ev;
		ev.type = SDL_MOUSEBUTTONDOWN;
		ev.button.button = SDL_BUTTON_LEFT;
		Action a = Action(&ev, 0.0, 0.0, 0, 0);

		switch (button)
		{
		case 0:
			d->btnStandoffSimulateLeftPress(&a);
			break;
		case 1:
			d->btnCautiousSimulateLeftPress(&a);
			break;
		case 2:
			d->btnStandardSimulateLeftPress(&a);
			break;
		case 3:
			d->btnAggressiveSimulateLeftPress(&a);
			break;
		case 4:
			d->btnDisengageSimulateLeftPress(&a);
			break;
		default:
			break;
		}
	}
}

/**
 * Goes through all dogfight instances and tries to award pilot experience.
 * This is called each time any UFO takes any damage in dogfight... very ugly, but I couldn't find a better place for it.
 *
 * History lesson:
 * - this was in the DogfightState destructor before, but could lead to CTD when people reloaded a saved game while dogfights were still active
 * - both OXCE and vanilla OXC still access already deleted objects (_craft and _ufo) in the destructor... that doesn't seem to trigger CTD that often though
 */
void GeoscapeState::handleDogfightExperience()
{
	for (auto d : _dogfights)
	{
		d->awardExperienceToPilots();
	}
	for (auto ds : _dogfightsToBeStarted)
	{
		ds->awardExperienceToPilots();
	}
}

/**
 * Gets the number of minimized dogfights.
 * @return Number of minimized dogfights.
 */
int GeoscapeState::minimizedDogfightsCount()
{
	int minimizedDogfights = 0;
	for (std::list<DogfightState*>::iterator d = _dogfights.begin(); d != _dogfights.end(); ++d)
	{
		if ((*d)->isMinimized())
		{
			++minimizedDogfights;
		}
	}
	return minimizedDogfights;
}

/**
 * Starts a new dogfight.
 */
void GeoscapeState::startDogfight()
{
	if (_globe->getZoom() < 3)
	{
		if (!_zoomInEffectTimer->isRunning())
		{
			_globe->saveZoomDogfight();
			_globe->rotateStop();
			_zoomInEffectTimer->start();
		}
	}
	else
	{
		_dogfightStartTimer->stop();
		_zoomInEffectTimer->stop();
		_dogfightTimer->start();
		timerReset();
		while (!_dogfightsToBeStarted.empty())
		{
			_dogfights.push_back(_dogfightsToBeStarted.back());
			_dogfightsToBeStarted.pop_back();
			_dogfights.back()->setInterceptionNumber(getFirstFreeDogfightSlot());
			_dogfights.back()->setInterceptionsCount(_dogfights.size() + _dogfightsToBeStarted.size());
		}
		// Set correct number of interceptions for every dogfight.
		for (std::list<DogfightState*>::iterator d = _dogfights.begin(); d != _dogfights.end(); ++d)
		{
			(*d)->setInterceptionsCount(_dogfights.size());
		}
	}
}

/**
 * Returns the first free dogfight slot.
 * @return free slot
 */
int GeoscapeState::getFirstFreeDogfightSlot()
{
	int slotNo = 1;
	for (std::list<DogfightState*>::iterator d = _dogfights.begin(); d != _dogfights.end(); ++d)
	{
		if ((*d)->getInterceptionNumber() == slotNo)
		{
			++slotNo;
		}
	}
	return slotNo;
}

/**
 * Handle base defense
 * @param base Base to defend.
 * @param ufo Ufo attacking base.
 */
void GeoscapeState::handleBaseDefense(Base *base, Ufo *ufo)
{
	// Get the shade and texture for the globe at the location of the base, using the ufo position
	int texture, shade;
	double baseLon = ufo->getLongitude();
	double baseLat = ufo->getLatitude();
	_globe->getPolygonTextureAndShade(baseLon, baseLat, &texture, &shade);

	int ufoDamagePercentage = 0;
	if (_game->getMod()->getLessAliensDuringBaseDefense())
	{
		ufoDamagePercentage = ufo->getDamagePercentage();
	}

	// Whatever happens in the base defense, the UFO has finished its duty
	ufo->setStatus(Ufo::DESTROYED);

	if (ufo->getRules()->getMissilePower() != 0)
	{
		if (ufo->getRules()->getMissilePower() < 0)
		{
			// It's a nuclear warhead... Skynet knows no mercy
			popup(new BaseDestroyedState(base, true, false));
		}
		else
		{
			// This is an overkill, since we may not lose any hangar/craft, but doing it properly requires tons of changes
			_game->getSavedGame()->stopHuntingXcomCrafts(base);

			// This can either damage facilities (=replace them with some other facilities); or also destroy facilities
			base->damageFacilities(ufo);

			// don't forget to reset pre-cached stuff
			base->cleanupDefenses(true);

			// let the player know that some facilities were destroyed, but the base survived
			popup(new BaseDestroyedState(base, true, true));
		}
	}
	else if (base->getAvailableSoldiers(true, true) > 0 || !base->getVehicles()->empty())
	{
		SavedBattleGame *bgame = new SavedBattleGame(_game->getMod(), _game->getLanguage());
		_game->getSavedGame()->setBattleGame(bgame);
		bgame->setMissionType("STR_BASE_DEFENSE");
		BattlescapeGenerator bgen = BattlescapeGenerator(_game);
		bgen.setBase(base);
		bgen.setAlienCustomDeploy(_game->getMod()->getDeployment(ufo->getCraftStats().missionCustomDeploy));
		bgen.setAlienRace(ufo->getAlienRace());
		bgen.setWorldShade(shade);
		auto globeTexture = _game->getMod()->getGlobe()->getTexture(texture);
		bgen.setWorldTexture(globeTexture, globeTexture);
		bgen.setUfoDamagePercentage(ufoDamagePercentage);
		bgen.run();
		_pause = true;
		_game->pushState(new BriefingState(0, base));
	}
	else
	{
		// Please garrison your bases in future
		popup(new BaseDestroyedState(base, false, false));
	}
}

/**
 * Determine the alien missions to start this month.
 */
void GeoscapeState::determineAlienMissions()
{
	SavedGame *save = _game->getSavedGame();
	AlienStrategy &strategy = save->getAlienStrategy();
	Mod *mod = _game->getMod();
	int month = _game->getSavedGame()->getMonthsPassed();
	int currentScore = save->getCurrentScore(month); // _monthsPassed was already increased by 1
	int performanceBonus = currentScore * mod->getPerformanceBonusFactor();
	if (performanceBonus < 0)
	{
		performanceBonus = 0; // bonus only, no malus
	}
	int64_t currentFunds = save->getFunds();
	currentFunds += save->getCountryFunding() + performanceBonus - save->getBaseMaintenance(); // peek into the next month
	std::vector<RuleMissionScript*> availableMissions;
	std::map<int, bool> conditions;

	// sorry to interrupt, but before we start determining the actual monthly missions, let's determine and/or adjust our overall game plan
	{
		std::vector<RuleArcScript*> relevantArcScripts;

		// first we need to build a list of "valid" commands
		for (auto& scriptName : *mod->getArcScriptList())
		{
			RuleArcScript* arcScript = mod->getArcScript(scriptName);

			// level one condition check: make sure we're within our time constraints
			if (arcScript->getFirstMonth() <= month &&
				(arcScript->getLastMonth() >= month || arcScript->getLastMonth() == -1) &&
				// and make sure we satisfy the difficulty restrictions
				(month < 1 || arcScript->getMinScore() <= currentScore) &&
				(month < 1 || arcScript->getMaxScore() >= currentScore) &&
				(month < 1 || arcScript->getMinFunds() <= currentFunds) &&
				(month < 1 || arcScript->getMaxFunds() >= currentFunds) &&
				arcScript->getMinDifficulty() <= save->getDifficulty() &&
				arcScript->getMaxDifficulty() >= save->getDifficulty())
			{
				// level two condition check: make sure we meet any research requirements, if any.
				bool triggerHappy = true;
				for (auto& trigger : arcScript->getResearchTriggers())
				{
					triggerHappy = (save->isResearched(trigger.first) == trigger.second);
					if (!triggerHappy)
						break;
				}
				if (triggerHappy)
				{
					// item requirements
					for (auto &triggerItem : arcScript->getItemTriggers())
					{
						triggerHappy = (save->isItemObtained(triggerItem.first) == triggerItem.second);
						if (!triggerHappy)
							break;
					}
				}
				if (triggerHappy)
				{
					// facility requirements
					for (auto &triggerFacility : arcScript->getFacilityTriggers())
					{
						triggerHappy = (save->isFacilityBuilt(triggerFacility.first) == triggerFacility.second);
						if (!triggerHappy)
							break;
					}
				}
				// level three condition check: does random chance favour this command's execution?
				if (triggerHappy && RNG::percent(arcScript->getExecutionOdds()))
				{
					relevantArcScripts.push_back(arcScript);
				}
			}
		}

		// start processing command array
		for (auto& arcCommand : relevantArcScripts)
		{
			// to remember stuff we can still enable
			std::vector<std::string> disabledSeqArcs;
			WeightedOptions disabledRngArcs;

			int arcsEnabled = 0;
			// level four condition check: check maxArcs (duplicates count, arcs enabled by other commands or in any other way count too!)
			{
				for (auto& seqArc : arcCommand->getSequentialArcs())
				{
					if (save->isResearched(seqArc))
						++arcsEnabled;
					else
						disabledSeqArcs.push_back(seqArc);
				}
				WeightedOptions tmp = arcCommand->getRandomArcs(); // copy for the iterator, because of getNames()
				disabledRngArcs = tmp; // copy for us to modify
				for (auto& rngArc : tmp.getNames())
				{
					if (save->isResearched(rngArc))
					{
						++arcsEnabled;
						disabledRngArcs.set(rngArc, 0); // delete
					}
				}
			}
			Base* hq = save->getBases()->front();
			bool canAddOneMore = arcCommand->getMaxArcs() == -1 || arcCommand->getMaxArcs() > arcsEnabled;
			if (canAddOneMore && !disabledSeqArcs.empty())
			{
				auto ruleResearchSeq = mod->getResearch(disabledSeqArcs.front(), true); // take first
				save->addFinishedResearch(ruleResearchSeq, mod, hq, true);
				++arcsEnabled;
				if (ruleResearchSeq)
				{
					if (ruleResearchSeq->getLookup().empty())
					{
						Ufopaedia::openArticle(_game, ruleResearchSeq->getName());
					}
					else
					{
						save->addFinishedResearch(mod->getResearch(ruleResearchSeq->getLookup(), true), mod, hq, true);
						Ufopaedia::openArticle(_game, ruleResearchSeq->getLookup());
					}
				}
			}
			canAddOneMore = arcCommand->getMaxArcs() == -1 || arcCommand->getMaxArcs() > arcsEnabled;
			if (canAddOneMore && !disabledRngArcs.empty())
			{
				auto ruleResearchRng = mod->getResearch(disabledRngArcs.choose(), true); // take random
				save->addFinishedResearch(ruleResearchRng, mod, hq, true);
				++arcsEnabled; // for good measure :)
				if (ruleResearchRng)
				{
					if (ruleResearchRng->getLookup().empty())
					{
						Ufopaedia::openArticle(_game, ruleResearchRng->getName());
					}
					else
					{
						save->addFinishedResearch(mod->getResearch(ruleResearchRng->getLookup(), true), mod, hq, true);
						Ufopaedia::openArticle(_game, ruleResearchRng->getLookup());
					}
				}
			}
		}
	}

	// well, here it is, ladies and gents, the nuts and bolts behind the geoscape mission scheduling.

	// first we need to build a list of "valid" commands
	for (std::vector<std::string>::const_iterator i = mod->getMissionScriptList()->begin(); i != mod->getMissionScriptList()->end(); ++i)
	{
		RuleMissionScript *command = mod->getMissionScript(*i);

			// level one condition check: make sure we're within our time constraints
		if (command->getFirstMonth() <= month &&
			(command->getLastMonth() >= month || command->getLastMonth() == -1) &&
			// make sure we haven't hit our run limit, if we have one
			(command->getMaxRuns() == -1 ||	command->getMaxRuns() > strategy.getMissionsRun(command->getVarName())) &&
			// and make sure we satisfy the difficulty restrictions
			(month < 1 || command->getMinScore() <= currentScore) &&
			(month < 1 || command->getMaxScore() >= currentScore) &&
			(month < 1 || command->getMinFunds() <= currentFunds) &&
			(month < 1 || command->getMaxFunds() >= currentFunds) &&
			command->getMinDifficulty() <= save->getDifficulty())
		{
			// level two condition check: make sure we meet any research requirements, if any.
			bool triggerHappy = true;
			for (std::map<std::string, bool>::const_iterator j = command->getResearchTriggers().begin(); triggerHappy && j != command->getResearchTriggers().end(); ++j)
			{
				triggerHappy = (save->isResearched(j->first) == j->second);
			}
			if (triggerHappy)
			{
				// item requirements
				for (auto &triggerItem : command->getItemTriggers())
				{
					triggerHappy = (save->isItemObtained(triggerItem.first) == triggerItem.second);
					if (!triggerHappy)
						break;
				}
			}
			if (triggerHappy)
			{
				// facility requirements
				for (auto &triggerFacility : command->getFacilityTriggers())
				{
					triggerHappy = (save->isFacilityBuilt(triggerFacility.first) == triggerFacility.second);
					if (!triggerHappy)
						break;
				}
			}
			// levels one and two passed: insert this command into the array.
			if (triggerHappy)
			{
				availableMissions.push_back(command);
			}
		}
	}

	// start processing command array.
	for (std::vector<RuleMissionScript*>::const_iterator i = availableMissions.begin(); i != availableMissions.end(); ++i)
	{
		RuleMissionScript *command = *i;
		bool process = true;
		bool success = false;
		// level three condition check: make sure our conditionals are met, if any. this list is dynamic, and must be checked here.
		for (std::vector<int>::const_iterator j = command->getConditionals().begin(); process && j != command->getConditionals().end(); ++j)
		{
			std::map<int, bool>::const_iterator found = conditions.find(std::abs(*j));
			// just an FYI: if you add a 0 to your conditionals, this flag will never resolve to true, and your command will never run.
			process = (found == conditions.end() || (found->second == true && *j > 0) || (found->second == false && *j < 0));
		}
		if (command->getLabel() > 0 && conditions.find(command->getLabel()) != conditions.end())
		{
			std::ostringstream ss;
			ss << "Mission generator encountered an error: multiple commands: " << command->getType() << " and ";
			for (std::vector<RuleMissionScript*>::const_iterator j = availableMissions.begin(); j != availableMissions.end(); ++j)
			{
				if (command->getLabel() == (*j)->getLabel() && (*j) != (*i))
				{
					ss << (*j)->getType() << ", ";
				}
			}
			ss  << "are sharing the same label: " << command->getLabel();
			throw Exception(ss.str());
		}
		// level four condition check: does random chance favour this command's execution?
		if (process && RNG::percent(command->getExecutionOdds()))
		{
			// good news, little command pointer! you're FDA approved! off to the main processing facility with you!
			success = processCommand(command);
		}
		if (command->getLabel() > 0)
		{
			// tsk, tsk. you really should be careful with these unique labels, they're supposed to be unique.
			if (conditions.find(command->getLabel()) != conditions.end())
			{
				throw Exception("Error in mission scripts: " + command->getType() + ". Two or more commands sharing the same label. That's bad, Mmmkay?");
			}
			// keep track of what happened to this command, so others may reference it.
			conditions[command->getLabel()] = success;
		}
	}

	// after the mission scripts, it's time for the event scripts
	{
		std::vector<RuleEventScript *> relevantEventScripts;

		// first we need to build a list of "valid" commands
		for (auto& scriptName : *mod->getEventScriptList())
		{
			RuleEventScript *eventScript = mod->getEventScript(scriptName);

			// level one condition check: make sure we're within our time constraints
			if (eventScript->getFirstMonth() <= month &&
				(eventScript->getLastMonth() >= month || eventScript->getLastMonth() == -1) &&
				// and make sure we satisfy the difficulty restrictions
				(month < 1 || eventScript->getMinScore() <= currentScore) &&
				(month < 1 || eventScript->getMaxScore() >= currentScore) &&
				(month < 1 || eventScript->getMinFunds() <= currentFunds) &&
				(month < 1 || eventScript->getMaxFunds() >= currentFunds) &&
				eventScript->getMinDifficulty() <= save->getDifficulty() &&
				eventScript->getMaxDifficulty() >= save->getDifficulty())
			{
				// level two condition check: make sure we meet any research requirements, if any.
				bool triggerHappy = true;
				for (auto& trigger : eventScript->getResearchTriggers())
				{
					triggerHappy = (save->isResearched(trigger.first) == trigger.second);
					if (!triggerHappy)
						break;
				}
				if (triggerHappy)
				{
					// item requirements
					for (auto &triggerItem : eventScript->getItemTriggers())
					{
						triggerHappy = (save->isItemObtained(triggerItem.first) == triggerItem.second);
						if (!triggerHappy)
							break;
					}
				}
				if (triggerHappy)
				{
					// facility requirements
					for (auto &triggerFacility : eventScript->getFacilityTriggers())
					{
						triggerHappy = (save->isFacilityBuilt(triggerFacility.first) == triggerFacility.second);
						if (!triggerHappy)
							break;
					}
				}
				// level three condition check: does random chance favour this command's execution?
				if (triggerHappy && RNG::percent(eventScript->getExecutionOdds()))
				{
					relevantEventScripts.push_back(eventScript);
				}
			}
		}

		// now, let's process the relevant event scripts
		for (auto& eventCommand : relevantEventScripts)
		{
			std::vector<const RuleEvent*> toBeGenerated;

			// 1. sequentially generated one-time events (cannot repeat)
			{
				std::vector<std::string> possibleSeqEvents;
				for (auto& seqEvent : eventCommand->getOneTimeSequentialEvents())
				{
					if (!save->wasEventGenerated(seqEvent))
						possibleSeqEvents.push_back(seqEvent); // insert
				}
				if (!possibleSeqEvents.empty())
				{
					auto eventRules = mod->getEvent(possibleSeqEvents.front(), true); // take first
					toBeGenerated.push_back(eventRules);
				}
			}

			// 2. randomly generated one-time events (cannot repeat)
			{
				WeightedOptions possibleRngEvents;
				WeightedOptions tmp = eventCommand->getOneTimeRandomEvents(); // copy for the iterator, because of getNames()
				possibleRngEvents = tmp; // copy for us to modify
				for (auto& rngEvent : tmp.getNames())
				{
					if (save->wasEventGenerated(rngEvent))
						possibleRngEvents.set(rngEvent, 0); // delete
				}
				if (!possibleRngEvents.empty())
				{
					auto eventRules = mod->getEvent(possibleRngEvents.choose(), true); // take random
					toBeGenerated.push_back(eventRules);
				}
			}

			// 3. randomly generated repeatable events
			{
				auto eventRules = mod->getEvent(eventCommand->generate(save->getMonthsPassed()), false);
				if (eventRules)
				{
					toBeGenerated.push_back(eventRules);
				}
			}

			// 4. generate
			for (auto eventRules : toBeGenerated)
			{
				GeoscapeEvent *newEvent = new GeoscapeEvent(*eventRules);
				int minutes = (eventRules->getTimer() + (RNG::generate(0, eventRules->getTimerRandom()))) / 30 * 30;
				if (minutes < 60) minutes = 60; // just in case
				newEvent->setSpawnCountdown(minutes);
				_game->getSavedGame()->getGeoscapeEvents().push_back(newEvent);

				// remember that it has been generated
				save->addGeneratedEvent(eventRules);
			}
		}
	}

	// Alien base upgrades happen only AFTER the first game month
	if (month > 0)
	{
		for (auto alienBase : *save->getAlienBases())
		{
			auto baseAgeInMonths = month - alienBase->getStartMonth();
			auto upgradeId = alienBase->getDeployment()->generateAlienBaseUpgrade(baseAgeInMonths);
			auto upgrade = mod->getDeployment(upgradeId, false);
			if (upgrade && upgrade != alienBase->getDeployment())
			{
				alienBase->setDeployment(upgrade);
			}
		}
	}
}


/**
 * Processes a directive to start up a mission, if possible.
 * @param command the directive from which to read information.
 * @return whether the command successfully produced a new mission.
 */
bool GeoscapeState::processCommand(RuleMissionScript *command)
{
	SavedGame *save = _game->getSavedGame();
	AlienStrategy &strategy = save->getAlienStrategy();
	Mod *mod = _game->getMod();
	int month = _game->getSavedGame()->getMonthsPassed();
	std::string targetRegion;
	const RuleAlienMission *missionRules;
	std::string missionType;
	std::string missionRace;
	int targetZone = -1;

	// terror mission type deal? this will require special handling.
	if (command->getSiteType())
	{
		// we know for a fact that this command has mission weights defined, otherwise this flag could not be set.
		missionType = command->generate(month, GEN_MISSION);
		std::vector<std::string> missions = command->getMissionTypes(month);
		int maxMissions = missions.size();
		bool targetBase = RNG::percent(command->getTargetBaseOdds());
		int currPos = 0;
		for (; currPos != maxMissions; ++currPos)
		{
			if (missions[currPos] == missionType)
			{
				break;
			}
		}

		// let's build a list of regions with spawn zones to pick from
		std::vector<std::pair<std::string, int> > validAreas;

		// this is actually a bit of a cheat, we ARE using the mission weights as defined, but we'll try them all if the one we pick first isn't valid.
		for (int h = 0; h != maxMissions; ++h)
		{
			// we'll use the regions listed in the command, if any, otherwise check all the regions in the ruleset looking for matches
			std::vector<std::string> regions = (command->hasRegionWeights()) ? command->getRegions(month) : mod->getRegionsList();
			missionRules = mod->getAlienMission(missionType, true);
			targetZone = missionRules->getSpawnZone();

			if (targetBase)
			{
				std::vector<std::string> regionsToKeep;
				//if we're targetting a base, we ignore regions that don't contain bases, simple.
				for (std::vector<Base*>::iterator i = save->getBases()->begin(); i != save->getBases()->end(); ++i)
				{
					regionsToKeep.push_back(save->locateRegion((*i)->getLongitude(), (*i)->getLatitude())->getRules()->getType());
				}
				for (std::vector<std::string>::iterator i = regions.begin(); i != regions.end();)
				{
					if (std::find(regionsToKeep.begin(), regionsToKeep.end(), *i) == regionsToKeep.end())
					{
						i = regions.erase(i);
					}
					else
					{
						++i;
					}
				}
			}

			for (std::vector<std::string>::iterator i = regions.begin(); i != regions.end();)
			{
				// we don't want the same mission running in any given region twice simultaneously, so prune the list as needed.
				bool processThisRegion = true;
				for (std::vector<AlienMission*>::const_iterator j = save->getAlienMissions().begin(); j != save->getAlienMissions().end(); ++j)
				{
					if ((*j)->getRules().getType() == missionRules->getType() && (*j)->getRegion() == *i)
					{
						processThisRegion = false;
						break;
					}
				}
				if (!processThisRegion)
				{
					i = regions.erase(i);
					continue;
				}
				// ok, we found a region that doesn't have our mission in it, let's see if it has an appropriate landing zone.
				// if it does, let's add it to our list of valid areas, taking note of which mission area(s) matched.
				RuleRegion *region = mod->getRegion(*i, true);
				if ((int)(region->getMissionZones().size()) > targetZone)
				{
					std::vector<MissionArea> areas = region->getMissionZones()[targetZone].areas;
					int counter = 0;
					for (std::vector<MissionArea>::const_iterator j = areas.begin(); j != areas.end(); ++j)
					{
						// validMissionLocation checks to make sure this city/whatever hasn't been used by the last n missions using this varName
						// this prevents the same location getting hit more than once every n missions.
						if ((*j).isPoint() && strategy.validMissionLocation(command->getVarName(), region->getType(), counter))
						{
							validAreas.push_back(std::make_pair(region->getType(), counter));
						}
						else if (!(*j).isPoint() && (*j).texture < 0)
						{
							validAreas.push_back(std::make_pair(region->getType(), counter));
						}
						counter++;
					}
				}
				++i;
			}

			// oh bother, we couldn't find anything valid, this mission won't run this month.
			if (validAreas.empty())
			{
				if (maxMissions > 1 && ++currPos == maxMissions)
				{
					currPos = 0;
				}
				missionType = missions[currPos];
			}
			else
			{
				break;
			}
		}

		if (validAreas.empty())
		{
			// now we're in real trouble, we've managed to make it out of the loop and we still don't have any valid choices
			// this command cannot run this month, we have failed, forgive us senpai.
			return false;
		}
		// reset this, we may have used it earlier, it longer represents the target zone type, but the target zone number within that type
		targetZone = -1;
		// everything went according to plan: we can now pick a city/whatever to attack.
		while (targetZone == -1)
		{
			if (command->hasRegionWeights())
			{
				// if we have a weighted region list, we know we have at least one valid choice for this mission
				targetRegion = command->generate(month, GEN_REGION);
			}
			else
			{
				// if we don't have a weighted list, we'll select a region at random from the ruleset,
				// validate that it's in our list, and pick one of its cities at random
				// this will give us an even distribution between regions regardless of the number of cities.
				targetRegion = mod->getRegionsList().at(RNG::generate(0, mod->getRegionsList().size() - 1));
			}

			// we need to know the range of the region within our vector, in order to randomly select a city from it
			int min = -1;
			int max = -1;
			int curr = 0;
			for (std::vector<std::pair<std::string, int> >::const_iterator i = validAreas.begin(); i != validAreas.end(); ++i)
			{
				if ((*i).first == targetRegion)
				{
					if (min == -1)
					{
						min = curr;
					}
					max = curr;
				}
				else if (min > -1)
				{
					// if we've stopped detecting matches, we're done looking.
					break;
				}
				++curr;
			}
			if (min != -1)
			{
				// we have our random range, we can make a selection, and we're done.
				targetZone = validAreas[RNG::generate(min, max)].second;
			}
		}
		// now add that city to the list of sites we've hit, store the array, etc.
		strategy.addMissionLocation(command->getVarName(), targetRegion, targetZone, command->getRepeatAvoidance());
	}
	else if (RNG::percent(command->getTargetBaseOdds()))
	{
		// build a list of the mission types we're dealing with, if any
		std::vector<std::string> types = command->getMissionTypes(month);
		// now build a list of regions with bases in.
		std::vector<std::string> regionsMaster;
		for (std::vector<Base*>::const_iterator i = save->getBases()->begin(); i != save->getBases()->end(); ++i)
		{
			regionsMaster.push_back(save->locateRegion(*(*i))->getRules()->getType());
		}
		// no defined mission types? then we'll prune the region list to ensure we only have a region that can generate a mission.
		if (types.empty())
		{
			for (std::vector<std::string>::iterator i = regionsMaster.begin(); i != regionsMaster.end();)
			{
				if (!strategy.validMissionRegion(*i))
				{
					i = regionsMaster.erase(i);
					continue;
				}
				++i;
			}
			// no valid missions in any base regions? oh dear, i guess we failed.
			if (regionsMaster.empty())
			{
				return false;
			}
			// pick a random region from our list
			targetRegion = regionsMaster[RNG::generate(0, regionsMaster.size()-1)];
		}
		else
		{
			// we don't care about regional mission distributions, we're targetting a base with whatever mission we pick, so let's pick now
			// we'll iterate the mission list, starting at a random point, and wrapping around to the beginning
			int max = types.size();
			int entry = RNG::generate(0,  max - 1);
			std::vector<std::string> regions;

			for (int i = 0; i != max; ++i)
			{
				regions = regionsMaster;
				for (std::vector<AlienMission*>::const_iterator j = save->getAlienMissions().begin(); j != save->getAlienMissions().end(); ++j)
				{
					// if the mission types match
					if (types[entry] == (*j)->getRules().getType())
					{
						for (std::vector<std::string>::iterator k = regions.begin(); k != regions.end();)
						{
							// and the regions match
							if ((*k) == (*j)->getRegion())
							{
								// prune the entry from the list
								k = regions.erase(k);
								continue;
							}
							++k;
						}
					}
				}

				// we have a valid list of regions containing bases, pick one.
				if (!regions.empty())
				{
					missionType = types[entry];
					targetRegion = regions[RNG::generate(0, regions.size()-1)];
					break;
				}
				// otherwise, try the next mission in the list.
				if (max > 1 && ++entry == max)
				{
					entry = 0;
				}
			}
		}
	}
	// now the easy stuff
	else if (!command->hasRegionWeights())
	{
		// no regionWeights means we pick from the table
		targetRegion = strategy.chooseRandomRegion(mod);
	}
	else
	{
		// otherwise, let the command dictate the region.
		targetRegion = command->generate(month, GEN_REGION);
	}

	if (targetRegion.empty())
	{
		// something went horribly wrong, we should have had at LEAST a region by now.
		return false;
	}

	// we're bound to end up with typos, so let's throw an exception instead of simply returning false
	// that way, the modder can fix their mistake
	if (mod->getRegion(targetRegion) == 0)
	{
		throw Exception("Error processing mission script named: " + command->getType() + ", region named: " + targetRegion + " is not defined");
	}

	if (missionType.empty()) // ie: not a terror mission, not targetting a base, or otherwise not already chosen
	{
		if (!command->hasMissionWeights())
		{
			// no weights means let the strategy pick
			missionType = strategy.chooseRandomMission(targetRegion);
		}
		else
		{
			// otherwise the command gives us the weights.
			missionType = command->generate(month, GEN_MISSION);
		}
	}

	if (missionType.empty())
	{
		// something went horribly wrong, we didn't manage to choose a mission type
		return false;
	}

	missionRules = mod->getAlienMission(missionType);

	// we're bound to end up with typos, so let's throw an exception instead of simply returning false
	// that way, the modder can fix their mistake
	if (missionRules == 0)
	{
		throw Exception("Error processing mission script named: " + command->getType() + ", mission type: " + missionType + " is not defined");
	}

	// do i really need to comment this? shouldn't it be obvious what's happening here?
	if (!command->hasRaceWeights())
	{
		missionRace = missionRules->generateRace(month);
	}
	else
	{
		missionRace = command->generate(month, GEN_RACE);
	}

	if (missionRace.empty())
	{
		throw Exception("Error processing mission script named: " + command->getType() + ", mission type: " + missionType + " has no available races");
	}

	// we're bound to end up with typos, so let's throw an exception instead of simply returning false
	// that way, the modder can fix their mistake
	if (mod->getAlienRace(missionRace) == 0)
	{
		throw Exception("Error processing mission script named: " + command->getType() + ", race: " + missionRace + " is not defined");
	}

	// ok, we've derived all the variables we need to start up our mission, let's do magic to turn those values into a mission
	AlienMission *mission = new AlienMission(*missionRules);
	mission->setRace(missionRace);
	mission->setId(_game->getSavedGame()->getId("ALIEN_MISSIONS"));
	mission->setRegion(targetRegion, *_game->getMod());
	mission->setMissionSiteZone(targetZone);
	strategy.addMissionRun(command->getVarName());
	mission->start(*_game, *_globe, command->getDelay());
	_game->getSavedGame()->getAlienMissions().push_back(mission);
	// if this flag is set, we want to delete it from the table so it won't show up again until the schedule resets.
	if (command->getUseTable())
	{
		strategy.removeMission(targetRegion, missionType);
	}

	// we did it, we can go home now.
	return true;

}

/**
 * Handler for clicking on a timer button.
 * @param action pointer to the mouse action.
 */
void GeoscapeState::btnTimerClick(Action *action)
{
	SDL_Event ev;
	ev.type = SDL_MOUSEBUTTONDOWN;
	ev.button.button = SDL_BUTTON_LEFT;
	Action a = Action(&ev, 0.0, 0.0, 0, 0);
	action->getSender()->mousePress(&a, this);
}

/**
 * Updates the scale.
 * @param dX delta of X;
 * @param dY delta of Y;
 */
void GeoscapeState::resize(int &dX, int &dY)
{
	if (_game->getSavedGame()->getSavedBattle())
		return;
	dX = Options::baseXResolution;
	dY = Options::baseYResolution;
	int divisor = 1;
	double pixelRatioY = 1.0;

	if (Options::nonSquarePixelRatio)
	{
		pixelRatioY = 1.2;
	}
	switch (Options::geoscapeScale)
	{
	case SCALE_SCREEN_DIV_6:
		divisor = 6;
		break;
	case SCALE_SCREEN_DIV_5:
		divisor = 5;
		break;
	case SCALE_SCREEN_DIV_4:
		divisor = 4;
		break;
	case SCALE_SCREEN_DIV_3:
		divisor = 3;
		break;
	case SCALE_SCREEN_DIV_2:
		divisor = 2;
		break;
	case SCALE_SCREEN:
		break;
	default:
		dX = 0;
		dY = 0;
		return;
	}

	Options::baseXResolution = std::max(Screen::ORIGINAL_WIDTH, Options::displayWidth / divisor);
	Options::baseYResolution = std::max(Screen::ORIGINAL_HEIGHT, (int)(Options::displayHeight / pixelRatioY / divisor));

	dX = Options::baseXResolution - dX;
	dY = Options::baseYResolution - dY;

	_globe->resize();

	for (std::vector<Surface*>::const_iterator i = _surfaces.begin(); i != _surfaces.end(); ++i)
	{
		if (*i != _globe)
		{
			(*i)->setX((*i)->getX() + dX);
			(*i)->setY((*i)->getY() + dY/2);
		}
	}

	_bg->setX((_globe->getWidth() - _bg->getWidth()) / 2);
	_bg->setY((_globe->getHeight() - _bg->getHeight()) / 2);

	int height = (Options::baseYResolution - Screen::ORIGINAL_HEIGHT) / 2 + 10;
	_sideTop->setHeight(height);
	_sideTop->setY(_sidebar->getY() - height - 1);
	_sideBottom->setHeight(height);
	_sideBottom->setY(_sidebar->getY() + _sidebar->getHeight() + 1);

	_sideLine->setHeight(Options::baseYResolution);
	_sideLine->setY(0);
	_sideLine->drawRect(0, 0, _sideLine->getWidth(), _sideLine->getHeight(), 15);
}
bool GeoscapeState::buttonsDisabled()
{
	return _zoomInEffectTimer->isRunning() || _zoomOutEffectTimer->isRunning();
}

void GeoscapeState::updateSlackingIndicator()
{
	if (!Options::oxceEnableSlackingIndicator)
		return;

	int scientistsSlacking = 0;
	int engineersSlacking = 0;
	for (auto xcomBase : *_game->getSavedGame()->getBases())
	{
		scientistsSlacking += xcomBase->getAvailableScientists();
		engineersSlacking += xcomBase->getAvailableEngineers();
	}
	if (scientistsSlacking > 0 || engineersSlacking > 0)
	{
		_txtSlacking->setText(tr("STR_SLACKING_INDICATOR").arg(scientistsSlacking).arg(engineersSlacking));
	}
	else
	{
		_txtSlacking->setText("");
	}
}

void GeoscapeState::cbxRegionChange(Action *)
{
	int index = _cbxRegion->getSelected();
	if (index < 1)
	{
		_game->getSavedGame()->debugRegion = nullptr;
	}
	else
	{
		_game->getSavedGame()->debugRegion = (*_game->getSavedGame()->getRegions())[index-1];
	}
}

void GeoscapeState::cbxZoneChange(Action *)
{
	_game->getSavedGame()->debugZone = _cbxZone->getSelected();
}

}
