#ifndef DBBROWSER_H
#define DBBROWSER_H

#include <QString>
#include <QtSql>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlDriver>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlError>
#include <QtSql/QSqlField>
#include <QtSql/QSqlQueryModel>
#include <QSharedPointer>
#include <QObject>
#include <QList>
#include <QMap>
#include <QMultiMap>

#define SQLITECIPHER_OK     0  /* Successful result */
#define SQLITECIPHER_DONE        101  /* sqlite3_step() has finished executing */

enum
{
    kLogMsg_User,
    kLogMsg_App
};


typedef QMultiMap<QString, class DBBrowserObject> objectMap;

class DBBrowserObject
{
public:
    DBBrowserObject() : name( "" ) { }
    DBBrowserObject( const QString& wname,const QString& wsql, const QString& wtype, const QString& tbl_name )
        : name( wname), sql( wsql ), type(wtype), table_name(tbl_name)
    { }

    //void addField(sqlb::FieldPtr field) { fldmap.push_back(field); }

    QString getname() const { return name; }
    QString getsql() const { return sql; }
    QString gettype() const { return type; }
    QString getTableName() const { return table_name; }

private:
    QString name;
    QString sql;
    QString type;
    QString table_name;     // The name of the table this object references, interesting for views, triggers and indices
};


class DBBrowser : public QSqlDatabase
{
public:
    DBBrowser();

    bool isopen() {return m_db.isOpen();}

    bool open(const QString& db, bool readOnly = false);
    bool attach(const QString& filename, QString attach_as = "");
    bool create ( const QString & db);
    bool close();
    bool dump(const QString & filename, const QStringList &tablesToDump, bool insertColNames, bool insertNew, bool exportSchema, bool exportData, bool keepOldSchema);
    bool executeSQL(QString statement);
    bool executeMultiSQL(const QString& statement, bool dirty = true, bool log = false);
    int addRecord(const QString& sTableName);
    const QString& lastError() const { return lastErrorMessage; }

    QString escapeIdentifier(QString id)
    {
        return '`' + id.replace('`', "``") + '`';
    }


    /**
     * @brief getRow Executes a sqlite statement to get the rowdata(columns)
     *        for the given rowid.
     * @param sTableName Table to query.
     * @param rowid The rowid to fetch.
     * @param rowdata A list of QByteArray containing the row data.
     * @return true if statement execution was ok, else false.
     */
    QString getRow(const QString& sTableName, const QString& rowid, QList<QByteArray>& rowdata);
    QString deleteRecords(const QString& sTableName, QString &columsName, const QStringList &rowId);
    QString renameTable(const QString& from_table, const QString& to_table);


    void updateSchema();

    QSqlDatabase getDb() const;

private:
    QString lastErrorMessage;
    QSqlDatabase m_db;
    QString curDBFilename;

};

#endif // DBBROWSER_H
