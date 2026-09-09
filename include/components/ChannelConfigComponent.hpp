#pragma once

#include <string>

struct ChannelConfigComponent
{
    std::string name;
    std::string topic;
    std::string key;
    size_t userLimit;
    bool isInviteOnly;
    bool isTopicRestricted;
    bool hasKey;
    bool hasLimit;

    ChannelConfigComponent() : name(""), topic(""), key(""), userLimit(0), isInviteOnly(false), isTopicRestricted(false), hasKey(false), hasLimit(false) {}
    ~ChannelConfigComponent() {}
};