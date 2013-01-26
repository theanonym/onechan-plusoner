/**
*
* (C) 2013 Theanonym
*
* https://github.com/theanonym/yoba-onechan-plusoner
*
*/

/*
 *
 * Обёртка над QNetworkAccessManager и QNetworkCookieJar
 * с добавлением возможности сохранять и загружать куки из файла.
 *
 */

#ifndef YOBANETWORKMANAGER_HPP
#define YOBANETWORKMANAGER_HPP

#include <QNetworkAccessManager>
#include <QNetworkCookieJar>
#include <QFile>

class YobaCookieJar : public QNetworkCookieJar
{
public:
   inline QList<QNetworkCookie> _allCookies() { return allCookies(); }
   inline void _setAllCookies(const QList<QNetworkCookie> & cookies) { setAllCookies(cookies); }
};

class YobaNetworkManager : public QNetworkAccessManager
{
   Q_OBJECT

   YobaCookieJar * m_cookies;

public:
   YobaNetworkManager(QObject * parent = 0);
   ~YobaNetworkManager();

   inline void _setCookieJar(YobaCookieJar * cookies) { m_cookies = cookies; setCookieJar(cookies); }
   inline YobaCookieJar * _cookieJar() const { return m_cookies; }

   bool saveCookiesToFile(const QString &) const;
   bool loadCookiesFromFile(const QString &);
};

#endif // YOBANETWORKMANAGER_HPP
