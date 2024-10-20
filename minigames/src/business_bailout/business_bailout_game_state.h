#ifndef INCLUDE_BUSINESS_BAILOUT_BUSINESS_BAILOUT_GAME_STATE_H_
#define INCLUDE_BUSINESS_BAILOUT_BUSINESS_BAILOUT_GAME_STATE_H_

#include "../debug.h"
#include <string>
#include <vector>

class BB_HostData {
  public:
	// The time the rocket will go on rising in miliseconds
	int minigame_time;

	BB_HostData() {}

	std::string Serialize() const;
	void Deserialize(const std::string &data);
};

class BB_PlayerData {
  public:
	// The amount of money the player sold at. -1 if not sold (yet)
	double money;

	std::string Serialize() const;
	void Deserialize(const std::string &data);
};

struct BB_MoneyMoment {
	// The amount of money at a certain moment in time
	double money;
	// The timestamp of this moment in miliseconds since game start
	double time;

	BB_MoneyMoment(double money, double time) : money(money), time(time) {}

	std::string Serialize() const;
	BB_MoneyMoment Deserialize(const std::string &data);
};

class BB_GlobalData {
  public:
	// List of all points of money moments on a certain time
	std::vector<BB_MoneyMoment> moneyMoments;

	BB_GlobalData();

	void addMoneyMoment(double money, double time);

	std::string Serialize() const;
	void Deserialize(const std::string &date);
};

#endif  // INCLUDE_BUSINESS_BAILOUT_BUSINESS_BAILOUT_GAME_STATE_H_
