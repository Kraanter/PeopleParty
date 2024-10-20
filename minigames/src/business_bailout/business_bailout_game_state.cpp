#include "business_bailout_game_state.h"

BB_GlobalData::BB_GlobalData() : moneyMoments() { }

std::string BB_GlobalData::Serialize() const {
    return "";
}

void BB_GlobalData::Deserialize(const std::string &data) {

}

std::string BB_HostData::Serialize() const {
    return "";
}

void BB_HostData::Deserialize(const std::string &data) {

}

std::string BB_MoneyMoment::Serialize() const {
    return "";
}

void BB_MoneyMoment::Deserialize(const std::string &data) {

}

std::string BB_PlayerData::Serialize() const {
    return "";
}

void BB_PlayerData::Deserialize(const std::string &data) {

}
