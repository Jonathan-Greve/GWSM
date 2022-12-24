#pragma once

enum ChatColor
{
    White,
    Black,
    DarkRed,
    Red,
    LightRed,
    DarkBlue,
    Blue,
    LightBlue,
    DarkGreen,
    Green,
    LightGreen
};

class ChatWriter
{
    static std::string ChatColorToString(ChatColor color)
    {
        std::string hex_color;
        switch (color)
        {
        case White:
            hex_color = "#FFFFFF";
            break;
        case Black:
            hex_color = "#000000";
            break;
        case DarkRed:
            hex_color = "#7B241C";
            break;
        case Red:
            hex_color = "#A93226";
            break;
        case LightRed:
            hex_color = "#E6B0AA";
            break;
        case DarkBlue:
            hex_color = "#1A5276";
            break;
        case Blue:
            hex_color = "#2980B9";
            break;
        case LightBlue:
            hex_color = "#A9CCE3";
            break;
        case DarkGreen:
            hex_color = "#196F3D";
            break;
        case Green:
            hex_color = "#28B463";
            break;
        case LightGreen:
            hex_color = "#ABEBC6";
            break;
        }
        return hex_color;
    }

public:
    static void WriteIngameDebugChat(std::string chat_message)
    {
        if (chat_message.size() == 0)
            return;

        std::wstring wide_chat_message(chat_message.begin(), chat_message.end());
        std::wstring init_message = std::format(L"<c=#FFFFFF>: {}</c>", wide_chat_message);

        GW::Chat::WriteChat(GW::Chat::CHANNEL_GWCA2, init_message.c_str(), nullptr);
    }

    static void WriteIngameDebugChat(std::string chat_message, std::string hex_color)
    {
        if (chat_message.size() == 0 || hex_color.size() == 0)
            return;

        std::wstring wide_chat_message(chat_message.begin(), chat_message.end());
        std::wstring wide_hex_color(hex_color.begin(), hex_color.end());
        std::wstring init_message;
        if (hex_color[0] == '#' && hex_color.size() == 7)
            init_message = std::format(L"<c={}>: {}</c>", wide_hex_color, wide_chat_message);
        else if (hex_color[0] != '#' && hex_color.size() == 6)
            init_message = std::format(L"<c=#{}>: {}</c>", wide_hex_color, wide_chat_message);
        else
            return;

        GW::Chat::WriteChat(GW::Chat::CHANNEL_GWCA2, init_message.c_str(), nullptr);
    }

    static void WriteIngameDebugChat(std::string chat_message, ChatColor color)
    {
        std::string hex_color = ChatWriter::ChatColorToString(color);
        std::wstring wide_hex_color(hex_color.begin(), hex_color.end());
        std::wstring wide_chat_message(chat_message.begin(), chat_message.end());

        std::wstring init_message = std::format(L"<c={}>: {}</c>", wide_hex_color, wide_chat_message);

        GW::Chat::WriteChat(GW::Chat::CHANNEL_GWCA2, init_message.c_str(), nullptr);
    }
};
