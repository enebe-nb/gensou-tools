#pragma once

#include <string>
#include <nlohmann/json.hpp>

class Event {
protected:
    nlohmann::json data;
public:
    inline Event(const nlohmann::json& data = "{\"type\":\"unknown\"}"_json) : data(data) {}
    inline std::string to_string() { return data.dump(); }
    operator std::string() const { return data.dump(); }

    class BattleBegin;
    class BattleEnd;
    class RoundEnd;
};

class Event::BattleBegin : public Event { public: BattleBegin(); };
class Event::BattleEnd : public Event { public: BattleEnd(); };
class Event::RoundEnd : public Event { public: RoundEnd(); };