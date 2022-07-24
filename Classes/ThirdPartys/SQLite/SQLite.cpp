#include "SQLite.h"
#include "../XXTea/EncryptionManager.h"

/**
 @brief  Constructor
 @param  -
 @return -
*/


SQLite::SQLite()
{
    database = NULL;
}


/**
 @brief    Get always the same instance of the object (Singleton)
 @param    -
 @return   SQLite*
*/

SQLite* SQLite::getInstance()
{
    static SQLite sharedInstance;
    return &sharedInstance;
}


/**
 @brief   Open the SQLite database
 @param   -
 @return  bool
*/

bool SQLite::openDatabase()
{
    string filePath = FileUtils::getInstance()->getWritablePath() + "db.sqlite3";
    SQLite *sqlite = SQLite::getInstance();
    
    if (sqlite3_open(filePath.c_str(), &sqlite->database) != SQLITE_OK)
    {
        sqlite3_close(sqlite->database);
        CCLOG("Failed to open database: %s", filePath.c_str());
        return false;
    }
    
    return true;
}


/**
 @brief   Close database
 @param   -
 @return  -
*/

void SQLite::closeDatabase()
{
    sqlite3_close(SQLite::getInstance()->database);
    database = NULL;
}


/**
 @brief  Exec a query
 @param  query: sql query
 @return bool
*/

bool SQLite::execQuery(string query)
{
    char *errorMsg;
    SQLite *sqlite = SQLite::getInstance();
    
    //Reset / re-init queryResult
    while(!sqlite->queryResult.empty())
        sqlite->queryResult.pop_back();
    
    if (sqlite3_exec(sqlite->database, query.c_str(), callback, 0, &errorMsg) != SQLITE_OK)
    {
        log("Error executing SQL statement: %s", errorMsg);
        return false;
    }
    
    return true;
}


/**
 @brief   Parse a value of queryResult. Format : [Column]=Value
 @param   string: string to parse
 @return  string
*/

string SQLite::parseValue(string value)
{
    unsigned long pos = value.find("=");
    if(pos != string::npos)
        return value.substr(pos+1);
    
    return "";
}


/**
 @brief   It's called by execQuery. It storages the result of a query. It's called N times, where N is the number of registers of the query.
 @param   data, argc, argv, azColname
 @return  0
*/

static int callback(void *data, int argc, char **argv, char **azColName)
{
    for(int i = 0; i < argc; i++) //Number of columns
    {
        string columnName(azColName[i]); //Column name
        string columnValue = argv[i] ? argv[i] : "NULL"; //Column value
        string data = "[" + columnName + "]=" + columnValue; //Format: [COLUMN]=VALUE
        
        //queryResult has N values, where N is the number of the columns of the result. Format: [COLUMN]=VALUE.
        //Note: We're supposing that always the result will contain 1 register ( simple query )
        SQLite::getInstance()->queryResult.push_back(data);
    }
    
    return 0;
}


/**
 @brief  Create  the SQLite Tables if is the first time that I play.
 @param  -
 @return -
*/

void SQLite::createTablesIfNotExists()
{
    openDatabase();

    //The table 'TaBnAeYvI' is the LoginInfo table. Column MT = LoginMethod (Mail or Facebook) / Column UR = User / Column PD = Password
    execQuery("CREATE TABLE IF NOT EXISTS TaBnAeYvI (MT varchar(255), UR varchar(255), PD varchar(255));"); //255 chars because the data is stored encrypted
  
    closeDatabase();
}


/**
 @brief  Get login method (Mail or Facebook)
 @param  -
 @return string
*/

string SQLite::getLoginMethod()
{
    string data = "";
    EncryptionManager *encryptionManager = EncryptionManager::getInstance();
    
    //The table 'TaBnAeYvI' is the LoginInfo table. And the Column 'MT' is the LoginMethod column.
    openDatabase();
    bool queryExecuted = execQuery("SELECT MT FROM TaBnAeYvI");
    
    if(queryExecuted and !queryResult.empty())
        data = encryptionManager->decryptString(parseValue(queryResult.at(0)));
    
    closeDatabase();
    return data;
}


/**
 @brief  Get Nakama user
 @param  -
 @return string
*/

string SQLite::getNakamaUser()
{
    string data = "";
    EncryptionManager *encryptionManager = EncryptionManager::getInstance();
    
    //The table 'TaBnAeYvI' is the LoginInfo table. And the Column 'UR' is the User column.
    openDatabase();
    bool queryExecuted = execQuery("SELECT UR FROM TaBnAeYvI");
    
    if(queryExecuted and !queryResult.empty())
        data = encryptionManager->decryptString(parseValue(queryResult.at(0)));
    
    closeDatabase();
    return data;
}


/**
 @brief  Get Nakama password
 @param  -
 @return string
*/

string SQLite::getNakamaPassword()
{
    string data = "";
    EncryptionManager *encryptionManager = EncryptionManager::getInstance();
    
    //The table 'TaBnAeYvI' is the LoginInfo table. And the Column 'PD' is the Password column.
    openDatabase();
    bool queryExecuted = execQuery("SELECT PD FROM TaBnAeYvI");
    
    if(queryExecuted and !queryResult.empty())
        data = encryptionManager->decryptString(parseValue(queryResult.at(0)));
    
    closeDatabase();
    return data;
}


/**
 @brief  Set the Login Method (Mail or Facebook)
 @param  loginMethod: mail or facebook
 @return bool
*/

bool SQLite::setLoginMethod(string loginMethod)
{
    bool queryExecuted;
    EncryptionManager *encryptionManager = EncryptionManager::getInstance();
    
    //The table 'TaBnAeYvI' is the LoginInfo table. And the Column 'MT' is the LoginMethod column.
    openDatabase();
    
    if(execQuery("SELECT COUNT(*) FROM TaBnAeYvI") and parseValue(queryResult.at(0)) == "0")
        queryExecuted = execQuery("INSERT INTO TaBnAeYvI (MT) values ('" + encryptionManager->encryptString(loginMethod) + "')");
    else
        queryExecuted = execQuery("UPDATE TaBnAeYvI SET MT='" + encryptionManager->encryptString(loginMethod) + "'");
    
    closeDatabase();
    
    return queryExecuted;
}


/**
 @brief  Set nakama user
 @param  user: nakama user
 @return bool
*/

bool SQLite::setNakamaUser(string user)
{
    bool queryExecuted;
    EncryptionManager *encryptionManager = EncryptionManager::getInstance();
    
    //The table 'TaBnAeYvI' is the LoginInfo table. And the Column 'UR' is the User column.
    openDatabase();
    
    if(execQuery("SELECT COUNT(*) FROM TaBnAeYvI") and parseValue(queryResult.at(0)) == "0")
        queryExecuted = execQuery("INSERT INTO TaBnAeYvI (UR) values ('" + encryptionManager->encryptString(user) + "')");
    else
        queryExecuted = execQuery("UPDATE TaBnAeYvI SET UR='" + encryptionManager->encryptString(user) + "'");
    
    closeDatabase();
    
    return queryExecuted;
}


/**
 @brief  Set nakama password
 @param  password: nakama password
 @return bool
*/

bool SQLite::setNakamaPassword(string password)
{
    bool queryExecuted;
    EncryptionManager *encryptionManager = EncryptionManager::getInstance();
    
    //The table 'TaBnAeYvI' is the LoginInfo table. And the Column 'PD' is the Password column.
    openDatabase();
    
    if(execQuery("SELECT COUNT(*) FROM TaBnAeYvI") and parseValue(queryResult.at(0)) == "0")
        queryExecuted = execQuery("INSERT INTO TaBnAeYvI (PD) values ('" + encryptionManager->encryptString(password) + "')");
    else
        queryExecuted = execQuery("UPDATE TaBnAeYvI SET PD='" + encryptionManager->encryptString(password) + "'");
    
    closeDatabase();
    
    return queryExecuted;
}


/**
 @brief  Destructor
 @param  -
 @return -
*/

SQLite::~SQLite()
{
}
