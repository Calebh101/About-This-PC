#ifndef SETTINGS_H
#define SETTINGS_H

#include "json.hpp"

using json = nlohmann::json;

class Settings
{
public:
    Settings();
    json loaded;
    static json defaults();
};

#endif // SETTINGS_H
