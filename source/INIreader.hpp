//
//  INIreader.hpp
//  INIreader
//
//  Created by Manuel Diaz on 5/1/20.
//  Copyright © 2020 WME7. All rights reserved.
//
//  Based on the benhoyt/inih github project:
//  https://github.com/benhoyt/inih
//

#ifndef INIreader_h
#define INIreader_h

#include <map>
#include <set>
#include <string>
#include <algorithm>
#include <cctype>
#include <cstdlib>

/* Make this header file easier to include in C++ code */
#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>

/* Parse given INI-style file. May have [section]s, name=value pairs
   (whitespace stripped), and comments starting with ';' (semicolon). Section
   is "" if name=value pair parsed before any section heading. name:value
   pairs are also supported as a concession to Python's ConfigParser.
   For each name=value pair parsed, call handler function with given user
   pointer as well as section, name, and value (data only valid for duration
   of handler call). Handler should return nonzero on success, zero on error.
   Returns 0 on success, line number of first error on parse error (doesn't
   stop on first error), -1 on file open error, or -2 on memory allocation
   error (only when INI_USE_STACK is zero).
*/
int ini_parse(const char* filename,
              int (*handler)(void* user, const char* section,
                             const char* name, const char* value),
              void* user);

/* Same as ini_parse(), but takes a FILE* instead of filename. This doesn't
   close the file when it's finished -- the caller must do that. */
int ini_parse_file(FILE* file,
                   int (*handler)(void* user, const char* section,
                                  const char* name, const char* value),
                   void* user);

/* Nonzero to allow multi-line value parsing, in the style of Python's
   ConfigParser. If allowed, ini_parse() will call the handler with the same
   name for each subsequent line parsed. */
#ifndef INI_ALLOW_MULTILINE
#define INI_ALLOW_MULTILINE 1
#endif

/* Nonzero to allow a UTF-8 BOM sequence (0xEF 0xBB 0xBF) at the start of
   the file. See http://code.google.com/p/inih/issues/detail?id=21 */
#ifndef INI_ALLOW_BOM
#define INI_ALLOW_BOM 1
#endif

/* Nonzero to use stack, zero to use heap (malloc/free). */
#ifndef INI_USE_STACK
#define INI_USE_STACK 1
#endif

/* Stop parsing on first error (default is to keep parsing). */
#ifndef INI_STOP_ON_FIRST_ERROR
#define INI_STOP_ON_FIRST_ERROR 0
#endif

/* Maximum line length for any line in INI file. */
#ifndef INI_MAX_LINE
#define INI_MAX_LINE 200
#endif




#include <ctype.h>
#include <string.h>

#if !INI_USE_STACK
#include <stdlib.h>
#endif

#define MAX_SECTION 50
#define MAX_NAME 50

/* Strip whitespace chars off end of given string, in place. Return s. */
static char* rstrip(char* s)
{
    char* p = s + strlen(s);
    while (p > s && isspace((unsigned char)(*--p)))
        *p = '\0';
    return s;
}

/* Return pointer to first non-whitespace char in given string. */
static char* lskip(const char* s)
{
    while (*s && isspace((unsigned char)(*s)))
        s++;
    return (char*)s;
}

/* Return pointer to first char c or ';' comment in given string, or pointer to
   null at end of string if neither found. ';' must be prefixed by a whitespace
   character to register as a comment. */
static char* find_char_or_comment(const char* s, char c)
{
    int was_whitespace = 0;
    while (*s && *s != c && !(was_whitespace && *s == ';')) {
        was_whitespace = isspace((unsigned char)(*s));
        s++;
    }
    return (char*)s;
}

/* Version of strncpy that ensures dest (size bytes) is null-terminated. */
static char* strncpy0(char* dest, const char* src, size_t size)
{
    strncpy(dest, src, size);
    dest[size - 1] = '\0';
    return dest;
}

/* See documentation in header file. */
int ini_parse_file(FILE* file,
                   int (*handler)(void*, const char*, const char*,const char*),
                   void* user)
{
    /* Uses a fair bit of stack (use heap instead if you need to) */
#if INI_USE_STACK
    char line[INI_MAX_LINE];
#else
    char* line;
#endif
    char section[MAX_SECTION] = "";
    char prev_name[MAX_NAME] = "";

    char* start;
    char* end;
    char* name;
    char* value;
    int lineno = 0;
    int error = 0;

#if !INI_USE_STACK
    line = (char*)malloc(INI_MAX_LINE);
    if (!line) {
        return -2;
    }
#endif

    /* Scan through file line by line */
    while (fgets(line, INI_MAX_LINE, file) != NULL) {
        lineno++;

        start = line;
#if INI_ALLOW_BOM
        if (lineno == 1 && (unsigned char)start[0] == 0xEF &&
                           (unsigned char)start[1] == 0xBB &&
                           (unsigned char)start[2] == 0xBF) {
            start += 3;
        }
#endif
        start = lskip(rstrip(start));

        if (*start == ';' || *start == '#') {
            /* Per Python ConfigParser, allow '#' comments at start of line */
        }
#if INI_ALLOW_MULTILINE
        else if (*prev_name && *start && start > line) {
            /* Non-black line with leading whitespace, treat as continuation
               of previous name's value (as per Python ConfigParser). */
            if (!handler(user, section, prev_name, start) && !error)
                error = lineno;
        }
#endif
        else if (*start == '[') {
            /* A "[section]" line */
            end = find_char_or_comment(start + 1, ']');
            if (*end == ']') {
                *end = '\0';
                strncpy0(section, start + 1, sizeof(section));
                *prev_name = '\0';
            }
            else if (!error) {
                /* No ']' found on section line */
                error = lineno;
            }
        }
        else if (*start && *start != ';') {
            /* Not a comment, must be a name[=:]value pair */
            end = find_char_or_comment(start, '=');
            if (*end != '=') {
                end = find_char_or_comment(start, ':');
            }
            if (*end == '=' || *end == ':') {
                *end = '\0';
                name = rstrip(start);
                value = lskip(end + 1);
                end = find_char_or_comment(value, '\0');
                if (*end == ';')
                    *end = '\0';
                rstrip(value);

                /* Valid name[=:]value pair found, call handler */
                strncpy0(prev_name, name, sizeof(prev_name));
                if (!handler(user, section, name, value) && !error)
                    error = lineno;
            }
            else if (!error) {
                /* No '=' or ':' found on name[=:]value line */
                error = lineno;
            }
        }

#if INI_STOP_ON_FIRST_ERROR
        if (error)
            break;
#endif
    }

#if !INI_USE_STACK
    free(line);
#endif

    return error;
}

/* See documentation in header file. */
int ini_parse(const char* filename,
              int (*handler)(void*, const char*, const char*, const char*),
              void* user)
{
    FILE* file;
    int error;

    file = fopen(filename, "r");
    if (!file)
        return -1;
    error = ini_parse_file(file, handler, user);
    fclose(file);
    return error;
}

#ifdef __cplusplus
}
#endif

// Read an INI file into easy-to-access name/value pairs. (Note that I've gone
// for simplicity here rather than speed, but it should be pretty decent.)
class INIReader
{
public:
    // Construct INIReader and parse given filename. See ini.h for more info
    // about the parsing.
    INIReader(std::string filename)
    {
        _error = ini_parse(filename.c_str(), ValueHandler, this);
    }
    
    // Destructor
    ~INIReader()
    {
        // Clean up the field sets
        std::map<std::string, std::set<std::string>*>::iterator fieldSetsIt;
        for (fieldSetsIt = _fields.begin(); fieldSetsIt != _fields.end(); ++fieldSetsIt)
            delete fieldSetsIt->second;
    }

    // Return the result of ini_parse(), i.e., 0 on success, line number of
    // first error on parse error, or -1 on file open error.
    int ParseError()
    {
        return _error;
    }

    // Get a string value from INI file, returning default_value if not found.
    std::string Get(std::string section, std::string name,std::string default_value)
    {
        std::string key = MakeKey(section, name);
        std::string value = _values.count(key) ? _values[key] : default_value;

        if (!value.empty() && (value.front() == '"' || value.front() == '\'')) {
            value.erase(0, 1);
        }
        if (!value.empty() && (value.back() == '"' || value.back() == '\'')) {
            value.pop_back();
        }

        return value;
    }

    // Get an integer (long) value from INI file, returning default_value if
    // not found or not a valid integer (decimal "1234", "-1234", or hex "0x4d2").
    long GetInteger(std::string section, std::string name, long default_value)
    {
        std::string valstr = Get(section, name, "");
        const char* value = valstr.c_str();
        char* end;
        // This parses "1234" (decimal) and also "0x4D2" (hex)
        long n = strtol(value, &end, 0);
        return end > value ? n : default_value;
    }

    // Get a real (floating point double) value from INI file, returning
    // default_value if not found or not a valid floating point value
    // according to strtod().
    double GetReal(std::string section, std::string name, double default_value)
    {
        std::string valstr = Get(section, name, "");
        const char* value = valstr.c_str();
        char* end;
        double n = strtod(value, &end);
        return end > value ? n : default_value;
    }

    // Get a boolean value from INI file, returning default_value if not found or if
    // not a valid true/false value. Valid true values are "true", "yes", "on", "1",
    // and valid false values are "false", "no", "off", "0" (not case sensitive).
    bool GetBoolean(std::string section, std::string name, bool default_value)
    {
        std::string valstr = Get(section, name, "");
        // Convert to lower case to make string comparisons case-insensitive
        std::transform(valstr.begin(), valstr.end(), valstr.begin(), ::tolower);
        if (valstr == "true" || valstr == "yes" || valstr == "on" || valstr == "1")
            return true;
        else if (valstr == "false" || valstr == "no" || valstr == "off" || valstr == "0")
            return false;
        else
            return default_value;
    }

    // Returns all the section names from the INI file, in alphabetical order, but in the
    // original casing
    std::set<std::string> GetSections() const
    {
        return _sections;
    }

    // Returns all the field names from a section in the INI file, in alphabetical order,
    // but in the original casing. Returns an empty set if the field name is unknown
    std::set<std::string> GetFields(std::string section) const
    {
        std::string sectionKey = section;
        std::transform(sectionKey.begin(), sectionKey.end(), sectionKey.begin(), ::tolower);
        std::map<std::string, std::set<std::string>*>::const_iterator fieldSetIt = _fields.find(sectionKey);
        if(fieldSetIt==_fields.end())
            return std::set<std::string>();
        return *(fieldSetIt->second);
    }

private:
    
    int _error;
    std::map<std::string, std::string> _values;
    // Because we want to retain the original casing in _fields, but
    // want lookups to be case-insensitive, we need both _fields and _values
    std::set<std::string> _sections;
    std::map<std::string, std::set<std::string>*> _fields;
    
    static std::string MakeKey(std::string section, std::string name)
    {
        std::string key = section + "=" + name;
        // Convert to lower case to make section/name lookups case-insensitive
        std::transform(key.begin(), key.end(), key.begin(), ::tolower);
        return key;
    }
    
    static int ValueHandler(void* user, const char* section, const char* name, const char* value)
    {
        INIReader* reader = (INIReader*)user;

        // Add the value to the lookup map
        std::string key = MakeKey(section, name);
        if (reader->_values[key].size() > 0)
            reader->_values[key] += "\n";
        reader->_values[key] += value;

        // Insert the section in the sections set
        reader->_sections.insert(section);

        // Add the value to the values set
        std::string sectionKey = section;
        std::transform(sectionKey.begin(), sectionKey.end(), sectionKey.begin(), ::tolower);

        std::set<std::string>* fieldsSet;
        std::map<std::string, std::set<std::string>*>::iterator fieldSetIt = reader->_fields.find(sectionKey);
        if(fieldSetIt==reader->_fields.end())
        {
            fieldsSet = new std::set<std::string>();
            reader->_fields.insert ( std::pair<std::string, std::set<std::string>*>(sectionKey,fieldsSet) );
        } else {
            fieldsSet=fieldSetIt->second;
        }
        fieldsSet->insert(name);

        return 1;
    }
};

#endif /* INIreader_h */
