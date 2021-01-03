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
#include "MonthlyReportState.h"
#include <climits>
#include <sstream>
#include "../Engine/Game.h"
#include "../Mod/Mod.h"
#include "../Engine/LocalizedText.h"
#include "../Interface/TextButton.h"
#include "../Interface/Window.h"
#include "../Interface/Text.h"
#include "../Savegame/SavedGame.h"
#include "../Savegame/Base.h"
#include "../Savegame/GameTime.h"
#include "PsiTrainingState.h"
#include "TrainingState.h"
#include "../Savegame/Region.h"
#include "../Savegame/Country.h"
#include "../Mod/RuleCountry.h"
#include "Globe.h"
#include "../Engine/Options.h"
#include "../Engine/Unicode.h"
#include "../Menu/CutsceneState.h"
#include "../Savegame/Base.h"
#include "../Battlescape/CommendationState.h"
#include "../Savegame/SoldierDiary.h"
#include "../Menu/SaveGameState.h"
#include "../Mod/RuleInterface.h"
#include "../Mod/RuleVideo.h"
#include "../Savegame/GeoscapeEvent.h"
#include "../Mod/RuleEvent.h"

namespace OpenXcom
{
/**
 * Initializes all the elements in the Monthly Report screen.
 * @param game Pointer to the core game.
 * @param psi Show psi training afterwards?
 * @param globe Pointer to the globe.
 */
MonthlyReportState::MonthlyReportState(Globe *globe) : _gameOver(0), _ratingTotal(0), _fundingDiff(0), _lastMonthsRating(0), _happyList(0), _sadList(0), _pactList(0), _cancelPactList(0)
{
	_globe = globe;
	// Create objects
	_window = new Window(this, 320, 200, 0, 0);
	_btnOk = new TextButton(50, 12, 135, 180);
	_btnBigOk = new TextButton(120, 18, 100, 174);
	_txtTitle = new Text(300, 17, 16, 8);
	_txtMonth = new Text(130, 9, 16, 24);
	_txtRating = new Text(160, 9, 146, 24);
	_txtIncome = new Text(300, 9, 16, 32);
	_txtMaintenance = new Text(130, 9, 16, 40);
	_txtBalance = new Text(160, 9, 146, 40);
	_txtBonus = new Text(300, 9, 16, 48);
	_txtDesc = new Text(280, 124, 16, 56);
	_txtFailure = new Text(290, 160, 15, 10);

	// Set palette
	setInterface("monthlyReport");

	add(_window, "window", "monthlyReport");
	add(_btnOk, "button", "monthlyReport");
	add(_btnBigOk, "button", "monthlyReport");
	add(_txtTitle, "text1", "monthlyReport");
	add(_txtMonth, "text1", "monthlyReport");
	add(_txtRating, "text1", "monthlyReport");
	add(_txtIncome, "text1", "monthlyReport");
	add(_txtMaintenance, "text1", "monthlyReport");
	add(_txtBalance, "text1", "monthlyReport");
	add(_txtBonus, "text1", "monthlyReport");
	add(_txtDesc, "text2", "monthlyReport");
	add(_txtFailure, "text2", "monthlyReport");

	centerAllSurfaces();

	// Set up objects
	setWindowBackground(_window, "monthlyReport");

	_btnOk->setText(tr("STR_OK"));
	_btnOk->onMouseClick((ActionHandler)&MonthlyReportState::btnOkClick);
	_btnOk->onKeyboardPress((ActionHandler)&MonthlyReportState::btnOkClick, Options::keyOk);
	_btnOk->onKeyboardPress((ActionHandler)&MonthlyReportState::btnOkClick, Options::keyCancel);

	_btnBigOk->setText(tr("STR_OK"));
	_btnBigOk->onMouseClick((ActionHandler)&MonthlyReportState::btnOkClick);
	_btnBigOk->onKeyboardPress((ActionHandler)&MonthlyReportState::btnOkClick, Options::keyOk);
	_btnBigOk->onKeyboardPress((ActionHandler)&MonthlyReportState::btnOkClick, Options::keyCancel);
	_btnBigOk->setVisible(false);

	_txtTitle->setBig();
	_txtTitle->setText(tr("STR_XCOM_PROJECT_MONTHLY_REPORT"));

	_txtFailure->setBig();
	_txtFailure->setAlign(ALIGN_CENTER);
	_txtFailure->setVerticalAlign(ALIGN_MIDDLE);
	_txtFailure->setWordWrap(true);
	_txtFailure->setText(tr("STR_YOU_HAVE_FAILED"));
	_txtFailure->setVisible(false);

	calculateChanges();

	int month = _game->getSavedGame()->getTime()->getMonth() - 1, year = _game->getSavedGame()->getTime()->getYear();
	if (month == 0)
	{
		month = 12;
		year--;
	}
	std::string m;
	switch (month)
	{
	case 1: m = "STR_JAN"; break;
	case 2: m = "STR_FEB"; break;
	case 3: m = "STR_MAR"; break;
	case 4: m = "STR_APR"; break;
	case 5: m = "STR_MAY"; break;
	case 6: m = "STR_JUN"; break;
	case 7: m = "STR_JUL"; break;
	case 8: m = "STR_AUG"; break;
	case 9: m = "STR_SEP"; break;
	case 10: m = "STR_OCT"; break;
	case 11: m = "STR_NOV"; break;
	case 12: m = "STR_DEC"; break;
	default: m = "";
	}
	_txtMonth->setText(tr("STR_MONTH").arg(tr(m)).arg(year));

	// Calculate rating
	int difficulty_threshold = _game->getMod()->getDefeatScore() + 100 * _game->getSavedGame()->getDifficultyCoefficient();
	std::string rating = tr("STR_RATING_TERRIBLE");
	if (_ratingTotal > difficulty_threshold - 300)
	{
		rating = tr("STR_RATING_POOR");
	}
	if (_ratingTotal > difficulty_threshold)
	{
		rating = tr("STR_RATING_OK");
	}
	if (_ratingTotal > 0)
	{
		rating = tr("STR_RATING_GOOD");
	}
	if (_ratingTotal > 500)
	{
		rating = tr("STR_RATING_EXCELLENT");
	}

	if (!_game->getMod()->getMonthlyRatings()->empty())
	{
		rating = "";
		int temp = INT_MIN;
		const std::map<int, std::string> *monthlyRatings = _game->getMod()->getMonthlyRatings();
		for (std::map<int, std::string>::const_iterator i = monthlyRatings->begin(); i != monthlyRatings->end(); ++i)
		{
			if (i->first > temp && i->first <= _ratingTotal)
			{
				temp = i->first;
				rating = tr(i->second);
			}
		}
	}

	_txtRating->setText(tr("STR_MONTHLY_RATING").arg(_ratingTotal).arg(rating));

	std::ostringstream ss;
	ss << tr("STR_INCOME") << "> " << Unicode::TOK_COLOR_FLIP << Unicode::formatFunding(_game->getSavedGame()->getCountryFunding());
	ss << " (";
	if (_fundingDiff > 0)
		ss << '+';
	ss << Unicode::formatFunding(_fundingDiff) << ")";
	_txtIncome->setText(ss.str());

	std::ostringstream ss2;
	ss2 << tr("STR_MAINTENANCE") << "> " << Unicode::TOK_COLOR_FLIP << Unicode::formatFunding(_game->getSavedGame()->getBaseMaintenance());
	_txtMaintenance->setText(ss2.str());

	int performanceBonus = _ratingTotal * _game->getMod()->getPerformanceBonusFactor();
	if (performanceBonus > 0)
	{
		// increase funds by performance bonus
		_game->getSavedGame()->setFunds(_game->getSavedGame()->getFunds() + performanceBonus);
		// display
		std::ostringstream ss4;
		ss4 << tr("STR_PERFORMANCE_BONUS") << "> " << Unicode::TOK_COLOR_FLIP << Unicode::formatFunding(performanceBonus);
		_txtBonus->setText(ss4.str());
		// shuffle the fields a bit for better overview
		int upper = _txtMaintenance->getY();
		int lower = _txtBonus->getY();
		_txtMaintenance->setY(lower);
		_txtBalance->setY(lower);
		_txtBonus->setY(upper);
	}
	else
	{
		// vanilla view
		_txtBonus->setVisible(false);
		_txtDesc->setY(_txtBonus->getY());
	}

	std::ostringstream ss3;
	ss3 << tr("STR_BALANCE") << "> " << Unicode::TOK_COLOR_FLIP << Unicode::formatFunding(_game->getSavedGame()->getFunds());
	_txtBalance->setText(ss3.str());

	_txtDesc->setWordWrap(true);

	// calculate satisfaction
	std::ostringstream ss5;
	std::string satisFactionString = tr("STR_COUNCIL_IS_DISSATISFIED");
	bool resetWarning = true;
	if (_ratingTotal > difficulty_threshold)
	{
		satisFactionString = tr("STR_COUNCIL_IS_GENERALLY_SATISFIED");
	}
	if (_ratingTotal > 500)
	{
		satisFactionString = tr("STR_COUNCIL_IS_VERY_PLEASED");
	}
	if (_lastMonthsRating <= difficulty_threshold && _ratingTotal <= difficulty_threshold)
	{
		satisFactionString = tr("STR_YOU_HAVE_NOT_SUCCEEDED");
		_pactList.erase(_pactList.begin(), _pactList.end());
		_cancelPactList.erase(_cancelPactList.begin(), _cancelPactList.end());
		_happyList.erase(_happyList.begin(), _happyList.end());
		_sadList.erase(_sadList.begin(), _sadList.end());
		_gameOver = 1;
	}

	ss5 << satisFactionString;

	if (!_gameOver)
	{
		if (_game->getSavedGame()->getFunds() <= _game->getMod()->getDefeatFunds())
		{
			if (_game->getSavedGame()->getWarned())
			{
				ss5.str("");
				ss5 << tr("STR_YOU_HAVE_NOT_SUCCEEDED");
				_pactList.erase(_pactList.begin(), _pactList.end());
				_cancelPactList.erase(_cancelPactList.begin(), _cancelPactList.end());
				_happyList.erase(_happyList.begin(), _happyList.end());
				_sadList.erase(_sadList.begin(), _sadList.end());
				_gameOver = 2;
			}
			else
			{
				ss5 << "\n\n" << tr("STR_COUNCIL_REDUCE_DEBTS");
				_game->getSavedGame()->setWarned(true);
				resetWarning = false;
			}
		}
	}
	if (resetWarning && _game->getSavedGame()->getWarned())
	{
		_game->getSavedGame()->setWarned(false);
	}

	ss5 << countryList(_happyList, "STR_COUNTRY_IS_PARTICULARLY_PLEASED", "STR_COUNTRIES_ARE_PARTICULARLY_HAPPY");
	ss5 << countryList(_sadList, "STR_COUNTRY_IS_UNHAPPY_WITH_YOUR_ABILITY", "STR_COUNTRIES_ARE_UNHAPPY_WITH_YOUR_ABILITY");
	ss5 << countryList(_pactList, "STR_COUNTRY_HAS_SIGNED_A_SECRET_PACT", "STR_COUNTRIES_HAVE_SIGNED_A_SECRET_PACT");
	ss5 << countryList(_cancelPactList, "STR_COUNTRY_HAS_CANCELLED_A_SECRET_PACT", "STR_COUNTRIES_HAVE_CANCELLED_A_SECRET_PACT");

	_txtDesc->setText(ss5.str());

	// Give modders some handles on political situation
	auto spawnEvent = [](const RuleEvent* spawnedEventRule)
	{
		if (spawnedEventRule)
		{
			GeoscapeEvent* newEvent = new GeoscapeEvent(*spawnedEventRule);
			int minutes = (spawnedEventRule->getTimer() + (RNG::generate(0, spawnedEventRule->getTimerRandom()))) / 30 * 30;
			if (minutes < 60) minutes = 60; // just in case
			newEvent->setSpawnCountdown(minutes);
			_game->getSavedGame()->getGeoscapeEvents().push_back(newEvent);

			// remember that it has been generated
			_game->getSavedGame()->addGeneratedEvent(spawnedEventRule);
		}
	};

	for (auto& traitorName : _pactList)
	{
		auto traitor = _game->getMod()->getCountry(traitorName, false);
		if (traitor)
		{
			spawnEvent(traitor->getSignedPactEvent());
		}
	}
	for (auto& exTraitorName : _cancelPactList)
	{
		auto exTraitor = _game->getMod()->getCountry(exTraitorName, false);
		if (exTraitor)
		{
			spawnEvent(exTraitor->getRejoinedXcomEvent());
		}
	}
}

/**
 *
 */
MonthlyReportState::~MonthlyReportState()
{
}

/**
 * Returns to the previous screen.
 * @param action Pointer to an action.
 */
void MonthlyReportState::btnOkClick(Action *)
{
	if (!_gameOver)
	{
		_game->popState();
		// Award medals for service time
		// Iterate through all your bases
		for (std::vector<Base*>::iterator b = _game->getSavedGame()->getBases()->begin(); b != _game->getSavedGame()->getBases()->end(); ++b)
		{
			// Iterate through all your soldiers
			for (std::vector<Soldier*>::iterator s = (*b)->getSoldiers()->begin(); s != (*b)->getSoldiers()->end(); ++s)
			{
				Soldier *soldier = _game->getSavedGame()->getSoldier((*s)->getId());
				// Award medals to eligible soldiers
				soldier->getDiary()->addMonthlyService();
				if (soldier->getDiary()->manageCommendations(_game->getMod(), _game->getSavedGame()->getMissionStatistics()))
				{
					_soldiersMedalled.push_back(soldier);
				}
			}
		}
		if (!_soldiersMedalled.empty())
		{
			_game->pushState(new CommendationState(_soldiersMedalled));
		}

		bool psi = false;
		for (std::vector<Base*>::const_iterator b = _game->getSavedGame()->getBases()->begin(); b != _game->getSavedGame()->getBases()->end(); ++b)
		{
			psi = psi || (*b)->getAvailablePsiLabs();
		}
		if (psi && !Options::anytimePsiTraining)
		{
			_game->pushState(new PsiTrainingState);
		}
		// Autosave
		if (_game->getSavedGame()->isIronman())
		{
			_game->pushState(new SaveGameState(OPT_GEOSCAPE, SAVE_IRONMAN, _palette));
		}
		else if (Options::autosave)
		{
			_game->pushState(new SaveGameState(OPT_GEOSCAPE, SAVE_AUTO_GEOSCAPE, _palette));
		}
	}
	else
	{
		if (_txtFailure->getVisible())
		{
			_game->popState(); // in case the cutscene is not marked as "game over" (by accident or not) let's return to the geoscape

			std::string cutsceneId;
			if (_gameOver == 1)
				cutsceneId = _game->getMod()->getLoseRatingCutscene();
			else
				cutsceneId = _game->getMod()->getLoseMoneyCutscene();

			const RuleVideo *videoRule = _game->getMod()->getVideo(cutsceneId, true);
			if (videoRule->getLoseGame())
			{
				_game->getSavedGame()->setEnding(END_LOSE);
			}

			_game->pushState(new CutsceneState(cutsceneId));
			if (_game->getSavedGame()->isIronman())
			{
				_game->pushState(new SaveGameState(OPT_GEOSCAPE, SAVE_IRONMAN, _palette));
			}
		}
		else
		{
			_window->setColor(_game->getMod()->getInterface("monthlyReport")->getElement("window")->color2);
			_txtTitle->setVisible(false);
			_txtMonth->setVisible(false);
			_txtRating->setVisible(false);
			_txtIncome->setVisible(false);
			_txtMaintenance->setVisible(false);
			_txtBalance->setVisible(false);
			_txtBonus->setVisible(false);
			_txtDesc->setVisible(false);
			_btnOk->setVisible(false);
			_btnBigOk->setVisible(true);
			_txtFailure->setVisible(true);
			_game->getMod()->playMusic("GMLOSE");
		}
	}
}

/**
 * Update all our activity counters, gather all our scores,
 * get our countries to make sign pacts, adjust their fundings,
 * assess their satisfaction, and finally calculate our overall
 * total score, with thanks to Volutar for the formulas.
 */
void MonthlyReportState::calculateChanges()
{
	// initialize all our variables.
	_lastMonthsRating = 0;
	int xcomSubTotal = 0;
	int xcomTotal = 0;
	int alienTotal = 0;
	int monthOffset = _game->getSavedGame()->getFundsList().size() - 2;
	int lastMonthOffset = _game->getSavedGame()->getFundsList().size() - 3;
	if (lastMonthOffset < 0)
		lastMonthOffset += 2;
	// update activity meters, calculate a total score based on regional activity
	// and gather last month's score
	for (std::vector<Region*>::iterator k = _game->getSavedGame()->getRegions()->begin(); k != _game->getSavedGame()->getRegions()->end(); ++k)
	{
		(*k)->newMonth();
		if ((*k)->getActivityXcom().size() > 2)
			_lastMonthsRating += (*k)->getActivityXcom().at(lastMonthOffset)-(*k)->getActivityAlien().at(lastMonthOffset);
		xcomSubTotal += (*k)->getActivityXcom().at(monthOffset);
		alienTotal += (*k)->getActivityAlien().at(monthOffset);
	}
	// apply research bonus AFTER calculating our total, because this bonus applies to the council ONLY,
	// and shouldn't influence each country's decision.

	// the council is more lenient after the first month
	if (_game->getSavedGame()->getMonthsPassed() > 1)
		_game->getSavedGame()->getResearchScores().at(monthOffset) += 400;

	xcomTotal = _game->getSavedGame()->getResearchScores().at(monthOffset) + xcomSubTotal;

	if (_game->getSavedGame()->getResearchScores().size() > 2)
		_lastMonthsRating += _game->getSavedGame()->getResearchScores().at(lastMonthOffset);

	// now that we have our totals we can send the relevant info to the countries
	// and have them make their decisions weighted on the council's perspective.
	const RuleAlienMission *infiltration = _game->getMod()->getRandomMission(OBJECTIVE_INFILTRATION, _game->getSavedGame()->getMonthsPassed());
	int pactScore = 0;
	if (infiltration)
	{
		pactScore = infiltration->getPoints();
	}
	int averageFunding = _game->getSavedGame()->getCountryFunding() / _game->getSavedGame()->getCountries()->size() / 1000 * 1000;
	for (std::vector<Country*>::iterator k = _game->getSavedGame()->getCountries()->begin(); k != _game->getSavedGame()->getCountries()->end(); ++k)
	{
		// add them to the list of new pact members
		// this is done BEFORE initiating a new month
		// because the _newPact flag will be reset in the
		// process
		if ((*k)->getNewPact())
		{
			_pactList.push_back((*k)->getRules()->getType());
		}
		if ((*k)->getCancelPact() && (*k)->getPact())
		{
			_cancelPactList.push_back((*k)->getRules()->getType());
		}
		// determine satisfaction level, sign pacts, adjust funding
		// and update activity meters,
		(*k)->newMonth(xcomTotal, alienTotal, pactScore, averageFunding);
		// and after they've made their decisions, calculate the difference, and add
		// them to the appropriate lists.
		_fundingDiff += (*k)->getFunding().back()-(*k)->getFunding().at((*k)->getFunding().size()-2);
		switch((*k)->getSatisfaction())
		{
		case 1:
			_sadList.push_back((*k)->getRules()->getType());
			break;
		case 3:
			_happyList.push_back((*k)->getRules()->getType());
			break;
		default:
			break;
		}
	}
	//calculate total.
	_ratingTotal = xcomTotal - alienTotal;
}

/**
 * Builds a sentence from a list of countries, adding the appropriate
 * separators and pluralization.
 * @param countries List of country string IDs.
 * @param singular String ID to append at the end if the list is singular.
 * @param plural String ID to append at the end if the list is plural.
 */
std::string MonthlyReportState::countryList(const std::vector<std::string> &countries, const std::string &singular, const std::string &plural)
{
	std::ostringstream ss;
	if (!countries.empty())
	{
		ss << "\n\n";
		if (countries.size() == 1)
		{
			ss << tr(singular).arg(tr(countries.front()));
		}
		else
		{
			LocalizedText list = tr(countries.front());
			std::vector<std::string>::const_iterator i;
			for (i = countries.begin() + 1; i < countries.end() - 1; ++i)
			{
				list = tr("STR_COUNTRIES_COMMA").arg(list).arg(tr(*i));
			}
			list = tr("STR_COUNTRIES_AND").arg(list).arg(tr(*i));
			ss << tr(plural).arg(list);
		}
	}
	return ss.str();
}

}
