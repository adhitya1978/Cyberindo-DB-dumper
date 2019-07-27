#include "dbbrowser.h"

DBBrowser::DBBrowser()
{
    /*Support driver: QMYSQL3, QMYSQL, QSQLITE, SQLITECIPHER*/
    m_db = QSqlDatabase::addDatabase("SQLITECIPHER");
}

bool DBBrowser::open(const QString &db, bool readOnly)
{
    if(isopen()) close();
    m_db.setDatabaseName(db);  // set db name

    // open database
    if(!m_db.open())
    {
        lastErrorMessage = QString("DBBrowser:open():%1 failed. error: %2").arg(db).arg(m_db.lastError().text());
        return false;
    }
    curDBFilename = db;

    return true;
}

bool DBBrowser::attach(const QString &filename, QString attach_as)
{
    // Ask for name to be given to the attached database if none was provided
    if(attach_as.isEmpty())
        lastErrorMessage = QString("Please specify the database name under which you want to access the attached database").trimmed();

    if(attach_as.isEmpty())
        return false;

    return true;


}

bool DBBrowser::create(const QString &db)
{
    if (isopen()) close();
    // read encoding from settings and open with sqlite3_open for utf8 and sqlite3_open16 for utf16
    QString sEncoding = "UTF-8";

    int openresult = SQLITECIPHER_OK;

    if(sEncoding == "UTF-8")
        m_db.setDatabaseName(db);
    if(!m_db.open())
    {
        lastErrorMessage = QString("create(%1). error:%%%2").arg(db).arg(m_db.lastError().text());
        return false;
    }

    curDBFilename = db;
    return true;

}

bool DBBrowser::close()
{
    if(m_db.isOpen())
    {
        m_db.close();
    }
    return true;
}

bool DBBrowser::executeSQL(QString statement)
{

    if(!isopen()) return false;

    statement = statement.trimmed(); /*clear space*/

    QSqlQuery query(m_db);

    if(query.exec(statement) != SQLITECIPHER_OK)
    {
        lastErrorMessage = QString("executeSQL(). error: %1").arg(query.lastError().text());
        return false;
    }
    return true;

}

bool DBBrowser::executeMultiSQL(const QString &statement, bool dirty, bool log)
{
    // First check if a DB is opened
    if(!isopen())
        return false;

    QString query = statement.trimmed();
    // Check if this SQL containts any transaction statements
    QRegExp transactionRegex("^\\s*BEGIN TRANSACTION;|COMMIT;\\s*$");

    if(query.contains(transactionRegex))
    {
        // If so remove them anc create a savepoint instead by overriding the dirty parameter
        query.remove(transactionRegex);
        dirty = true;
    }
    // Log the statement if needed
    //if(log)        logSQL(query, kLogMsg_App);

    /** Set DB to dirty/create restore point if necessary
    QString savepoint_name;

    if(dirty)
    {
        savepoint_name = generateSavepointName("execmultisql");
        setSavepoint(savepoint_name);
    }
     Execute the statement by looping until SQLite stops giving back a tail string

     **/
    QByteArray utf8Query = query.toUtf8();
    const char *tail = utf8Query.data();
    int res = SQLITECIPHER_OK;
    unsigned int line = 0;
    QSqlQuery m_query(m_db);

    while(tail && *tail != 0 && (res == SQLITECIPHER_OK || res == SQLITECIPHER_DONE))
    {
        line++;
        // Check whether the DB structure is changed by this statement
        QString qtail = QString(tail);
        // Execute next statement
        res = m_query.prepare(qtail);
        if(res == SQLITECIPHER_OK)
        {
            if(!m_query.exec())
            {
                // at least know which statement have error
                lastErrorMessage =QString("ExecutingMultiSQL(). error:%1 with statement: %2").arg(m_query.lastError().text()).arg(qtail);
                m_query.finish();
                return false;
                break;
            }
            if(!m_query.next())
            {
                lastErrorMessage = QObject::tr("Error in statement #%1: %2.\n"
                    "Aborting execution.").arg(line).arg(m_query.lastError().text());
                return false;
            }
            else
            {
                m_query.finish();
            }
        }
        else
        {
            lastErrorMessage = QObject::tr("Error in statement #%1: %2.\n"
                "Aborting execution.").arg(line).arg(m_query.lastError().text());
            return false;
        }
    }
     // Exit
    return true;

}

QString DBBrowser::getRow(const QString &sTableName, const QString &rowid, QList<QByteArray> &rowdata)
{
    QSqlDriver *m_QDriver = m_db.driver();

    return QString("SELECT * FROM %1 WHERE %2='%3';")
            .arg(m_QDriver->escapeIdentifier(sTableName, QSqlDriver::TableName))
            .arg(m_QDriver->escapeIdentifier(m_db.record(sTableName).field(sTableName).name(), QSqlDriver::FieldName))
            .arg(rowid);
}


// DELETE FROM %1 WHERE %2 IN (%3);
QString DBBrowser::deleteRecords(const QString &sTableName,QString &columsName, const QStringList& rowids)
{
    QStringList quoted_rowids;
    foreach(QString rowid, rowids)
    {
        quoted_rowids.append("'" + rowid + "'");
    }
    QString statement = QString("DELETE FROM %1 WHERE %2 IN (%3);")
            .arg(escapeIdentifier(sTableName))
            .arg(escapeIdentifier(columsName))
            .arg(quoted_rowids.join(", "));

    return statement;
}

QString DBBrowser::renameTable(const QString &from_table, const QString &to_table)
{
    return QString("ALTER TABLE %1 RENAME TO %2").arg(escapeIdentifier(from_table)).arg(escapeIdentifier(to_table));
}


void DBBrowser::updateSchema()
{

}

QSqlDatabase DBBrowser::getDb() const
{
    return m_db;
}

bool DBBrowser::dump(const QString &filename, const QStringList &tablesToDump, bool insertColNames, bool insertNew, bool exportSchema, bool exportData, bool keepOldSchema)
{
    size_t numRecordsTotal = 0, numRecordsCurrent = 0;

    QSqlQueryModel *model;
    // Open file
    QFile *file = new QFile(filename);


    if(!file->open(QIODevice::WriteOnly))
    {
        lastErrorMessage = QString("dump(). error:%1").arg(file->errorString());
        return false;
    }
    model = new QSqlQueryModel;

    return false;
}
