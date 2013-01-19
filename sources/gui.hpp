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
#include <QThread>
#include <QFileDialog>

#include "ui_gui.h"

#include "plusoner.hpp"
#include "plusonerthread.hpp"
#include "proxylist.hpp"

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

   bool m_is_running;
   bool m_captcha_displayed;
   bool m_proxies_is_accepted;
   int  m_loglevel;
   int  m_rate;
   int  m_thread;

   void printMessage(const QString &, int);

   void enableGUI();
   void disableGUI();
   bool checkSettings();
   void updateCounters();
   void displayCaptcha(const QPixmap &, const QString &);
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
   void slot_startButtonPressed();
   void slot_threadChanged(QString);
   void slot_rateChanged(int);
   void slot_logLevelChanged(int);
   void slot_captchaEntered();
   void slot_acceptProxiesButtonPressed();
   void slot_loadProxiesFromFileButtonPressed();

   void slot_newMessage(QString);
   void slot_captchaRequestFinished();
   void slot_tryVoteRequestFinished();
   void slot_voteRequestFinished();

};

#endif // GUI_HPP
