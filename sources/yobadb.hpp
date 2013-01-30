/**
*
* (C) 2013 Theanonym
*
* https://github.com/theanonym/yoba-onechan-plusoner
*
*/

#ifndef YOBADB_HPP
#define YOBADB_HPP

#include <QMap>
#include <QString>
#include <QFile>
#include <QRegExp>

class YobaDB : public QObject
{
   Q_OBJECT

   QMap<QString, QString> m_db;

   QString m_fname;

public:
   YobaDB(QObject * parent = 0);
   YobaDB(const QString & fname, QObject * parent = 0);

   inline int size() const { return m_db.size(); }
   inline bool isEmpty() const { return m_db.isEmpty(); }
   inline bool contains(const QString & key) const { return m_db.contains(key); }

   inline void insert(const QString & key, const QString & value) { m_db.insert(key, value); }
   inline QString get(const QString & key) const { return m_db.value(key); }
   inline void remove(const QString & key) { m_db.remove(key); }
   inline void clear() { m_db.clear(); }

   inline void setFileName(const QString & fname) { m_fname = fname; }
   inline QString getFileName() const { return m_fname; }
   inline bool hasFileName() const { return !m_fname.isEmpty(); }

   bool saveToFile(const QString &) const;
   bool saveToFile() const;
   bool loadFromFile(const QString &);
   bool loadFromFile();
};

#endif // YOBADB_HPP
