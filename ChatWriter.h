#pragma once

enum ChatColor
{
    White,
    Black,
    Red,
    Yellow,
    Blue,
    Purple,
    Green,
    Orange,
    Pink,
    LightRed,
    LightYellow,
    LightBlue,
    LightPurple,
    LightGreen,
    LightOrange,
    LightPink,
    DarkRed,
    DarkYellow,
    DarkBlue,
    DarkPurple,
    DarkGreen,
    DarkOrange,
    DarkPink,
    VeryLightRed,
    VeryLightYellow,
    VeryLightBlue,
    VeryLightPurple,
    VeryLightGreen,
    VeryLightOrange,
    VeryLightPink,
    VeryDarkRed,
    VeryDarkYellow,
    VeryDarkBlue,
    VeryDarkPurple,
    VeryDarkGreen,
    VeryDarkOrange,
    VeryDarkPink
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
        case Red:
            hex_color = "#FF4136";
            break;
        case Yellow:
            hex_color = "#FFDC00";
            break;
        case Blue:
            hex_color = "#0074D9";
            break;
        case Purple:
            hex_color = "#B10DC9";
            break;
        case Green:
            hex_color = "#2ECC40";
            break;
        case Orange:
            hex_color = "#FF851B";
            break;
        case Pink:
            hex_color = "#F012BE";
            break;
        case LightRed:
            hex_color = "#FF7F50";
            break;
        case LightYellow:
            hex_color = "#FFF8DC";
            break;
        case LightBlue:
            hex_color = "#87CEEB";
            break;
        case LightPurple:
            hex_color = "#DDA0DD";
            break;
        case LightGreen:
            hex_color = "#90EE90";
            break;
        case LightOrange:
            hex_color = "#FFA07A";
            break;
        case LightPink:
            hex_color = "#FFC0CB";
            break;
        case DarkRed:
            hex_color = "#8B0000";
            break;
        case DarkYellow:
            hex_color = "#9B870C";
            break;
        case DarkBlue:
            hex_color = "#00008B";
            break;
        case DarkPurple:
            hex_color = "#4B0082";
            break;
        case DarkGreen:
            hex_color = "#006400";
            break;
        case DarkOrange:
            hex_color = "#FF8C00";
            break;
        case DarkPink:
            hex_color = "#FF1493";
            break;
        case VeryLightRed:
            hex_color = "#FFF8DC";
            break;
        case VeryLightYellow:
            hex_color = "#FFFFF0";
            break;
        case VeryLightBlue:
            hex_color = "#F0FFFF";
            break;
        case VeryLightPurple:
            hex_color = "#F5F0FF";
            break;
        case VeryLightGreen:
            hex_color = "#F0FFF0";
            break;
        case VeryLightOrange:
            hex_color = "#FFECB3";
            break;
        case VeryLightPink:
            hex_color = "#FFF8DC";
            break;
        case VeryDarkRed:
            hex_color = "#800000";
            break;
        case VeryDarkYellow:
            hex_color = "#964B00";
            break;
        case VeryDarkBlue:
            hex_color = "#000080";
            break;
        case VeryDarkPurple:
            hex_color = "#663399";
            break;
        case VeryDarkGreen:
            hex_color = "#003300";
            break;
        case VeryDarkOrange:
            hex_color = "#FF7F00";
            break;
        case VeryDarkPink:
            hex_color = "#FF00FF";
            break;
        }
        return hex_color;
    }

public:
    static void WriteIngameDebugChat(std::wstring chat_message)
    {
        std::wstring init_message = std::format(L"<c=#FFFFFF>: {}</c>", chat_message);

        GW::Chat::WriteChat(GW::Chat::CHANNEL_GWCA2, init_message.c_str(), nullptr);
    }

    static void WriteIngameDebugChat(std::string chat_message)
    {
        if (chat_message.size() == 0)
            return;

        std::wstring wide_chat_message(chat_message.begin(), chat_message.end());

        ChatWriter::WriteIngameDebugChat(wide_chat_message);
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
