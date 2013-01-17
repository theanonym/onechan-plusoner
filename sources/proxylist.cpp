/**
*
* (C) 2013 Theanonym
*
* https://github.com/theanonym/yoba-onechan-plusoner
*
*/

#include "proxylist.hpp"

Proxylist::Proxylist()
{

}

Proxylist::~Proxylist()
{

}

/*
 * Добавление прокси.
 *
 * Возвращает результат добавления.
 */
bool Proxylist::add(const QNetworkProxy & proxy)
{
   QString host = proxy.hostName();

   if(!m_hosts.contains(host))
   {
      m_hosts.push_back(host);
      m_proxies.push_back(proxy);
      return true;
   }
   else
   {
      return false;
   }
}

/*
 * Добавление из строки и номера порта.
 *
 * Возвращает результат добавления.
 */
bool Proxylist::add(const QString & host, qint16 port)
{
   return add(QNetworkProxy(QNetworkProxy::HttpProxy, host, port));
}

/*
 * Считывание прокси из текста.
 *
 * Возвращает количество полученных прокси.
 */
int Proxylist::addFromText(const QString & text, int max_count)
{
   int count = 0;

   QRegExp re_main("((?:\\w+://)?[\\w\\.]{5,}\\.(?:\\w{2,3}|\\d{1,3}):\\d{2,4})");
   QRegExp re_proxy("^(?:^\\w+://)?([\\w\\.]+):(\\d+)$");
   re_proxy.setMinimal(true);

   int pos = 0;
   while(text.indexOf(re_main, pos) != -1)
   {
      QString capture = re_main.cap(1);

      capture.indexOf(re_proxy);
      QString host = re_proxy.cap(1);
      QString port = re_proxy.cap(2);

      if(add(host, port.toInt()))
      {
         count++;
         if((count != -1) && count == max_count)
         {
            return count;
         }
      }

      pos += capture.length() + 1;
   }

   return count;
}

/*
 * Считывание прокси из файла.
 *
 * Возвращает количество полученных прокси.
 */
int Proxylist::addFromFile(const QString & fname)
{
   QFile file(fname);

   if(file.open(QIODevice::ReadOnly))
   {
      return addFromText(file.readAll());
   }
   else
   {
      return 0;
   }
}

/*
 * Сохранение в файл.
 *
 * Возвращает результат сохранения.
 */
bool Proxylist::saveToFile(const QString & fname) const
{
   if(count() == 0)
      return false;

   QFile file(fname);

   if(file.open(QIODevice::WriteOnly))
   {
      foreach(const QNetworkProxy & p, m_proxies)
      {
         QString line = QString("http://%1:%2\n").arg(p.hostName(), QString::number(p.port()));
         file.write(line.toAscii());
      }

      return true;
   }
   else
   {
      return false;
   }
}

/*
 * Отображение всего проксилиста в виде строки.
 */
QString Proxylist::toString() const
{
   if(count() == 0)
      return QString();

   QString out;
   out.reserve(count() * 30);

   foreach(const QNetworkProxy & p, m_proxies)
   {
      out.append(QString("http://%1:%2\n").arg(p.hostName(), QString::number(p.port())));
   }

   out.squeeze();
   return out;
}
