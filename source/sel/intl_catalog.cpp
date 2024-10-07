// The SEL extension library
// Free software published under the MIT license.

#include "intl_catalog.hpp"
#include <vector>
#include <mutex>
#include <limits.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>

#if defined(_WIN32)
#include "intl_win32.hpp"
#endif

namespace sel
{
namespace intl
{

char *catalog_entry::get_plural(uint64_t nth) const noexcept
{
    char *text = m_translated;
    if (nth == 0)
        return text;

    if (nth - 1 >= m_extra_plurals)
        return nullptr;

    for (; nth > 0; --nth)
        text += strlen(text) + 1;

    return text;
}

template <typename T, class Hash = std::hash<T>>
static void hash_combine(size_t &seed, const T &val)
{
    seed ^= Hash{}(val) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}

size_t catalog_key_hash::operator()(const catalog_key &key) const noexcept
{
    size_t h = std::hash<int>{}(key.m_category);
    hash_combine(h, std::hash<std::string_view>{}(key.m_message));
    return h;
}

bool catalog_key_equal::operator()(const catalog_key &a, const catalog_key &b) const noexcept
{
    return a.m_category == b.m_category && a.m_message == b.m_message;
}

const char *catalog::lookup(const char *text, int category)
{
    catalog_key key;
    key.m_category = category;
    key.m_message = text;

    auto it = m_strings.find(key);
    if (it == m_strings.end())
        return nullptr;

    catalog_entry &ent = it->second;
    const char *translated = ent.m_translated;
    if (!translated[0])
        return nullptr;

    return ent.m_translated;
}

const char *catalog::plural_lookup(const char *text, const char *plural, unsigned long n, int category)
{
    plural_forms *pf = m_plural.get();
    uint64_t plural_index = n != 1;
    if (pf)
    {
        if (!pf->m_expr_plural.eval(n, &plural_index) || plural_index >= pf->m_num_plurals)
            return nullptr;
    }

    //XXX form the msgid by concatenating
    char *msgid;
    size_t msgid_len;
    char msgid_buf[1024];
    std::unique_ptr<char[]> msgid_dynbuf;
    {
        size_t len1 = strlen(text);
        size_t len2 = strlen(plural);
        msgid = msgid_buf;
        msgid_len = len1 + 1 + len2;
        if (sizeof(msgid_buf) < msgid_len)
        {
            msgid_dynbuf.reset(new char[msgid_len]);
            msgid = msgid_dynbuf.get();
        }
        memcpy(msgid, text, len1);
        msgid[len1] = '\0';
        memcpy(msgid + len1 + 1, plural, len2);
    }

    //
    catalog_key key;
    key.m_category = category;
    key.m_message = std::string_view(msgid, msgid_len);

    auto it = m_strings.find(key);
    if (it == m_strings.end())
        return nullptr;

    catalog_entry &ent = it->second;
    const char *translated = ent.get_plural(plural_index);
    if (!translated || !translated[0])
        return nullptr;

    return translated;
}

bool catalog::load(int category, std::string_view lang, std::shared_lock<std::shared_mutex> &shared_lock)
{
    bool ok = true;

    assert(shared_lock.owns_lock());

    if (m_loaded & (1u << category))
        return ok;

    std::shared_mutex *mutex = shared_lock.mutex();
    shared_lock.unlock();

    {
        std::lock_guard<std::shared_mutex> excl_lock(*mutex);

        if (!(m_loaded & (1u << category)))
        {
            std::string path_buf;
            path_buf.reserve(1024);

            for (std::string_view variant(lang); !variant.empty(); )
            {
#if !defined(_WIN32)
                char sep = '/';
#else
                char sep = '\\';
#endif

                path_buf.assign(m_dir);
                path_buf.push_back(sep);
                path_buf.append(variant);
                path_buf.push_back(sep);
                path_buf.append(string_of_category(category));
                path_buf.push_back(sep);
                path_buf.append(m_domain);
                path_buf.append(".mo");

                if (!load_file_strings(path_buf, category))
                    ok = false;

                size_t pos = variant.find_last_of("_.@");
                variant = std::string_view(
                    variant.data(), (pos == variant.npos) ? 0 : pos);
            }

            m_loaded |= 1u << category;
        }
    }

    shared_lock.lock();

    return ok;
}

static bool char7_isspace(char c)
{
    return c == ' ' || c == '\f' || c == '\n' ||
        c == '\r' || c == '\t' || c == '\v';
}

[[nodiscard]] static std::string_view string7_strip_front(std::string_view x)
{
    while (!x.empty() && char7_isspace(x.front()))
        x.remove_prefix(1);
    return x;
}

[[nodiscard]] static std::string_view string7_strip_back(std::string_view x)
{
    while (!x.empty() && char7_isspace(x.back()))
        x.remove_suffix(1);
    return x;
}

[[nodiscard]] static std::string_view string7_strip(std::string_view x)
{
    return string7_strip_front(string7_strip_back(x));
}

template <class Vis>
static bool string_visit_splits(std::string_view input, char sep, Vis &&vis)
{
    bool more = true;
    size_t index = 0;

    while (more)
    {
        size_t pos = input.find(sep, index);
        more = pos != input.npos;
        if (!vis(input.substr(index, more ? (pos - index) : input.npos)))
            return false;
        if (more)
            index = pos + 1;
    }

    return true;
}

static bool parse_uint(std::string_view text, unsigned int &result)
{
    unsigned int value = 0;
    for (char c : text)
    {
        if (c < '0' || c > '9')
            return false;
        unsigned int digit = c - '0';
        if (value > UINT_MAX / 10 || UINT_MAX - 10 * value < digit)
            return false;
        value = 10 * value + digit;
    }
    result = value;
    return true;
};

bool catalog::load_file_strings(const std::string &path, int category)
{
#if !defined(_WIN32)
    FILE *fh = fopen(path.c_str(), "rb");
#else
    FILE *fh = _wfopen(wstring_from_string(path).c_str(), L"rb");
#endif
    if (!fh)
        return false;

    struct FILE_delete
    {
        void operator()(FILE *fh) const noexcept { fclose(fh); }
    };
    std::unique_ptr<FILE, FILE_delete> fh_cleanup(fh);

    bool little = true;

    auto read_u32 = [&little](FILE *fh, uint32_t *retval) -> bool
    {
        uint8_t data[4];
        if (fread(data, 4, 1, fh) != 1)
            return false;
        if (little)
            *retval = ((uint32_t)data[0]) | ((uint32_t)data[1] << 8) |
                ((uint32_t)data[2] << 16) | ((uint32_t)data[3] << 24);
        else
            *retval = ((uint32_t)data[3]) | ((uint32_t)data[2] << 8) |
                ((uint32_t)data[1] << 16) | ((uint32_t)data[0] << 24);
        return true;
    };

    uint32_t magic;
    if (!read_u32(fh, &magic))
        return false;

    if (magic == 0x950412de)
        little = true;
    else if (magic == 0xde120495)
        little = false;
    else
        return false;

    uint32_t revision;
    if (!read_u32(fh, &revision))
        return false;

    uint32_t major_revision = revision >> 16;
    //uint32_t minor_revision = revision & 0xffff;
    if (major_revision > 1)
        return false;

    uint32_t num_strings;
    uint32_t off_source_table;
    uint32_t off_translated_table;
    if (!read_u32(fh, &num_strings) ||
        !read_u32(fh, &off_source_table) ||
        !read_u32(fh, &off_translated_table))
    {
        return false;
    }

    struct table_entry
    {
        uint32_t len_source;
        uint32_t off_source;
        char *source;
        uint32_t len_translated;
        uint32_t off_translated;
        char *translated;
    };

    std::vector<table_entry> table;
    table.resize(num_strings);

    if (fseek(fh, off_source_table, SEEK_SET) != 0)
        return false;

    for (uint32_t i = 0; i < num_strings; ++i)
    {
        if (!read_u32(fh, &table[i].len_source) ||
            !read_u32(fh, &table[i].off_source))
        {
            return false;
        }
    }

    if (fseek(fh, off_translated_table, SEEK_SET) != 0)
        return false;

    for (uint32_t i = 0; i < num_strings; ++i)
    {
        if (!read_u32(fh, &table[i].len_translated) ||
            !read_u32(fh, &table[i].off_translated))
        {
            return false;
        }
    }

    //---------------------------------------------------------------------------

    {
        std::size_t blob_size = 0;
        for (uint32_t i = 0; i < num_strings; ++i)
            blob_size += table[i].len_source + 1 + table[i].len_translated + 1;

        char *blob = new char[blob_size];
        m_blob.reset(blob);

        char *cursor = blob;

        for (uint32_t i = 0; i < num_strings; ++i)
        {
            uint32_t off = table[i].off_source;
            if (fseek(fh, off, SEEK_SET) != 0)
                return false;

            uint32_t len = table[i].len_source;
            if (fread(cursor, 1, len, fh) != len)
                return false;

            table[i].source = cursor;
            cursor[len] = '\0';
            cursor += len + 1;;
        }

        for (uint32_t i = 0; i < num_strings; ++i)
        {
            uint32_t off = table[i].off_translated;
            if (fseek(fh, off, SEEK_SET) != 0)
                return false;

            uint32_t len = table[i].len_translated;
            if (fread(cursor, 1, len, fh) != len)
                return false;

            table[i].translated = cursor;
            cursor[len] = '\0';
            cursor += len + 1;;
        }
    }

    //---------------------------------------------------------------------------
    std::string_view null_entry;

    for (uint32_t i = 0; i < num_strings; ++i)
    {
        uint32_t len_source = table[i].len_source;
        uint32_t len_translated = table[i].len_translated;

        if (len_source == 0)
        {
            null_entry = std::string_view(table[i].translated, len_translated);
            continue;
        }

        if (len_source == 0 || len_translated == 0)
            continue;

        catalog_entry ent;
        ent.m_source = table[i].source;
        ent.m_translated = table[i].translated;

        // plural forms
        {
            char *txt = ent.m_translated;
            char *cur = txt, *end = txt + len_translated;
            uint32_t count = 0;
            while ((cur = (char *)memchr(cur, '\0', end - cur)))
            {
                ++cur;
                /*cur*/
                ++count;
            }
            ent.m_extra_plurals = count;
        }

        catalog_key key;
        key.m_category = category;
        key.m_message = std::string_view(ent.m_source, len_source);
        m_strings.insert(std::make_pair(key, std::move(ent)));
    }

    string_visit_splits(null_entry, '\n', [this](std::string_view line)
    {
        size_t colon_pos = line.find(':');
        if (colon_pos != line.npos)
        {
            std::string_view line_key = string7_strip(line.substr(0, colon_pos));
            std::string_view line_val = string7_strip(line.substr(colon_pos + 1));

            if (line_key == "Plural-Forms")
            {
                std::string_view nplurals;
                std::string_view plural;

                string_visit_splits(line_val, ';', [&nplurals, &plural](std::string_view chunk) -> bool
                {
                    size_t equal_pos = chunk.find('=');
                    if (equal_pos != chunk.npos)
                    {
                        std::string_view key = string7_strip(chunk.substr(0, equal_pos));
                        std::string_view val = string7_strip(chunk.substr(equal_pos + 1));
                        if (key == "nplurals")
                            nplurals = val;
                        else if (key == "plural")
                            plural = val;
                    }
                    return true;
                });

                std::unique_ptr<plural_forms> pf(new plural_forms);
                pf->m_expr_plural = plural_expr(plural);
                if (pf->m_expr_plural.valid() &&
                    parse_uint(nplurals, pf->m_num_plurals) && pf->m_num_plurals > 0)
                {
                    m_plural = std::move(pf);
                }
                else
                {
                    //error
                }
            }
        }
        return true;
    });

    return true;
}

std::string_view catalog::string_of_category(int category)
{
    switch (category)
    {
    default: return {};

    #define X(category) case category: return #category

    X(LC_CTYPE);
    X(LC_NUMERIC);
    X(LC_TIME);
    X(LC_COLLATE);
    X(LC_MONETARY);
    X(LC_MESSAGES);
    X(LC_ALL);

    #if 0
    X(LC_PAPER);
    X(LC_NAME);
    X(LC_ADDRESS);
    X(LC_TELEPHONE);
    X(LC_MEASUREMENT);
    X(LC_IDENTIFICATION);
    #endif

    #undef X
    }
}

}
// namespace intl
}
// namespace sel
