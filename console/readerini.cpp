#include "readerini.h"

readerini::readerini(QString file):
    m_Error(0),
    m_Loaded(false)

{
    this->setFile(file);
    if(this->m_Error == 0)
        m_Loaded = true;
    else
        m_Loaded = false;
}

int readerini::ValueHandler(void *user, const char *section, const char *name, const char *value)
{
    readerini* m_readerini = (readerini*)user;
    QString key = MakeKey(section, name);
    if (m_readerini->hash[key].size() > 0)
        m_readerini->hash[key] += "\n";
    m_readerini->hash[key] += value;
    return 1;
}

QString readerini::MakeKey(const QString &section, const QString &name)
{
    QString key = section + "=" + name;
    // Convert to lower case to make section/name lookups case-insensitive
    //std::transform(key.begin(), key.end(), key.begin(), ::tolower);
    return key.toLower();
}

QString readerini::Get(const QString &section, const QString &name, const QString &default_value) const
{
    QString key = MakeKey(section, name);
    // Use _values.find() here instead of _values.at() to support pre C++11 compilers
    return hash.count(key) ? hash.find(key).value():default_value;
}

void readerini::setFile(const QString file)
{
   this->m_Error = ini_parse(file.toStdString().c_str(), ValueHandler, this);
}
