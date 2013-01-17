/**
*
* (C) 2013 Theanonym
*
* https://github.com/theanonym/yoba-onechan-plusoner
*
*/

#include "gui.hpp"

GUI::GUI(QWidget * parent)
   : QWidget(parent)
{
   ui.setupUi(this);

   m_is_running = false;
   m_captcha_displayed = false;
   m_proxies_is_accepted = false;
   m_loglevel = 1;
   m_rate = 1;
   m_thread = -1;

   /*
    * Элементы управления и сигналы в основной вкладке
    */
   ui.line_thread->setValidator(new QRegExpValidator(QRegExp("[0-9]{1,10}"), this));
   ui.line_captcha->setEnabled(false);
   ui.tabs->setCurrentIndex(0);
   ui.line_thread->setFocus();
   ui.editor_output->setReadOnly(true);
   ui.combobox_loglevel->setCurrentIndex(0);

   connect(ui.button_start,  SIGNAL(clicked()),                SLOT(slot_startButtonPresed()));
   connect(ui.combobox_rate, SIGNAL(currentIndexChanged(int)), SLOT(slot_rateChanged(int)));
   connect(ui.line_thread,   SIGNAL(textChanged(QString)),     SLOT(slot_threadChanged(QString)));
   connect(ui.line_captcha,  SIGNAL(returnPressed()),          SLOT(slot_captchaEntered()));
   connect(ui.combobox_loglevel, SIGNAL(currentIndexChanged(int)), SLOT(slot_logLevelChanged(int)));

   /*
    * Элементы управления и сигналы во вкладке "Прокси"
    */
   ui.spinbox_max_proxies->setValue(50);

   connect(ui.button_proxies_accept, SIGNAL(clicked()), SLOT(slot_AcceptProxiesButtonPresed()));

   printMessage("Привет.", 0);
}

GUI::~GUI()
{
   deletePlusoners();
}

/*
 * Вывод сообщения в лог.
 */
void GUI::printMessage(const QString & text, int loglevel)
{
   if(loglevel <= m_loglevel)
      ui.editor_output->appendPlainText(text);
}

/*
 * Разблокировка элементов управления в основной вкладке
 */
void GUI::enableGUI()
{
   ui.line_thread->setEnabled(true);
   ui.combobox_rate->setEnabled(true);
   ui.spinbox_max_proxies->setEnabled(true);
   ui.spinbox_ignore_proxies->setEnabled(true);
   ui.button_proxies_accept->setEnabled(true);
   ui.button_proxies_from_file->setEnabled(true);
}

/*
 * Блокировка элементов управления в основной вкладке
 */
void GUI::disableGUI()
{
   ui.line_thread->setEnabled(false);
   ui.combobox_rate->setEnabled(false);
   ui.spinbox_max_proxies->setEnabled(false);
   ui.spinbox_ignore_proxies->setEnabled(false);
   ui.button_proxies_accept->setEnabled(false);
   ui.button_proxies_from_file->setEnabled(false);
}

/*
 * Проверка настроек (загружены ли прокси, указан ли тред).
 *
 * В случае ошибки выводится окошко с сообщением.
 */
bool GUI::checkSettings()
{
   QStringList errors;
   bool has_error = false;

   if(m_thread == -1)
   {
      has_error = true;
      errors.append("не указан тред");
   }

   if(m_plusoners.count() == 0)
   {
      has_error = true;
      errors.append("не указаны прокси");
   }

   if(has_error)
   {
      QString error_string = errors.join(", ");
      error_string[0] = error_string[0].toUpper();
      error_string.append('.');

      printMessage("Ошибка: " + error_string, 0);

      QMessageBox * box = new QMessageBox(QMessageBox::Critical, "Ошибка", error_string, QMessageBox::Ok);
      connect(box, SIGNAL(finished(int)), box, SLOT(deleteLater()));
      box->show();

      return false;
   }
   else
   {
      return true;
   }
}

/*
 * Отображение капчи.
 * Принимает картинку и строку с прокси.
 * Разблокирировка поля для ввода.
 */
void GUI::displayCaptcha(const QPixmap & image, const QString & proxy)
{
   printMessage("Выведена капча для " + proxy, 0);

   ui.label_captcha->setPixmap(image);
   ui.label_text_proxy->setText(proxy);
   ui.line_captcha->setEnabled(true);
   ui.line_captcha->setFocus();
   m_captcha_displayed = true;
}

/*
 * Скрытие картинки капчи.
 * Блокируется поле для ввода.
 */
void GUI::hideCaptcha()
{
   ui.label_captcha->clear();
   ui.label_text_proxy->setText("Капча");
   ui.line_captcha->setEnabled(false);
   m_captcha_displayed = false;
}

/*
 * Обновление счётчиков
 */
void GUI::updateCounters()
{
   ui.label_counter_wait->setNum(m_counters.waiting);
   ui.label_counter_try_vote->setNum(m_counters.try_vote);
   ui.label_counter_captcha->setNum(m_counters.captcha);
   ui.label_counter_enter->setNum(m_counters.enter);
   ui.label_counter_vote->setNum(m_counters.vote);
   ui.label_counter_success->setNum(m_counters.success);
   ui.label_counter_errors->setNum(m_counters.error);
}

/*
 * Создание плюсонеров и потоков для каждой прокси.
 *
 * Если используется не впервые, уже имеющиеся плюсонеры и
 * потоки должны быть остановлены функцией stopPlusoners() и
 * удалены из памяти функцией deletePlusoners().
 */
void GUI::createPlusoners()
{
   // Определение начального и конечного индекса проксилиста
   // исходя из указанных в гуе параметров "Пропустить" и "Использовать".
   int begin = ui.spinbox_ignore_proxies->value();
   int end   = begin + ui.spinbox_max_proxies->value() - 1;

   if(begin >= m_proxylist.count())
   {
      qDebug() << "[Warning] Пропущено слишком много прокси, ни одной не было взято.";
      return;
   }

   if(end >= m_proxylist.count())
   {
      qDebug() << "[Warning] Попытка использовать больше прокси, чем есть.";
      end = m_proxylist.count() - 1;
   }

   qDebug() << "[Info] используются прокси с" << begin << "по" << end;

   // Перебор проксей
   for(int i = begin; i <= end; i++)
   {
      const QNetworkProxy & proxy = m_proxylist.get(i);

      // Создание плюсонера для этой прокси
      Plusoner * plusoner = new Plusoner();
      plusoner->setProxy(proxy);

      connect(plusoner, SIGNAL(signal_newMessage(QString)),      SLOT(slot_newMessage(QString)));
      connect(plusoner, SIGNAL(signal_captchaRequestFinished()), SLOT(slot_captchaRequestFinished()));
      connect(plusoner, SIGNAL(signal_tryVoteRequestFinished()), SLOT(slot_tryVoteRequestFinished()));
      connect(plusoner, SIGNAL(signal_voteRequestFinished()),    SLOT(slot_voteRequestFinished()));

      // Создание нового потока для этого плюсонера
      PlusonerThread * thread = new PlusonerThread();
      thread->setPlusoner(plusoner);
      plusoner->moveToThread(thread);

      // Сохранение указателей для дальнейшего использования
      m_plusoners.push_back(plusoner);
      m_threads.push_back(thread);
   }

   // Запуск всех потоков (вызов PlusonerThread::exec(), ни к каким действиям это не приводит)
   foreach(PlusonerThread * thread, m_threads)
      thread->start();

   m_counters.waiting = m_plusoners.count();
}

/*
 * Удаление всех плюсонеров и потоков из памяти.
 * Перед вызовом плюсонеры должны быть остановлены функцией
 * stopPlusoners().
 */
void GUI::deletePlusoners()
{
   if(m_plusoners.empty())
   {
      qDebug() << "[Warning] Нет плюсонеров для удаления.";
      return;
   }
   else
   {
      foreach(PlusonerThread * thread, m_threads)
         thread->quit();

      foreach(Plusoner * plusoner, m_plusoners)
         delete plusoner;

      foreach(PlusonerThread * thread, m_threads)
         delete thread;
   }

   m_plusoners.clear();
   m_threads.clear();

   m_counters.waiting = 0;
}

/*
 * Установка параметров (номер треда, плюс/минус) для всех плюсонеров
 * в соответствии с гуем.
 */
void GUI::setPlusoners()
{
   if(m_plusoners.empty())
   {
      qDebug() << "[Warning] Нет плюсонеров для установки параметров.";
      return;
   }
   else
   {
      foreach(Plusoner * plusoner, m_plusoners)
      {
         plusoner->setThread(m_thread);
         plusoner->setRate(m_rate);
      }
   }
}

//FIXME Plusoner::slot_stop() не работает и соотвественно эта функция тоже.
/*
 * Остановка всех плюсонеров.
 *
 * Вызывается abort() для всех соединений и сбрасываются все параметры,
 * так, будто плюсонер был только что создан.
 *
 * Остаются неизменными устнановленная для нетворк-менеджера прокси и
 * сохранённые им куки, чтобы не нужно было заново вводить капчу.
 */
void GUI::stopPlusoners()
{
   foreach(Plusoner * plusoner, m_plusoners)
   {
      QMetaObject::invokeMethod(plusoner, "slot_stop");
   }

   m_is_running = false;

   m_counters.waiting = m_plusoners.count();
}

/*
 * Нажата кнопка старт/стоп
 */
void GUI::slot_startButtonPresed()
{
   // Если плюсонилка не запущена - запускаем
   if(!m_is_running)
   {
      // Если в настройках что-то неправильно, ничего не делаем
      if(!checkSettings())
         return;

      // Если нет плюсонеров для запуска, ничего не делаем
      if(m_plusoners.empty())
      {
         qDebug() << "[Warning] Нет плюсонеров для запуска.";
         return;
      }

      m_is_running = true;

      // Меняем текст кнопки на "Стоп" и блокируем гуй
      ui.button_start->setText("Стоп");
      disableGUI();

      // Запускаем все плюсонеры
      setPlusoners();
      foreach(Plusoner * plusoner, m_plusoners)
      {
         QMetaObject::invokeMethod(plusoner, "slot_sendTryVoteRequest");
         m_counters.waiting--;
         m_counters.try_vote++;
      }

   }

   // Если запущена - останавливаем
   else
   {
      m_is_running = false;

      // Меняем текст кнопки на "Старт" и разблокируем гуй
      ui.button_start->setText("Старт");
      enableGUI();

      // Если капча отображается, убираем
      if(m_captcha_displayed)
      {
         ui.label_captcha->clear();
         ui.line_captcha->setEnabled(false);
         m_captcha_displayed = false;
      }

      // Останавливаем все плюсонеры
      stopPlusoners();

      // Счётчики
      m_counters.reset();
      m_counters.waiting = m_plusoners.count();
      updateCounters();
   }
}


/*
 * Изменён голос (плюс/минус)
 */
void GUI::slot_rateChanged(int index)
{
   m_rate = !index;
   setPlusoners();
}

/*
 * Изменён тред
 */
void GUI::slot_threadChanged(QString thread)
{
   if(!thread.isEmpty())
      m_thread = thread.toInt();
   else
      m_thread = -1;

   setPlusoners();
}

/*
 * Изменён уровень вывода сообщений
 */
void GUI::slot_logLevelChanged(int index)
{
   switch(index)
   {
      case 0: m_loglevel = 1; break;
      case 1: m_loglevel = 0; break;
      default: m_loglevel = 0;
   }
}

/*
 * Введён текст капчи
 */
void GUI::slot_captchaEntered()
{
   // Получаем текст капчи
   QString text = ui.line_captcha->text();
   ui.line_captcha->clear();

   // Если текст введён, запускаем плюсонер
   if(!text.isEmpty())
   {
      // Устанавливаем верхнему плюсонеру в очереди текст капчи
      // и отправляем с него голос за пост
      Plusoner * plusoner = m_captcha_queue.dequeue();
      plusoner->setCaptchaText(text);
      QMetaObject::invokeMethod(plusoner, "slot_sendVoteRequest");

      // Счётчики
      m_counters.enter--;
      m_counters.vote++;
      updateCounters();

      // Далее
      // Если очередь капч не пуста, отображаем следующую капчу
      if(!m_captcha_queue.empty())
      {
         Plusoner * next_plusoner = m_captcha_queue.first();
         displayCaptcha(next_plusoner->getCaptchaImage(), next_plusoner->proxyToString());
      }
      // Иначе скрываем поле для ввода
      else
      {
         hideCaptcha();
      }
   }
}

/*
 * Один из плюсонеров завершил голосование без капчи
 */
void GUI::slot_tryVoteRequestFinished()
{
   Plusoner * plusoner = (Plusoner*)sender();

   // Счётчики
   m_counters.try_vote--;
   if(plusoner->tryVoteIsSuccess())
      m_counters.success++;
   else if(!plusoner->needCaptcha())
      m_counters.error++;

   // Если плюсонеру нужна капча, отправляем запрос капчи
   if(plusoner->needCaptcha())
   {
      QMetaObject::invokeMethod(plusoner, "slot_sendCaptchaRequest");

      m_counters.captcha++;
   }

   updateCounters();
}

/*
 * Один из плюсонеров завершил запрос капчи
 */
void GUI::slot_captchaRequestFinished()
{
   Plusoner * plusoner = (Plusoner*)sender();

   m_counters.captcha--;

   // Если плюсонер имеет капчу, вставляем его в очередь
   if(plusoner->hasCaptchaImage())
   {
      m_captcha_queue.enqueue(plusoner);

      // Счётчики
      m_counters.enter++;

      // Если капча в данный момент не отображается,
      // отображаем новую капчу
      if(!m_captcha_displayed)
      {
         displayCaptcha(plusoner->getCaptchaImage(), plusoner->proxyToString());
      }
   }
   else
   {
      m_counters.error++;
   }

   updateCounters();
}

/*
 * Один из плюсонеров завершил голосование с капчей
 */
void GUI::slot_voteRequestFinished()
{
   Plusoner * plusoner = (Plusoner*)sender();

   // Счётчики
   m_counters.vote--;
   if(plusoner->voteIsSuccess())
      m_counters.success++;
   else
      m_counters.error++;
   updateCounters();
}

/*
 * Один из плюсонеров прислал сообщение
 */
void GUI::slot_newMessage(QString msg)
{
   printMessage(msg, 1);
}

/*
 * Нажата кнопка "Принять" прокси
 */
void GUI::slot_AcceptProxiesButtonPresed()
{
   // Если прокси ранее не были приняты, принимаем
   if(!m_proxies_is_accepted)
   {
      m_proxylist.clear();
      m_proxylist.addFromText(ui.editor_proxies->toPlainText());
      ui.editor_proxies->clear();

      if(m_proxylist.count() > 0)
      {
         m_proxies_is_accepted = true;

         printMessage(QString("%1 прокси загружены.").arg(m_proxylist.count()), 0);

         // Создаём плюсонеры
         stopPlusoners();
         deletePlusoners();
         createPlusoners();

         // Счётчики
         updateCounters();

         // Блокируем элементы управления во вкладке "Прокси"
         ui.editor_proxies->setPlainText(m_proxylist.toString());
         ui.button_proxies_accept->setText("Очистить");
         ui.button_proxies_from_file->setEnabled(false);
         ui.editor_proxies->setReadOnly(true);
         ui.spinbox_ignore_proxies->setEnabled(false);
         ui.spinbox_max_proxies->setEnabled(false);
      }

      ui.label_proxy_count->setNum(m_proxylist.count());
   }

   // Иначе очищаем имеющиеся прокси
   else
   {
      m_proxies_is_accepted = false;
      m_proxylist.clear();

      // Удаляем плюсонеры
      stopPlusoners();
      deletePlusoners();

      // Счётчики
      updateCounters();

      // Разблокируем элементы управления во вкладке "Прокси"
      ui.button_proxies_accept->setText("Принять");
      ui.button_proxies_from_file->setEnabled(true);
      ui.editor_proxies->setReadOnly(false);
      ui.editor_proxies->clear();
      ui.spinbox_ignore_proxies->setEnabled(true);
      ui.spinbox_max_proxies->setEnabled(true);

      ui.label_proxy_count->setNum(m_proxylist.count());
   }
}
