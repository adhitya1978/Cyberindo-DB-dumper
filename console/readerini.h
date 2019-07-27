#ifndef READERINI_H
#define READERINI_H

#include <QString>
#include <QHash>

#include "ini.h"

class readerini
{

private:
    int m_Error;
    QHash<QString, QString> hash;

    void setFile(const QString file);
    static int ValueHandler(void* user, const char* section, const char* name,
                        const char* value);
    static QString MakeKey(const QString &section, const QString &name);
public:
    bool m_Loaded;

    readerini(QString pszFile);
    QString Get(const QString &section, const QString &name, const QString &default_value) const;
};

#endif // READERINI_H
