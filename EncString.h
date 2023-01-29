#pragma once

class EncString
{
protected:
    std::wstring encoded_ws;
    std::wstring decoded_ws;
    std::string decoded_s;
    bool decoding = false;
    bool decoded = false;
    bool sanitised = false;
    virtual void sanitise();
    GW::Constants::TextLanguage language_id = (GW::Constants::TextLanguage)-1;
    static void OnStringDecoded(void* param, wchar_t* decoded);

public:
    // Set the language for decoding this encoded string. If the language has changed, resets the decoded result. Returns this for chaining.
    EncString* language(GW::Constants::TextLanguage l = (GW::Constants::TextLanguage)-1);
    bool IsDecoding() { return decoding && decoded_ws.empty(); };
    // Recycle this EncString by passing a new encoded string id to decode.
    // Set sanitise to true to automatically remove guild tags etc from the string
    void reset(uint32_t _enc_string_id = 0, bool sanitise = true);
    // Recycle this EncString by passing a new string to decode.
    // Set sanitise to true to automatically remove guild tags etc from the string
    void reset(const wchar_t* _enc_string = nullptr, bool sanitise = true);
    std::wstring& wstring();
    std::string& string();
    const std::wstring& encoded() const { return encoded_ws; };
    EncString(const wchar_t* _enc_string = nullptr, bool sanitise = true) { reset(_enc_string, sanitise); }
    EncString(const uint32_t _enc_string, bool sanitise = true) { reset(_enc_string, sanitise); }
    // Disable object copying; decoded_ws is passed to GW by reference and would be bad to do this. Pass by pointer instead.
    EncString(const EncString& temp_obj) = delete;
    EncString& operator=(const EncString& temp_obj) = delete;
    virtual ~EncString() = default;
};
