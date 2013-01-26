/**
*
* (C) 2013 Theanonym
*
* https://github.com/theanonym/yoba-onechan-plusoner
*
*/

/*
 *
 * Простая обёртка над QThread.
 *
 */

#ifndef PLUSONERTHREAD_HPP
#define PLUSONERTHREAD_HPP

#include <QThread>

#include "plusoner.hpp"

class PlusonerThread : public QThread
{
   Q_OBJECT

   Plusoner * m_plusoner;

   void run();

public:
   PlusonerThread(QObject * parent = 0);
   ~PlusonerThread();

   inline void setPlusoner(Plusoner * plusoner) { m_plusoner = plusoner; }
   inline Plusoner * getPlusoner() { return m_plusoner; }
};

#endif // PLUSONERTHREAD_HPP
