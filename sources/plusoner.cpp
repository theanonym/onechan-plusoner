/**
*
* (C) 2013 Theanonym
*
* https://github.com/theanonym/yoba-onechan-plusoner
*
*/

#include "plusoner.hpp"

Plusoner::Plusoner(QObject * parent)
   : QObject(parent)
{
   m_nmanager = new QNetworkAccessManager(this);

   m_timer = new QTimer(this);
   m_timer->setSingleShot(true);
   connect(m_timer, SIGNAL(timeout()), SLOT(slot_timeout()));

   m_default_request.setRawHeader("User-Agent", "Opera/9.80 (X11; Linux i686) Presto/2.12.388 Version/12.12");
   m_default_request.setRawHeader("Referer", "http://1chan.ru/");

   m_has_proxy = false;
   m_try_vote_is_running = false;
   m_captcha_is_running  = false;
   m_vote_is_running     = false;

   reset();
}

Plusoner::~Plusoner()
{

}

/*
 * Установка всех значений по умолчанию.
 */
void Plusoner::reset()
{
   m_thread  = -1;
   m_rate    = -1;
   m_timeout = 0;
   m_need_captcha = false;

   m_captcha_is_succes = false;
   m_try_vote_is_success = false;
   m_vote_is_success = false;
   m_has_captcha_image = false;

//   m_captcha_image = QPixmap();
   m_captcha_text = QString();
}

void Plusoner::setProxy(const QNetworkProxy & proxy)
{
   m_proxy = proxy;
   m_has_proxy = true;
   m_nmanager->setProxy(m_proxy);
}

QString Plusoner::proxyToString() const
{
   return hasProxy() ? QString("%1:%2").arg(m_proxy.hostName()).arg(m_proxy.port())  : "Без прокси";
}

/*
 * Установка куки (из текста заголовков)
 */
void Plusoner::setCookies(const QString & text)
{
   m_nmanager->cookieJar()->setCookiesFromUrl(QNetworkCookie::parseCookies(text.toAscii()), QUrl("http://1chan.ru"));
}

/*
 * Отправка сообщения в гуй
 */
void Plusoner::message(const QString & text)
{
   emit signal_newMessage(text);
}

/*
 * Остановка работы.
 */
void Plusoner::slot_stop()
{
   if(m_try_vote_is_running)
   {
      m_try_vote_reply->abort();
   }

   if(m_captcha_is_running)
   {
      m_captcha_reply->abort();
   }

   if(m_vote_is_running)
   {
      m_vote_reply->abort();
   }

   reset();
}

/*
 * Отправка голоса (без капчи)
 */
void Plusoner::slot_sendTryVoteRequest()
{
   Q_ASSERT(hasThread());
   Q_ASSERT(hasRate());

   // Сообщение в лог
   message(QString("[%2] [%1] %3").arg(proxyToString(), "Try vote", "запрос отправлен"));

   // Создание запроса
   QUrl url(QString("http://1chan.ru/news/res/%1/rate_post/%2/").arg(m_thread).arg(m_rate ? "up" : "down"));
   QNetworkRequest request = m_default_request;
   request.setUrl(url);

   // Отправка запроса
   m_try_vote_reply = m_nmanager->get(request);
   connect(m_try_vote_reply, SIGNAL(finished()), SLOT(slot_tryVoteRequestFinished()));
   m_try_vote_is_running = true;

   if(hasTimeout())
      m_timer->start(m_timeout * 1000);

}

/*
 * Запрос капчи
 */
void Plusoner::slot_sendCaptchaRequest()
{
   Q_ASSERT(hasThread());
   Q_ASSERT(hasRate());
   Q_ASSERT(hasPHPSessid());

   // Сообщение в лог
   message(QString("[%2] [%1] %3").arg(proxyToString(), "Get captcha", "запрос отправлен"));

   // Создание запроса
   QUrl url(QString("http://1chan.ru/captcha/?key=rate&PHPSESSID=%1").arg(m_phpsessid));
   QNetworkRequest request = m_default_request;
   request.setUrl(url);

   // Отправка запроса
   m_captcha_reply = m_nmanager->get(request);
   connect(m_captcha_reply, SIGNAL(finished()), SLOT(slot_captchaRequestFinished()));
   m_captcha_is_running = true;

   if(hasTimeout())
      m_timer->start(m_timeout * 1000);

}

/*
 * Отправка голоса (с капчей)
 */
void Plusoner::slot_sendVoteRequest()
{
   Q_ASSERT(hasThread());
   Q_ASSERT(hasRate());
   Q_ASSERT(hasCaptchaText());

   // Сообщение в лог
   message(QString("[%2] [%1] %3").arg(proxyToString(), "Vote", "запрос отправлен, c:" + getCaptchaText()));

   // Создание запроса
   QUrl url(QString("http://1chan.ru/news/res/%1/rate_post/%2/").arg(m_thread).arg(m_rate ? "up" : "down"));
   QNetworkRequest request = m_default_request;
   request.setUrl(url);
   QByteArray content = "referer=http://1chan.ru/news/all/&captcha_key=rate&captcha=";
   content.append(m_captcha_text);
   request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
   request.setHeader(QNetworkRequest::ContentLengthHeader, content.length());

   // Отправка запроса
   m_vote_reply = m_nmanager->post(request, content);
   connect(m_vote_reply, SIGNAL(finished()), SLOT(slot_voteRequestFinished()));
   m_vote_is_running = true;

   if(hasTimeout())
      m_timer->start(m_timeout * 1000);

}

/*
 * Обработка ответа на попытку проголосовать без капчи
 */
void Plusoner::slot_tryVoteRequestFinished()
{
   m_try_vote_is_running = false;

   if(m_timer->isActive())
      m_timer->stop();

   // Получение кода ответа
   int code = m_try_vote_reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

   // Код 303 - голос принят
   if(code == 303)
   {
      message(QString("[%2] [%1] %3").arg(proxyToString(), "Try vote", "успешно"));
      m_try_vote_is_success = true;
      m_need_captcha = false;
   }

   // Код 200 - движок выдал ошибку
   else if(code == 200)
   {
      // Надобность вводить капчу определяется по заголовку страницы
      QByteArray content = m_try_vote_reply->readAll();
      m_need_captcha = content.contains("<title>Голосование за пост");

      // Если нужно ввести капчу, сохраняем печеньку PHPSESSID
      if(needCaptcha())
      {
         m_try_vote_is_success = false;

         // Сообщение в лог
         message(QString("[%2] [%1] %3").arg(proxyToString(), "Try vote", "нужна капча"));

         // Обработка печенек
         QString raw_cookies;
         foreach(const QNetworkCookie & cookie, m_nmanager->cookieJar()->cookiesForUrl(QUrl("http://1chan.ru")))
         {
            raw_cookies.append(cookie.toRawForm());
            raw_cookies.append('\n');

            if(cookie.name() == "PHPSESSID")
            {
               m_phpsessid = cookie.value();
            }
         }

         // Отправка сигнала гую, чтобы он добавил куки в базу
         if(hasPHPSessid() && hasProxy() && !raw_cookies.isEmpty())
            emit signal_newCookie(m_proxy.hostName(), raw_cookies);
      }

      // Код 200, но капчу вводить не нужно - неизвестная ошибка
      else
      {
         message(QString("[%2] [%1] %3").arg(proxyToString(), "Try vote", m_try_vote_reply->errorString()));
         m_try_vote_is_success = false;
      }
   }

   // Ошибка соединения
   else
   {
      if(m_try_vote_reply->error() != QNetworkReply::NoError)
         message(QString("[%2] [%1] %3").arg(proxyToString(), "Try vote", m_try_vote_reply->errorString()));
      else
         message(QString("[%2] [%1] %3").arg(proxyToString(), "Try vote", "ошибка соединения: "));
      m_try_vote_is_success = false;
   }

   // Удаление ответа
   m_try_vote_reply->disconnect();
   m_try_vote_reply->deleteLater();

   // Отправка сигнала гую
   emit signal_tryVoteRequestFinished();
}

/*
 * Обработка ответа на запрос капчи
 */
void Plusoner::slot_captchaRequestFinished()
{
   m_captcha_is_running = false;

   if(m_timer->isActive())
      m_timer->stop();

   // Получение кода ответа
   int code = m_captcha_reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

   // Код 200 и Content-Type image/* - успешный запрос капчи
   if(code == 200 && m_captcha_reply->header(QNetworkRequest::ContentTypeHeader).toString().indexOf(QRegExp("image/\\w+")) != -1)
   {
      m_captcha_is_succes = true;

      // Сообщение в лог
      message(QString("[%2] [%1] %3").arg(proxyToString(), "Get captcha", "капча получена"));

      // Сохранение картинки в память
      m_captcha_image.loadFromData(m_captcha_reply->readAll());

      m_has_captcha_image = true;
   }

   // Ошибка соединения
   else
   {
      if(m_captcha_reply->error() != QNetworkReply::NoError)
         message(QString("[%2] [%1] %3").arg(proxyToString(), "Get captcha", m_captcha_reply->errorString()));
      else
         message(QString("[%2] [%1] %3").arg(proxyToString(), "Get captcha", "ошибка соединения: "));
      m_captcha_is_succes = false;
   }

   // Удаление ответа
   m_captcha_reply->disconnect();
   m_captcha_reply->deleteLater();

   // Отправка сигнала гую
   emit signal_captchaRequestFinished();
}

/*
 * Обработка ответа на попытку проголосовать с капчей
 */
void Plusoner::slot_voteRequestFinished()
{
   m_vote_is_running = false;

   if(m_timer->isActive())
      m_timer->stop();

   // Получение кода ответа
   int code = m_vote_reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

   // Код 303 - голос принят
   if(code == 303)
   {
      message(QString("[%2] [%1] %3").arg(proxyToString(), "Try vote", "успешно"));
      m_vote_is_success = true;
   }

   // Ошибка соединения
   else
   {
      if(m_vote_reply->error() != QNetworkReply::NoError)
         message(QString("[%2] [%1] %3").arg(proxyToString(), "Vote", m_vote_reply->errorString()));
      else
         message(QString("[%2] [%1] %3").arg(proxyToString(), "Vote", "ошибка соединения: "));
      m_vote_is_success = false;
   }

   // Удаление ответа
   m_vote_reply->disconnect();
   m_vote_reply->deleteLater();

   // Отправка сигнала гую
   emit signal_voteRequestFinished();
}

void Plusoner::slot_timeout()
{
   slot_stop();
}

