
#include <QtCore/QCoreApplication>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlDriver>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlError>
#include <QtSql/QSqlQueryModel>
#include <QVariant>
#include <QDebug>
#include <QStringList>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QFileInfoList>
#include <QtCore/QtCore>
#include <QList>
#include <QFile>
#include <QTextStream>



#include "readerini.h"
#include "dbbrowser.h"

static QString m_Root_Directory ;
static QStringList db_list_name;
static QString m_activeDb;
static DBBrowser *m_dbBrowser = NULL;
static bool done;

int openDb(QString dbName, QString querycmd, QString fname)
{
    bool exported = fname.length() > 0;

    QFile *csvfile = new QFile(fname);

    /*open database*/
    if(!m_dbBrowser->open(dbName, false))
    {
        qDebug() << m_dbBrowser->lastError();
        return -1;
    }

    /*initial database*/
    QSqlQuery query(m_dbBrowser->getDb());

    /*preparing the query command*/
    if(!query.prepare(querycmd))
    {
        qDebug() << query.lastError();
        m_dbBrowser->close();
        return -1;
    }
    /*execute the query command*/
    if(!query.exec(querycmd))
    {
        qDebug() << query.lastError();
        m_dbBrowser->close();
        return -1;
    }

    /*represent pointer sql as table model*/
    QSqlQueryModel *m_Model = new QSqlQueryModel;
    m_Model->setQuery(query);

    //qDebug() << QString("Row.count = %1").arg(m_Model->rowCount());
    //qDebug() << QString("Columns.count = %1").arg(m_Model->columnCount());


    /*activate read more record */
    while(m_Model->canFetchMore())
        m_Model->fetchMore();

    /*if no assign csv file, preview to console*/
    if(!exported)
    {
        if(m_Model->rowCount() > 0 && m_Model->columnCount() > 0)
        {
            for(int r =0; r < m_Model->rowCount(); r++)
            {
                for (int j = 0; j < m_Model->columnCount(); j++)
                {
                    //qDebug() << QString("Position Row:%1, Position Column: %2").arg(r).arg(j);
                    qDebug() << m_Model->data(m_Model->index(r,j)).toString();
                }
            }
        }      
    }
    else
    {
        if(!csvfile->open(QIODevice::ReadWrite))
            return -1;/*failed to create or read assigned file*/
        QTextStream data(csvfile);
        QStringList strlist;
        /*write header*/
        for (int j = 0; j < m_Model->columnCount(); j++)
        {
            if(m_Model->headerData(j, Qt::Horizontal,Qt::DisplayRole).toString().length() > 0)
                strlist.append("\"" + m_Model->headerData(j, Qt::Horizontal, Qt::DisplayRole).toString() + "\"");
            else
                strlist.append("");
        }
        data << strlist.join(",") << "\n";
        /*write contents*/
        for (int i = 0; i < m_Model->rowCount(); i++) {
            strlist.clear();
            for (int j = 0; j < m_Model->columnCount(); j++) {

                if (m_Model->data(m_Model->index(i, j)).toString().length() > 0)
                    strlist.append("\"" + m_Model->data(m_Model->index(i, j)).toString() + "\"");
                else
                    strlist.append("");
            }
            data << strlist.join(",") + "\n";
        }

        csvfile->close();/*closing csv file*/
        m_dbBrowser->close(); /*closing data base*/
        done =true;
    }
    return 0;
}

/*check assigned database return true if available*/
bool checkDbName(QString dbname)
{
    for(int i=0; i < db_list_name.count(); i++)
    {
        if(QString(db_list_name[i]).contains(dbname))
        {
            m_activeDb = db_list_name[i];
            return true;
        }

    }
    return false;
}

/**
  * represent to communicate with data base
int ValidDbFile(QString connection)
{

    QStringList CMDLIST;

    CMDLIST << CMD_DROP_TABLE << CMD_CREATE_TABLE << CMD_INSERT_INDEX ;


    QSqlDatabase db = QSqlDatabase::addDatabase("SQLITECIPHER");
    db.setDatabaseName(connection);

    if(!db.open())
    {
        qDebug() << db.lastError();
        db.close();
        return -1;
    }


    QSqlQuery query(db);
    for(QStringList::const_iterator list = CMDLIST.constBegin(); list != CMDLIST.constEnd(); ++ list)
    {
        QString m_CMD = *list;
        if(!query.prepare(m_CMD) )
        {
            qDebug() << query.lastError();
            db.close();
            return -1;
        }
        if(!query.exec(m_CMD))
        {
            qDebug() << query.lastError();
            db.close();
            return -1;
        }
        else
        {
            query.clear();
        }

    }
    db.close();
    return 0;
}
**/

/* read server.ini file*/
int ReadServerIni(QString path)
{
    QFile *f = NULL;
    QString m_File = path + "/server.ini";
    readerini *m_read_server_ini;
    QString test;


    f = new QFile(m_File);
    if(!f->exists())
    {
        qDebug() << QString("File : %1 doesnt exist.").arg(f->fileName());
        return -1;
    }
    if(!f->open(QIODevice::ReadOnly))
    {
        qDebug() << "can't open server.ini";
        return -1;
    }
    f->close();

    m_read_server_ini = new readerini(m_File);

    if(!m_read_server_ini->m_Loaded)
    {
        qDebug() << "Parsing server.ini failure";
        return -1;
    }

    if((m_Root_Directory = m_read_server_ini->Get("General", "DataFolder", "C:/ProgramData/GBillingServer")).length() < 0)
    {
        qDebug() << "Failed get data folder on Server.ini";
        return -1;
    }

    return 0;

}

/* check database directory is valid*/
int ValidDbDirectory()
{
    QString m_pathDbFile;
    QDir *m_Db_Directory;
    QFileInfoList m_Db_List;

    if(m_Root_Directory.length() < 0)
    {
        qDebug() << "root Db directory is null";
        return -1;
    }

    m_pathDbFile.append(m_Root_Directory);
    m_pathDbFile.append("/");
    m_pathDbFile.append("Database");

    m_Db_Directory = new QDir(m_pathDbFile);
    m_Db_List = m_Db_Directory->entryInfoList(QDir::NoFilter , QDir::NoSort);


    for(QFileInfoList::ConstIterator list = m_Db_List.constBegin(); list != m_Db_List.constEnd(); ++list)
    {
        QFileInfo m_file_info = ((QFileInfo)*list);
        if(QString::compare(m_file_info.fileName(), "Main.gbsdat") == 0) db_list_name << m_file_info.filePath() + ";";
        if(QString::compare(m_file_info.fileName(), "CafeInfo.gbsdat") == 0) db_list_name << m_file_info.filePath() + ";";
        if(QString::compare(m_file_info.fileName(), "GuestInfo.gbsdat") == 0) db_list_name << m_file_info.filePath() + ";";
        if(QString::compare(m_file_info.fileName(), "OrderHistory.gbsdat") == 0) db_list_name << m_file_info.filePath() + ";";
        if(QString::compare(m_file_info.fileName(), "ServerHistory.gbsdat") == 0) db_list_name << m_file_info.filePath() + ";";
        if(QString::compare(m_file_info.fileName(), "SessionHistory.gbsdat") == 0)db_list_name << m_file_info.filePath() + ";";
        if(QString::compare(m_file_info.fileName(), "SystemHistory.gbsdat") == 0) db_list_name << m_file_info.filePath() + ";";
        if(QString::compare(m_file_info.fileName(), "TransactionHistory.gbsdat") == 0) db_list_name << m_file_info.filePath() + ";";
        if(QString::compare(m_file_info.fileName(), "TransferDetailHistory.gbsdat") == 0) db_list_name << m_file_info.filePath() + ";";
        if(QString::compare(m_file_info.fileName(), "WebControl.gbsdat") == 0) db_list_name << m_file_info.filePath() + ";";
        if(QString::compare(m_file_info.fileName(), "WebHistory.gbsdat") == 0) db_list_name << m_file_info.filePath() + ";";
        if(QString::compare(m_file_info.fileName(), "Coupons.gbsdat") == 0) db_list_name << m_file_info.filePath() + ";";
        if(QString::compare(m_file_info.fileName(), "TopupQueueHistory.gbsdat") == 0) db_list_name << m_file_info.filePath() + ";";
        if(QString::compare(m_file_info.fileName(), "WebHistory.gbsdat") == 0) db_list_name << m_file_info.filePath() + ";";
    }


    return 0;

}

/*Main or entry point of application*/
int main(int argc, char* argv[])
{
    QString m_dbName;
    QString m_Query;
    QString m_csvName;
    done =false;

    QCoreApplication a(argc, argv);

    /*check available sql drivers*/
    //qDebug() << QSqlDatabase::drivers();

    /*Get root directory of main application*/
    QString m_path = QCoreApplication::applicationDirPath();
    QDir::setCurrent(m_path);

    /*initial class database*/
    m_dbBrowser = new DBBrowser;

    /*parsing argument of cli*/
    if(argc == 0)
    {
        qDebug() << "Usage -n <dbname> -c <sql command> -f <csv name>";
        return -1;
    }
    for(int i =0; i < argc; i++)
    {
        if(strcmp(argv[i], "-n") == 0)
        {
            m_dbName = argv[i + 1];
        }
        if(strcmp(argv[i], "-c") == 0)
        {
            m_Query = argv[i + 1];
        }
        if(strcmp(argv[i], "-f") == 0)
        {
            m_csvName = argv[i + 1];
        }
    }

    if(m_dbName.length() == 0 && m_Query.length() == 0)
    {
        qDebug() << "untuk access database chiper(encrypt). harus dipilih nama file database";
        return -1;
    }

    /* read server ini*/
    if(ReadServerIni(m_path) != 0)
    {
        qDebug() << "file server.ini tidak ada.";
        return -1;
    }

    /*check data base directory*/
    if(ValidDbDirectory() != 0)
    {
        qDebug() << "nama file database tidak ditemukan di direktori";
        return -1;
    }
    // check assign db name
    if(!checkDbName(m_dbName))
    {
        qDebug() << QString("Data base : %1 not available").arg(m_dbName);
        return -1;
    }

    /*reformat path data base */
    QString temp =  QString(m_activeDb).replace(";", "");
    if(openDb(temp, m_Query, m_csvName) != 0)
    {
        qDebug() << QString("File: %1 berikut tidak ditemukan di directori database").arg(temp)  ;
        return -1;
    }

    /*done operation will clear all */
    if(done)
    {
        m_dbBrowser = NULL;
        a.flush();
        a.quit();
    }
    return -1;
}
