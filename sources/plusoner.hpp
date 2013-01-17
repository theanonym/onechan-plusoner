/**
*
* (C) 2013 Theanonym
*
* https://github.com/theanonym/yoba-onechan-plusoner
*
*/

/*
 *
 * Класс для работы с движком одинчана. Содержит в себе
 * QNetworkAccessManager. Каждый плюсонер должен находиться в
 * отдельном потоке.
 *
 * Сообщается с гуем посредством сигналов и слотов.
 *
 */

#ifndef PlUSONER_HPP
#define PlUSONER_HPP

#include <QThread>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QNetworkCookieJar>
#include <QNetworkProxy>
#include <QPixmap>
#include <QRegExp>

class Plusoner : public QObject
{
   Q_OBJECT

   bool m_is_running;
   int m_rate;
   int m_thread;
   bool m_need_captcha;
   QString m_phpsessid;

   QNetworkAccessManager * m_nmanager;
   QNetworkRequest m_default_request;
   QNetworkReply * m_captcha_reply;
   QNetworkReply * m_try_vote_reply;
   QNetworkReply * m_vote_reply;
   QNetworkProxy m_proxy;

   QPixmap m_captcha_image;
   QString m_captcha_text;

   bool m_captcha_is_succes;
   bool m_try_vote_is_success;
   bool m_vote_is_success;
   bool m_has_proxy;
   bool m_has_captcha_image;

   void reset();
   void message(const QString &);

public:
   Plusoner(QObject * parent = 0);
   ~Plusoner();

   inline bool isRunning() const { return m_is_running; }

   inline bool captchaIsSuccess() const { return m_captcha_is_succes; }
   inline bool tryVoteIsSuccess() const { return m_try_vote_is_success; }
   inline bool voteIsSuccess() const { return m_vote_is_success; }

   inline void setRate(int rate) { m_rate = rate; }
   inline int getRate() const { return m_rate; }
   inline bool hasRate() const { return m_rate != -1; }

   inline void setThread(int thread) { m_thread = thread; }
   inline int getThread() const { return m_thread; }
   inline bool hasThread() const { return m_thread != -1; }

   inline void setProxy(const QNetworkProxy & proxy) { m_proxy = proxy; m_has_proxy = true; m_nmanager->setProxy(m_proxy); }
   inline QNetworkProxy getProxy() const { return m_proxy; }
   inline bool hasProxy() const { return m_has_proxy; }
   inline QString proxyToString() const { return hasProxy() ? m_proxy.hostName() : "Без прокси"; }

   inline QPixmap getCaptchaImage() const { return m_captcha_image; }
   inline bool hasCaptchaImage() const { return !m_captcha_image.isNull(); }

   inline void setCaptchaText(const QString & text) { m_captcha_text = text; }
   inline QString getCaptchaText() const { return m_captcha_text; }
   inline bool hasCaptchaText() const { return !m_captcha_text.isEmpty(); }

   inline bool needCaptcha() const { return m_need_captcha; }

   inline bool hasPHPSessid() const { return !m_phpsessid.isEmpty(); }

signals:
   void signal_newMessage(QString);
   void signal_captchaRequestFinished();
   void signal_tryVoteRequestFinished();
   void signal_voteRequestFinished();

private slots:
   void slot_captchaRequestFinished();
   void slot_tryVoteRequestFinished();
   void slot_voteRequestFinished();

public slots:
   void slot_stop();
   void slot_sendCaptchaRequest();
   void slot_sendTryVoteRequest();
   void slot_sendVoteRequest();
};

#endif // PlUSONER_HPP
