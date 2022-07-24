#ifndef __SQLITE_H__
#define __SQLITE_H__

#include "cocos2d.h"
#include "sqlite3.h"

static int callback(void *data, int argc, char **argv, char **azColName);

using namespace std;

class SQLite
{
private:
    sqlite3 *database;
    
    bool openDatabase();
    void closeDatabase();
    
    bool execQuery(string query);
    string parseValue(string value);
  
public:
    SQLite();
    ~SQLite();
    static SQLite* getInstance();
  
    vector<string> queryResult;
    void createTablesIfNotExists();
    
    //Get SQLite Info
    string getLoginMethod();
    string getNakamaUser();
    string getNakamaPassword();
    
    //Set SQLite Info
    bool setLoginMethod(string loginMethod);
    bool setNakamaUser(string user);
    bool setNakamaPassword(string password);
};

#endif
