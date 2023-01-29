#include "pch.h"
#include "EncString.h"

// Convert a wide Unicode string to an UTF8 string
std::string WStringToString(const std::wstring& s)
{
    // @Cleanup: assert used incorrectly here; value passed could be from anywhere!
    if (s.empty())
        return "";
    // NB: GW uses code page 0 (CP_ACP)
    int size_needed =
      WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, &s[0], (int)s.size(), NULL, 0, NULL, NULL);
    assert(size_needed != 0);
    std::string strTo(size_needed, 0);
    assert(WideCharToMultiByte(CP_UTF8, 0, &s[0], (int)s.size(), &strTo[0], size_needed, NULL, NULL));
    return std::move(strTo);
}

void EncString::reset(const uint32_t _enc_string_id, bool sanitise)
{
    if (_enc_string_id && encoded_ws.length())
    {
        uint32_t this_id = GW::UI::EncStrToUInt32(encoded_ws.c_str());
        if (this_id == _enc_string_id)
            return;
    }
    reset(nullptr, sanitise);
    if (_enc_string_id)
    {
        wchar_t out[8] = {0};
        if (! GW::UI::UInt32ToEncStr(_enc_string_id, out, _countof(out)))
            return;
        encoded_ws = out;
    }
}
EncString* EncString::language(GW::Constants::TextLanguage l)
{
    if (language_id == l)
        return this;
    decoded_ws.clear();
    decoded_s.clear();
    decoding = sanitised = decoded = false;
    language_id = l;
    return this;
}

void EncString::reset(const wchar_t* _enc_string, bool sanitise)
{
    if (_enc_string && wcscmp(_enc_string, encoded_ws.c_str()) == 0)
        return;
    encoded_ws.clear();
    decoded_ws.clear();
    decoded_s.clear();
    decoding = decoded = false;
    sanitised = ! sanitise;
    if (_enc_string)
        encoded_ws = _enc_string;
}

std::wstring& EncString::wstring()
{
    if (! decoded && ! decoding && ! encoded_ws.empty())
    {
        decoding = true;
        GW::UI::AsyncDecodeStr(encoded_ws.c_str(), OnStringDecoded, (void*)this, (uint32_t)language_id);
    }
    sanitise();
    return decoded_ws;
}

void EncString::sanitise()
{
    if (! sanitised && ! decoded_ws.empty())
    {
        sanitised = true;
        static const std::wregex sanitiser(L"<[^>]+>");
        decoded_ws = std::regex_replace(decoded_ws, sanitiser, L"");
    }
}
void EncString::OnStringDecoded(void* param, wchar_t* decoded)
{
    EncString* context = (EncString*)param;
    assert(context && context->decoding && ! context->decoded);
    if (decoded && decoded[0])
        context->decoded_ws = decoded;
    context->decoded = true;
    context->decoding = false;
}

std::string format(const char* msg, ...)
{
    std::string out;
    va_list args;
    va_start(args, msg);
    const auto size = vsnprintf(NULL, 0, msg, args);
    out.resize(size + 1);
    assert(vsnprintf(out.data(), out.size(), msg, args) <= size);
    va_end(args);
    return std::move(out);
}
std::wstring format(const wchar_t* msg, ...)
{
    std::wstring out;
    va_list args;
    va_start(args, msg);
    const auto size = _vsnwprintf(NULL, 0, msg, args);
    out.resize(size + 1);
    assert(_vsnwprintf(out.data(), out.size(), msg, args) <= size);
    va_end(args);
    return std::move(out);
}

std::string& EncString::string()
{
    wstring();
    if (sanitised && ! decoded_ws.empty() && decoded_s.empty())
    {
        decoded_s = WStringToString(decoded_ws);
    }
    return decoded_s;
}
