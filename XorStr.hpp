#pragma once

/*
Original Author: Adam Yaxley
LICENSE: Public Domain (http://www.unlicense.org)
Website: https://github.com/adamyaxley

Obfuscate
Guaranteed compile-time string literal obfuscation library for C++14
*/

// Workaround for __LINE__ not being constexpr when /ZI (Edit and Continue) is enabled in Visual Studio
// See: https://developercommunity.visualstudio.com/t/-line-cannot-be-used-as-an-argument-for-constexpr/195665
#ifdef _MSC_VER
#define AY_CAT(X,Y) AY_CAT2(X,Y)
#define AY_CAT2(X,Y) X##Y
#define AY_LINE int(AY_CAT(__LINE__,U))
#else
#define AY_LINE __LINE__
#endif

namespace ay {
    using size_type = unsigned long long;
    using key_type  = unsigned long long;

    // Generate a pseudo-random key that spans all 8 bytes
    constexpr key_type generate_key(key_type seed) {
        // Use the MurmurHash3 64-bit finalizer to hash our seed
        key_type key = seed;
        key ^= (key >> 33);
        key *= 0xff51afd7ed558ccd;
        key ^= (key >> 33);
        key *= 0xc4ceb9fe1a85ec53;
        key ^= (key >> 33);

        // Make sure that a bit in each byte is set
        key |= 0x0101010101010101ull;

        return key;
    }

    // Obfuscates or deobfuscates data with key
    constexpr void cipher(char* data, size_type size, key_type key) {
        // Obfuscate with a simple XOR cipher based on key
        for (size_type i = 0; i < size; i++)
            data[i] ^= char((key >> ((i % 8) * 8)) & 0xFF);
    }

    // Obfuscates a string at compile time
    template <size_type N, key_type KEY>
    class obfuscator {
    public:
        // Obfuscates the string 'data' on construction
        constexpr obfuscator(const char* data) {
            // Copy data
            for (size_type i = 0; i < N; i++)
                m_data[i] = data[i];

            // On construction each of the characters in the string is
            // obfuscated with an XOR cipher based on key
            cipher(m_data, N, KEY);
        }

        constexpr const char* data() const {
            return &m_data[0];
        }

        constexpr size_type size() const {
            return N;
        }

        constexpr key_type key() const {
            return KEY;
        }

    private:
        char m_data[N]{};
    };

    // Handles decryption and re-encryption of an encrypted string at runtime
    template <size_type N, key_type KEY>
    class obfuscated_data {
    public:
        obfuscated_data(const obfuscator<N, KEY>& obfuscator) {
            // Copy obfuscated data
            for (size_type i = 0; i < N; i++)
                m_data[i] = obfuscator.data()[i];
        }

        ~obfuscated_data() {
            // Zero m_data to remove it from memory
            for (size_type i = 0; i < N; i++)
                m_data[i] = 0;
        }

        // Returns a pointer to the plain text string, decrypting it if
        // necessary
        operator char* () {
            return decrypt();
        }

        // Manually decrypt the string
        char* decrypt() {
            if (m_encrypted)
            {
                cipher(m_data, N, KEY);
                m_encrypted = false;
            }
            return m_data;
        }

        // Manually re-encrypt the string
        char* encrypt() {
            if (!m_encrypted)
            {
                cipher(m_data, N, KEY);
                m_encrypted = true;
            }
            return m_data;
        }

        // Returns true if this string is currently encrypted, false otherwise.
        bool is_encrypted() const {
            return m_encrypted;
        }

    private:
        // Local storage for the string. Call is_encrypted() to check whether or
        // not the string is currently obfuscated.
        char m_data[N];
        // Whether data is currently encrypted
        bool m_encrypted{ true };
    };

    // This function exists purely to extract the number of elements 'N' in the
    // array 'data'
    template <size_type N, key_type KEY = ay::generate_key(AY_LINE)>
    constexpr auto make_obfuscator(const char(&data)[N]) {
        return obfuscator<N, KEY>(data);
    }
}

// Obfuscates the string 'data' with 'key' at compile-time and returns a
// reference to a ay::obfuscated_data object with global lifetime that has
// functions for decrypting the string and is also implicitly convertable to a
// char*
#define AY_EncStr_KEY(data, key) \
    []() -> ay::obfuscated_data<sizeof(data)/sizeof(data[0]), key>& { \
        static_assert(sizeof(decltype(key)) == sizeof(ay::key_type), "key must be a 64 bit unsigned integer"); \
        static_assert((key) >= (1ull << 56), "key must span all 8 bytes"); \
        constexpr auto n = sizeof(data)/sizeof(data[0]); \
        constexpr auto obfuscator = ay::make_obfuscator<n, key>(data); \
        thread_local auto obfuscated_data = ay::obfuscated_data<n, key>(obfuscator); \
        return obfuscated_data; \
    }()

#define CmdStr(data) AY_EncStr_KEY(data, ay::generate_key((int)'C'))
#define EncStr(data) AY_EncStr_KEY(data, ay::generate_key((int)'X'))
#define NewStr(data) AY_EncStr_KEY(data, ay::generate_key((int)'N'))
#define APIStr(data) AY_EncStr_KEY(data, ay::generate_key((int)'W'))
#define HookStr(data) AY_EncStr_KEY(data, ay::generate_key((int)'P'))
#define MsgBoxStr(data) AY_EncStr_KEY(data, ay::generate_key((int)'M'))
#define DLLStr(data) AY_EncStr_KEY(data, ay::generate_key((int)'D'))
#define LogStr(data) AY_EncStr_KEY(data, ay::generate_key((int)'L'))
#define AuthStr(data) AY_EncStr_KEY(data, ay::generate_key((int)'A'))
#define MPStr(data) AY_EncStr_KEY(data, ay::generate_key((int)'M'))
#define NexonStr(data) AY_EncStr_KEY(data, ay::generate_key((int)'X'))

// EASY CONTROL
namespace XorStr
{
    inline char* Kernel32DLL() {
        return EncStr("kernel32.dll");
    }

    inline char* Kernel32DLL_IsDebuggerPresent() {
        return EncStr("IsDebuggerPresent");
    }

    inline char* CREATEINTERFACE() {
        return EncStr("CreateInterface");
    }

    inline char* FatalError() {
        return EncStr("Fatal Error");
    }

    inline char* Error() {
        return EncStr("Error");
    }

    inline char* FS_r() {
        return EncStr("r");
    }
    inline char* FS_rb() {
        return EncStr("rb");
    }
}