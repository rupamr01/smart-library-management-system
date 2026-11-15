#ifndef STORAGE_H
#define STORAGE_H

#include <string>
using namespace std;

struct LoadReport
{
    bool fatal = false;
    bool hadWarnings = false;
    int books = 0;
    int students = 0;
    int transactions = 0;
    int reservations = 0;
    string warnings;
};

bool loadAllData(const string& dataDir, LoadReport& report);
bool saveAllData(const string& dataDir, string& error);

#endif
