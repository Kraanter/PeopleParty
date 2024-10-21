#include "business_bailout_game_state.h"

BB_GlobalData::BB_GlobalData() : moneyMoments() { }

std::string BB_GlobalData::Serialize() const {
    return "global";
}

void BB_GlobalData::Deserialize(const std::string &data) {

}

void BB_GlobalData::addMoneyMoment(double money, double time) {
    auto moment = BB_MoneyMoment(money, time);

    this->moneyMoments.push_back(moment);
}

std::string BB_HostData::Serialize() const {
    return "host";
}

void BB_HostData::Deserialize(const std::string &data) {
    this->minigame_time = 1000;
}

std::string BB_MoneyMoment::Serialize() const {
    return "money";
}

void BB_MoneyMoment::Deserialize(const std::string &data) {
}

std::string BB_PlayerData::Serialize() const {
    return "player";
}

void BB_PlayerData::Deserialize(const std::string &data) {
    this->money = 10;
}
