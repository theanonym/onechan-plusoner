/**
*
* (C) 2013 Theanonym
*
* https://github.com/theanonym/yoba-onechan-plusoner
*
*/

/*
 *
 * Основной класс.
 *
 * Обеспечивает взаимодействие гуя и плюсонеров, находящихся
 * в других потоках.
 *
 */

#ifndef GUI_HPP
#define GUI_HPP

#include <QWidget>
#include <QDebug>
#include <QValidator>
#include <QMessageBox>
#include <QQueue>
#include <QFileDialog>

#include "ui_gui.h"

#include "plusoner.hpp"
#include "plusonerthread.hpp"
#include "proxylist.hpp"
#include "yobadb.hpp"

struct Counters
{
   int waiting;  // Ожидающие нажатия кнопки "Старт"
   int try_vote; // Голосующие без капчи
   int captcha;  // Отправившие запрос капчи
   int enter;    // Ожидающие ввода капчи
   int vote;     // Голосующие с капчей
   int success;  // Успешно проголосовавшие (с капчей или без)
   int error;    // Ошибка соединения на одном из этапов

   Counters()
   {
      reset();
   }

   void reset()
   {
      waiting = try_vote = captcha = enter = vote = success = error = 0;
   }
};

class GUI : public QWidget
{
   Q_OBJECT

   Ui::GUI ui;

   QList<Plusoner*>       m_plusoners;
   QList<PlusonerThread*> m_threads;
   QQueue<Plusoner*>      m_captcha_queue;
   Proxylist              m_proxylist;
   Counters m_counters;
   YobaDB   m_cookies_db;
   QString  m_cookies_db_file;

   bool m_is_running;
   bool m_captcha_displayed;
   bool m_proxies_is_accepted;
   int  m_loglevel;
   int  m_rate;
   int  m_thread;
   int  m_attempts;
   int  m_timeout;
   int  m_max_connections;

   void printMessage(const QString &, int);

   void enableGUI();
   void disableGUI();
   bool checkSettings();
   void updateCounters();
   void displayCaptcha(QPixmap, QString);
   void hideCaptcha();

   void createPlusoners();
   void setPlusoners();
   void stopPlusoners();
   void deletePlusoners();

   void acceptProxies();
   void clearProxies();

public:
   GUI(QWidget * parent = 0);
   ~GUI();

private slots:
   void slStartButtonPressed();
   void slThreadChanged(QString);
   void slRateChanged(int);
   void slAttemptsChanged(int);
   void slTimeoutChanged(int);
   void slMaxConnectionsChanged(int);
   void slLogLevelChanged(int);
   void slCaptchaEntered();
   void slAcceptProxiesButtonPressed();
   void slLoadProxiesButtonPressed();

   void slNewMessage(QString);
   void slNewCookie(QString, QString);
   void slCaptchaRequestFinished(Plusoner *);
   void slTryVoteRequestFinished(Plusoner *);
   void slVoteRequestFinished(Plusoner *);

};

#endif // GUI_HPP
