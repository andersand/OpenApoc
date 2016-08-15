#include "game/state/gametime.h"
#include <sstream>

// for my sake
using namespace boost::gregorian;
using namespace boost::posix_time;

namespace OpenApoc
{

const ptime GameTime::GAME_START = ptime(date(2084, Mar, 7), time_duration(0, 0, 0));
std::locale *GameTime::TIME_FORMAT = nullptr;
std::locale *GameTime::DATE_FORMAT = nullptr;

// FIXME: Refactor to always use ptime instead of ticks?
time_duration GameTime::ticksToPosix(int64_t ticks)
{
	int64_t tickTotal = std::round(static_cast<double>(ticks * time_duration::ticks_per_second()) /
	                               TICKS_PER_SECOND);
	return time_duration(0, 0, 0, tickTotal);
}

GameTime::GameTime(uint64_t ticks) : ticks(ticks), dayPassedFlag(false), weekPassedFlag(false)
{
	datetime = GAME_START + ticksToPosix(ticks);
}

UString GameTime::getTimeString() const
{
	std::stringstream ss;
	if (TIME_FORMAT == nullptr)
	{
		// locale controls the facet
		time_facet *timeFacet = new time_facet("%H:%M:%S");
		TIME_FORMAT = new std::locale(std::locale::classic(), timeFacet);
	}
	ss.imbue(*TIME_FORMAT);
	ss << datetime;
	return ss.str();
}

UString GameTime::getDateString() const
{
	std::stringstream ss;
	if (DATE_FORMAT == nullptr)
	{
		date_facet *dateFacet = new date_facet("%A, %d %B, %Y");
		DATE_FORMAT = new std::locale(std::locale::classic(), dateFacet);

		std::vector<std::string> months = {
		    tr("January").str(), tr("February").str(), tr("March").str(),
		    tr("April").str(),   tr("May").str(),      tr("June").str(),
		    tr("July").str(),    tr("August").str(),   tr("September").str(),
		    tr("October").str(), tr("November").str(), tr("December").str()};
		dateFacet->long_month_names(months);

		std::vector<std::string> weekdays = {
		    tr("Sunday").str(),   tr("Monday").str(), tr("Tuesday").str(), tr("Wednesday").str(),
		    tr("Thursday").str(), tr("Friday").str(), tr("Saturday").str()};
		dateFacet->long_weekday_names(weekdays);
	}
	ss.imbue(*DATE_FORMAT);
	ss << datetime.date();
	return ss.str();
}

UString GameTime::getWeekString() const { return UString::format("%s %d", tr("Week"), getWeek()); }

unsigned int GameTime::getWeek() const
{
	date firstMonday = previous_weekday(GAME_START.date(), greg_weekday(Monday));
	date lastMonday = previous_weekday(datetime.date(), greg_weekday(Monday));
	date_duration duration = lastMonday - firstMonday;
	return duration.days() / 7 + 1;
}

unsigned int GameTime::getDay() const { return datetime.date().day(); }

unsigned int GameTime::getHours() const { return datetime.time_of_day().hours(); }

unsigned int GameTime::getMinutes() const { return datetime.time_of_day().minutes(); }

uint64_t GameTime::getTicks() const { return ticks; }

bool GameTime::dayPassed() const { return dayPassedFlag; }

bool GameTime::weekPassed() const { return weekPassedFlag; }

void GameTime::clearFlags()
{
	dayPassedFlag = false;
	weekPassedFlag = false;
}

void GameTime::addTicks(uint64_t ticks)
{
	this->ticks += ticks;
	this->datetime += ticksToPosix(ticks);
	uint64_t dayTicks = this->ticks % TICKS_PER_DAY;
	if (dayTicks < ticks)
	{
		uint64_t days = this->ticks / TICKS_PER_DAY;
		dayPassedFlag = true;
		if (days % 7 == 0)
		{
			weekPassedFlag = true;
		}
	}
}

GameTime GameTime::midday() { return GameTime(TICKS_PER_HOUR * 12); }
}