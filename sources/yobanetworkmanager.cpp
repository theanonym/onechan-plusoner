/**
*
* (C) 2013 Theanonym
*
* https://github.com/theanonym/yoba-onechan-plusoner
*
*/

#include "yobanetworkmanager.hpp"

YobaNetworkManager::YobaNetworkManager(QObject * parent)
   : QNetworkAccessManager(parent)
{
   m_cookies = new YobaCookieJar();
   this->setCookieJar(m_cookies);
}

YobaNetworkManager::~YobaNetworkManager()
{
   delete m_cookies;
}

/*
 * Сохранение куки в файл
 */
bool YobaNetworkManager::saveCookiesToFile(const QString & fname) const
{
   QFile file(fname);

   if(file.open(QFile::WriteOnly))
   {
      QList<QNetworkCookie> cookies = this->_cookieJar()->_allCookies();

      if(!cookies.empty())
      {
         foreach(const QNetworkCookie & cookie, cookies)
         {
            file.write(cookie.toRawForm());
            file.putChar('\n');
         }

         file.close();
         return true;
      }
      else
      {
         return false;
      }
   }
   else
   {
      return false;
   }
}

/*
 * Загрузка куки из файла
 */
bool YobaNetworkManager::loadCookiesFromFile(const QString & fname)
{
   QFile file(fname);

   if(file.open(QFile::ReadOnly))
   {
      QList<QNetworkCookie> cookies = QNetworkCookie::parseCookies(file.readAll());

      if(!cookies.empty())
      {
         this->_cookieJar()->_setAllCookies(cookies);
         return true;
      }
      else
      {
         return false;
      }
   }
   else
   {
      return false;
   }
}
