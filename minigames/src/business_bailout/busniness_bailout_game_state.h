#ifndef INCLUDE_GPT_CUSTOM_H_
#define INCLUDE_GPT_CUSTOM_H_

#include "../debug.h"
#include <iostream>
#include <ostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

class BB_HostData {
public:
    // The time the rocket will go on rising in miliseconds
    int minigame_time;

    BB_HostData() { }

    std::string Serialize() const {
        return std::to_string(minigame_time);
    }

    void Deserialize(const std::string& data) {
        std::cout << "host " << data << std::endl;
        minigame_time = std::stoi(data);
    }
};

class BB_PlayerData {
public:
    // The amount of money the player sold at. -1 if not sold (yet)
    double money;

    std::string Serialize() const {
        return std::to_string(money);
    }

    void Deserialize(const std::string& data) {
        money = std::stod(data);
    }
};

struct BB_MoneyMoment {
    // The amount of money at a certain moment in time
    double money;
    // The timestamp of this moment in miliseconds since game start
    double time;

    BB_MoneyMoment(double money, double time): money(money), time(time) { }

    std::string Serialize() const {
        return std::to_string(money) + "-" + std::to_string(time);
    }

    static BB_MoneyMoment Deserialize(const std::string& data) {
        size_t split = data.find("-");

        if (split == std::string::npos) {
            throw std::runtime_error("Invalid serialized format for money moment");
        }

        BB_MoneyMoment retObj(std::stod(data.substr(0, split)), std::stod(data.substr(split + 1)));

        return retObj;
    }
};

class BB_GlobalData {
public:
    // List of all points of money moments on a certain time
    std::vector<BB_MoneyMoment> moneyMoments;

    BB_GlobalData();

    void addMoneyMoment(double money, double time) {
        BB_MoneyMoment mm(money, time);
        moneyMoments.push_back(mm);
    }

    std::string Serialize() const {
        std::string result = "";

        for (const auto &item : moneyMoments) {
            result += item.Serialize() + "\n";
        }

        return result;
    }

    void Deserialize(const std::string& data) {
        moneyMoments.clear();
        std::istringstream moneyStream(data);
        std::string line;
        while(std::getline(moneyStream, line)) {
            moneyMoments.push_back(BB_MoneyMoment::Deserialize(line));
        }
    }
};

#endif  // INCLUDE_GPT_CUSTOM_H_
