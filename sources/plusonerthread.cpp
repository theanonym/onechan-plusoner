/**
*
* (C) 2013 Theanonym
*
* https://github.com/theanonym/yoba-onechan-plusoner
*
*/

#include "plusonerthread.hpp"

PlusonerThread::PlusonerThread(QObject * parent)
   : QThread(parent)
{

}

PlusonerThread::~PlusonerThread()
{

}

void PlusonerThread::run()
{
   exec();
}

void PlusonerThread::setPlusoner(Plusoner * plusoner)
{
   m_plusoner = plusoner;
}
