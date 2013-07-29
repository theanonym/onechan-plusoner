/**
*
* (C) 2013 Theanonym
*
* https://github.com/theanonym/onechan-plusoner
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
#include <QPointer>

#include "plusoner.hpp"

class PlusonerThread : public QThread
{
   Q_OBJECT

   QPointer<Plusoner> m_plusoner;

   void run();

public:
   PlusonerThread(QObject * parent = 0);
   ~PlusonerThread();

   inline void setPlusoner(Plusoner * plusoner) { m_plusoner = plusoner; }
   inline Plusoner * getPlusoner() { Q_ASSERT(!m_plusoner.isNull()); return m_plusoner.data(); }
};

#endif // PLUSONERTHREAD_HPP
