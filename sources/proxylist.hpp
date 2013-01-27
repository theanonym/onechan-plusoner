/**
*
* (C) 2013 Theanonym
*
* https://github.com/theanonym/yoba-onechan-plusoner
*
*/

/*
 *
 * Класс для работы с проксями.
 * Умеет парсить прокси из текста и загружать их из файла.
 *
 */

#ifndef PROXYLIST_HPP
#define PROXYLIST_HPP

#include <QList>
#include <QString>
#include <QStringList>
#include <QFile>
#include <QNetworkProxy>

class Proxylist
{
   QList<QNetworkProxy> m_proxies;
   QStringList m_hosts;

public:
   Proxylist();
   ~Proxylist();

   bool add(const QNetworkProxy &);
   bool add(const QString &, qint16);
   int  addFromText(const QString & text, int max_count = -1);
   int  addFromFile(const QString &);

   bool saveToFile(const QString &) const;
   QString toString() const;

   QNetworkProxy & get(int index) { return m_proxies[index]; }
   QList<QNetworkProxy> & getAll() { return m_proxies; }

   inline void clear() { m_proxies.clear(); m_hosts.clear(); }
   inline int count() const { return m_proxies.count(); }
   inline bool isEmpty() const { return m_proxies.isEmpty(); }
};

#endif // PROXYLIST_HPP
