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

   m_default_request.setRawHeader("User-Agent", "Opera/9.80 (X11; Linux i686) Presto/2.12.388 Version/12.12");
   m_default_request.setRawHeader("Referer", "http://1chan.ru/");

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
   m_is_running = false;
   m_rate = -1;
   m_thread = -1;
   m_need_captcha = false;

   m_captcha_is_succes = false;
   m_try_vote_is_success = false;
   m_vote_is_success = false;
   m_has_proxy = false;

   m_captcha_image = QPixmap();
   m_captcha_text = QString();
}

/*
 * Остановка работы.
 */
void Plusoner::slot_stop()
{
   //TODO abort() приводит к сегфолту.

//   if(m_try_vote_reply && m_try_vote_reply->isRunning())
//   {
//      m_try_vote_reply->abort();
//   }

//   if(m_captcha_reply && m_captcha_reply->isRunning())
//   {
//      m_captcha_reply->abort();
//   }

//   if(m_vote_reply && m_vote_reply->isRunning())
//   {
//      m_vote_reply->abort();
//   }

//   reset();
}

/*
 * Отправка сообщения в гуй
 */
void Plusoner::message(const QString & text)
{
   emit signal_newMessage(text);
}

/*
 * Отправка голоса (без капчи)
 */
void Plusoner::slot_sendTryVoteRequest()
{
   if(!hasThread() || !hasRate())
      throw 1;

   // Сообщение в лог
   message(QString("[%2] [%1] %3").arg(proxyToString(), "Try vote", "запрос отправлен"));

   // Создание запроса
   QUrl url(QString("http://1chan.ru/news/res/%1/rate_post/%2/").arg(m_thread).arg(m_rate ? "up" : "down"));
   QNetworkRequest request = m_default_request;
   request.setUrl(url);

   // Отправка запроса
   m_try_vote_reply = m_nmanager->get(request);
   connect(m_try_vote_reply, SIGNAL(finished()), SLOT(slot_tryVoteRequestFinished()));
}

/*
 * Запрос капчи
 */
void Plusoner::slot_sendCaptchaRequest()
{
   if(!hasPHPSessid())
      throw 1;

   // Сообщение в лог
   message(QString("[%2] [%1] %3").arg(proxyToString(), "Get captcha", "запрос отправлен"));

   // Создание запроса
   QUrl url(QString("http://1chan.ru/captcha/?key=rate&PHPSESSID=%1").arg(m_phpsessid));
   QNetworkRequest request = m_default_request;
   request.setUrl(url);

   // Отправка запроса
   m_captcha_reply = m_nmanager->get(request);
   connect(m_captcha_reply, SIGNAL(finished()), SLOT(slot_captchaRequestFinished()));
}

/*
 * Отправка голоса (с капчей)
 */
void Plusoner::slot_sendVoteRequest()
{
   if(!hasCaptchaText())
      throw 1;

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
}


/*
 * Обработка ответа на попытку проголосовать без капчи
 */
void Plusoner::slot_tryVoteRequestFinished()
{
   // Получение строки статуса и контента
   int code = m_try_vote_reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
   QString status_line = m_try_vote_reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toString()
                         + " " + m_try_vote_reply->attribute(QNetworkRequest::HttpReasonPhraseAttribute).toString();
   QByteArray content = m_try_vote_reply->readAll();


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
      m_need_captcha = content.contains("<title>Голосование за пост");

      // Если нужно ввести капчу, сохраняем печеньку PHPSESSID
      if(needCaptcha())
      {
         m_try_vote_is_success = false;

         // Сообщение в лог
         message(QString("[%2] [%1] %3").arg(proxyToString(), "Try vote", "нужна капча"));

         // Сохраняем печеньку
         foreach(const QNetworkCookie & cookie, m_nmanager->cookieJar()->cookiesForUrl(QUrl("http://1chan.ru")))
         {
            if(cookie.name() == "PHPSESSID")
            {
               m_phpsessid = cookie.value();
               break;
            }
         }
      }

      // Код 200, но капчу вводить не нужно - неизвестная ошибка
      else
      {
         message(QString("[%2] [%1] %3").arg(proxyToString(), "Try vote", "неизвестная ошибка"));
         m_try_vote_is_success = false;
      }
   }

   // Ошибка соединения
   else
   {
      if(m_try_vote_reply->error() != QNetworkReply::NoError)
         message(QString("[%2] [%1] %3").arg(proxyToString(), "Try vote", "ошибка соединения: " + m_try_vote_reply->errorString()));
      else
         message(QString("[%2] [%1] %3").arg(proxyToString(), "Try vote", "ошибка соединения: " + status_line));
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
   // Получение строки статуса и контента
   int code = m_captcha_reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
   QString status_line = m_captcha_reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toString()
                         + " " + m_captcha_reply->attribute(QNetworkRequest::HttpReasonPhraseAttribute).toString();

   QByteArray content = m_captcha_reply->readAll();

   // Код 200 и Content-Type image/* - успешный запрос капчи
   if(code == 200 && m_captcha_reply->header(QNetworkRequest::ContentTypeHeader).toString().indexOf(QRegExp("image/\\w+")) != -1)
   {
      m_captcha_is_succes = true;

      // Сообщение в лог
      message(QString("[%2] [%1] %3").arg(proxyToString(), "Get captcha", "капча получена"));

      // Сохранение картинки в память
      m_captcha_image.loadFromData(content);
   }

   // Ошибка соединения
   else
   {
      if(m_captcha_reply->error() != QNetworkReply::NoError)
         message(QString("[%2] [%1] %3").arg(proxyToString(), "Get captcha", "ошибка соединения: " + m_captcha_reply->errorString()));
      else
         message(QString("[%2] [%1] %3").arg(proxyToString(), "Get captcha", "ошибка соединения: " + status_line));
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
   // Получение строки статуса
   int code = m_vote_reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
   QString status_line = m_vote_reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toString()
                         + " " + m_vote_reply->attribute(QNetworkRequest::HttpReasonPhraseAttribute).toString();
//   QByteArray content = m_vote_reply->readAll(); // Контент не нужен

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
         message(QString("[%2] [%1] %3").arg(proxyToString(), "Vote", "ошибка соединения: " + m_vote_reply->errorString()));
      else
         message(QString("[%2] [%1] %3").arg(proxyToString(), "Vote", "ошибка соединения: " + status_line));
      m_try_vote_is_success = false;
   }

   // Удаление ответа
   m_vote_reply->disconnect();
   m_vote_reply->deleteLater();

   // Отправка сигнала гую
   emit signal_voteRequestFinished();
}
