#ifndef COMMON_VALIDATION_H
#define COMMON_VALIDATION_H

#include <iostream>
#include <string>
#include <cctype>
#include <climits>
#include <cstdlib>
using namespace std;

inline string trim(const string& text)
{
    size_t start = 0;
    size_t end = text.size();

    while (start < end && isspace(static_cast<unsigned char>(text[start])))
    {
        ++start;
    }

    while (end > start && isspace(static_cast<unsigned char>(text[end - 1])))
    {
        --end;
    }

    return text.substr(start, end - start);
}

inline string toLower(const string& text)
{
    string result = text;

    for (size_t i = 0; i < result.size(); ++i)
    {
        result[i] = static_cast<char>(tolower(static_cast<unsigned char>(result[i])));
    }

    return result;
}

inline bool containsIgnoreCase(const string& haystack, const string& needle)
{
    return toLower(haystack).find(toLower(needle)) != string::npos;
}

inline bool parseInt(const string& text, int& value)
{
    string input = trim(text);

    if (input.empty())
    {
        return false;
    }

    size_t index = 0;
    bool negative = false;

    if (input[index] == '+' || input[index] == '-')
    {
        negative = (input[index] == '-');
        ++index;
    }

    if (index == input.size())
    {
        return false;
    }

    long long result = 0;

    for (; index < input.size(); ++index)
    {
        if (!isdigit(static_cast<unsigned char>(input[index])))
        {
            return false;
        }

        result = result * 10 + (input[index] - '0');

        if (result > INT_MAX)
        {
            return false;
        }
    }

    value = negative ? -static_cast<int>(result) : static_cast<int>(result);
    return true;
}

inline bool parseDouble(const string& text, double& value)
{
    string input = trim(text);

    if (input.empty())
    {
        return false;
    }

    char* end = nullptr;
    value = strtod(input.c_str(), &end);
    return end != input.c_str() && *end == '\0';
}

inline string fitText(const string& text, size_t width)
{
    if (text.size() <= width)
    {
        return text;
    }

    if (width <= 3)
    {
        return text.substr(0, width);
    }

    return text.substr(0, width - 3) + "...";
}

inline bool readLine(const string& prompt, string& output)
{
    cout << prompt;

    if (!getline(cin, output))
    {
        return false;
    }

    output = trim(output);
    return true;
}

inline bool readNonEmptyLine(const string& prompt, string& output)
{
    while (readLine(prompt, output))
    {
        if (!output.empty())
        {
            return true;
        }

        cout << "Input cannot be empty." << endl;
    }

    return false;
}

inline bool readInt(const string& prompt, int& output)
{
    string line;

    while (readLine(prompt, line))
    {
        if (parseInt(line, output))
        {
            return true;
        }

        cout << "Invalid number. Please try again." << endl;
    }

    return false;
}

inline bool readYesNo(const string& prompt)
{
    string answer;

    if (!readLine(prompt, answer))
    {
        return false;
    }

    return answer == "y" || answer == "Y" || answer == "yes" || answer == "Yes" || answer == "YES";
}

#endif
