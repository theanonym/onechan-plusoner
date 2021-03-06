/**
*
* (C) 2013 Theanonym
*
* https://github.com/theanonym/onechan-plusoner
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
#include <QNetworkCookie>
#include <QNetworkCookieJar>
#include <QNetworkProxy>
#include <QPixmap>
#include <QRegExp>
#include <QTimer>
#include <QDir>
#include <QApplication>

class Plusoner : public QObject
{
   Q_OBJECT

   int m_rate;
   int m_thread;
   int m_timeout;
   int m_attempts;
   bool m_need_captcha;
   bool m_try_vote_is_running;
   bool m_captcha_is_running;
   bool m_vote_is_running;
   bool m_captcha_is_succes;
   bool m_try_vote_is_success;
   bool m_vote_is_success;
   bool m_has_proxy;
   bool m_is_timeout;
   QString m_phpsessid;
   QPixmap m_captcha_image;
   QString m_captcha_text;

   QNetworkAccessManager * m_nmanager;
   QNetworkRequest m_default_request;
   QNetworkReply * m_captcha_reply;
   QNetworkReply * m_try_vote_reply;
   QNetworkReply * m_vote_reply;
   QNetworkProxy m_proxy;
   QTimer * m_timer;

   void reset();
   void message(const QString &);

public:
   Plusoner(QObject * parent = 0);
   ~Plusoner();

   void setCookies(const QString &);

   inline bool captchaIsSuccess() const { return m_captcha_is_succes; }
   inline bool tryVoteIsSuccess() const { return m_try_vote_is_success; }
   inline bool voteIsSuccess() const    { return m_vote_is_success; }
   inline bool isTimeout() const        { return m_is_timeout; }
   inline bool needCaptcha() const      { return m_need_captcha; }
   inline bool hasPHPSessid() const     { return !m_phpsessid.isEmpty(); }

   inline void setRate(int rate) { m_rate = rate; }
   inline int getRate() const    { return m_rate; }
   inline bool hasRate() const   { return m_rate != -1; }

   inline void setThread(int thread) { m_thread = thread; }
   inline int getThread() const      { return m_thread; }
   inline bool hasThread() const     { return m_thread != -1; }

   void setProxy(const QNetworkProxy &);
   inline QNetworkProxy getProxy() const { return m_proxy; }
   inline bool hasProxy() const          { return m_has_proxy; }
   QString proxyToString() const;

   inline QPixmap getCaptchaImage() const { return m_captcha_image; }

   inline void setCaptchaText(const QString & text) { m_captcha_text = text; }
   inline QString getCaptchaText() const            { return m_captcha_text; }
   inline bool hasCaptchaText() const               { return !m_captcha_text.isEmpty(); }

   inline void setTimeout(int timeout) { m_timeout = timeout; }
   inline int  getTimeout() const      { return m_timeout; }
   inline bool hasTimeout() const      { return m_timeout != 0; }

   inline void setAttempts(int att) { m_attempts = att; }
   inline int hasAttempts() const   { return m_attempts != 0; }

private slots:
   void slCaptchaRequestFinished();
   void slTryVoteRequestFinished();
   void slVoteRequestFinished();
   void slTimeout();

public slots:
   void slStop();
   void slSendCaptchaRequest();
   void slSendTryVoteRequest();
   void slSendVoteRequest();

signals:
   void siNewMessage(QString);
   void siNewCookie(QString, QString);
   void siCaptchaRequestFinished(Plusoner *);
   void siTryVoteRequestFinished(Plusoner *);
   void siVoteRequestFinished(Plusoner *);
};

#endif // PlUSONER_HPP
