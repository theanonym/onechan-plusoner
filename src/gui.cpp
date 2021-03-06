/**
*
* (C) 2013 Theanonym
*
* https://github.com/theanonym/onechan-plusoner
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
   m_rate     = 1;
   m_thread   = -1;
   m_attempts = -1;
   m_timeout  = -1;
   m_max_connections = -1;

   /*
    * Элементы управления во вкладке "Основное"
    */
   connect(ui.button_start,        SIGNAL(clicked()),                   SLOT(slStartButtonPressed()));
   connect(ui.combobox_rate,       SIGNAL(currentIndexChanged(int)),    SLOT(slRateChanged(int)));
   connect(ui.line_thread,         SIGNAL(textChanged(QString)),        SLOT(slThreadChanged(QString)));
   connect(ui.line_captcha,        SIGNAL(returnPressed()),             SLOT(slCaptchaEntered()));
   connect(ui.combobox_loglevel,   SIGNAL(currentIndexChanged(int)),    SLOT(slLogLevelChanged(int)));
   connect(ui.button_output_clear, SIGNAL(clicked()), ui.editor_output, SLOT(clear()));

   ui.line_thread->setValidator(new QRegExpValidator(QRegExp("[0-9 ]{1,20}"), this));
   ui.line_captcha->setEnabled(false);
   ui.tabs->setCurrentIndex(0);
   ui.line_thread->setFocus();
   ui.editor_output->setReadOnly(true);
   ui.combobox_loglevel->setCurrentIndex(0);
   ui.combobox_rate->setCurrentIndex(0);

   /*
    * Элементы управления и сигналы во вкладке "Прокси"
    */
   connect(ui.button_proxies_accept,    SIGNAL(clicked()), SLOT(slAcceptProxiesButtonPressed()));
   connect(ui.button_proxies_from_file, SIGNAL(clicked()), SLOT(slLoadProxiesButtonPressed()));

   ui.spinbox_max_proxies->setValue(150);

   /*
    * Элементы управления во вкладке "Настройки"
    */
   connect(ui.spinbox_attempts,         SIGNAL(valueChanged(int)), SLOT(slAttemptsChanged(int)));
   connect(ui.spinbox_timeout,          SIGNAL(valueChanged(int)), SLOT(slTimeoutChanged(int)));
   connect(ui.spinbox_max_connections,  SIGNAL(valueChanged(int)), SLOT(slMaxConnectionsChanged(int)));

   ui.spinbox_attempts->setValue(3);
   ui.spinbox_timeout->setValue(30);
   //ui.spinbox_max_connections->setValue(50);

   // Загрузка базы с куками
   m_cookies_db_file = QDir(QApplication::applicationDirPath()).filePath("cookies.ydb");
   m_cookies_db.setFileName(m_cookies_db_file);
   if(m_cookies_db.loadFromFile())
      printMessage(QString("%1 куки загружены из 'cookies.ydb'").arg(m_cookies_db.size()), 0);
}

GUI::~GUI()
{
   deletePlusoners();

   if(!m_proxylist.isEmpty())
      m_proxylist.saveToFile("_proxies.txt"); // На будущее

   // Сохранение базы с куками
   if(!m_cookies_db.isEmpty())
   {
      if(m_cookies_db.saveToFile())
         qDebug() << "[Info] куки сохранены на диск";
      else
         qDebug() << "[Error] не удалось сохранить куки на диск";
   }
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
   ui.spinbox_attempts->setEnabled(true);
   ui.spinbox_timeout->setEnabled(true);
   //ui.spinbox_max_connections->setEnabled(true);
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
   ui.spinbox_attempts->setEnabled(false);
   ui.spinbox_timeout->setEnabled(false);
   //ui.spinbox_max_connections->setEnabled(false);
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
void GUI::displayCaptcha(QPixmap image, QString proxy)
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

      // Получение куки из базы, если есть
      if(m_cookies_db.contains(proxy.hostName()))
         plusoner->setCookies(m_cookies_db.get(proxy.hostName()));

      connect(plusoner, SIGNAL(siNewMessage(QString)),                SLOT(slNewMessage(QString)));
      connect(plusoner, SIGNAL(siNewCookie(QString, QString)),        SLOT(slNewCookie(QString, QString)));
      connect(plusoner, SIGNAL(siCaptchaRequestFinished(Plusoner *)), SLOT(slCaptchaRequestFinished(Plusoner *)));
      connect(plusoner, SIGNAL(siTryVoteRequestFinished(Plusoner *)), SLOT(slTryVoteRequestFinished(Plusoner *)));
      connect(plusoner, SIGNAL(siVoteRequestFinished(Plusoner *)),    SLOT(slVoteRequestFinished(Plusoner *)));

      // Создание нового потока для этого плюсонера
      PlusonerThread * thread = new PlusonerThread();
      thread->setPlusoner(plusoner);
      plusoner->moveToThread(thread);

      // Сохранение указателей для дальнейшего использования
      m_plusoners.append(plusoner);
      m_threads.append(thread);
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
      {
         thread->quit();
         thread->wait();
      }

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
         plusoner->setTimeout(m_timeout);
      }
   }
}

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
      QMetaObject::invokeMethod(plusoner, "slStop");
   }

   m_is_running = false;

   m_counters.waiting = m_plusoners.count();
}

void GUI::acceptProxies()
{
   m_proxies_is_accepted = true;

   printMessage(QString("%1 прокси загружены.").arg(m_proxylist.count()), 0);

   // Создаём плюсонеры
   stopPlusoners();
   deletePlusoners();
   createPlusoners();

   // Блокируем элементы управления во вкладке "Прокси"
   ui.editor_proxies->clear();
   ui.editor_proxies->setPlainText(m_proxylist.toString());
   ui.button_proxies_accept->setText("Очистить");
   ui.button_proxies_from_file->setEnabled(false);
   ui.editor_proxies->setReadOnly(true);
   ui.spinbox_ignore_proxies->setEnabled(false);
   ui.spinbox_max_proxies->setEnabled(false);

   ui.label_proxy_count->setNum(m_proxylist.count());
}

void GUI::clearProxies()
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

/*
 * Нажата кнопка старт/стоп
 */
void GUI::slStartButtonPressed()
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
      printMessage("Запуск.", 0);

      // Меняем текст кнопки на "Стоп" и блокируем гуй
      ui.button_start->setText("Стоп");
      disableGUI();

      // Запускаем все плюсонеры
      setPlusoners();
      foreach(Plusoner * plusoner, m_plusoners)
      {
         plusoner->setAttempts(m_attempts);
         QMetaObject::invokeMethod(plusoner, "slSendTryVoteRequest");
         m_counters.waiting--;
         m_counters.try_vote++;
      }

   }

   // Если запущена - останавливаем
   else
   {
      ui.button_start->setEnabled(false);

      m_is_running = false;

      // Если капча отображается, убираем
      if(m_captcha_displayed)
         hideCaptcha();

      // Останавливаем все плюсонеры
      stopPlusoners();
      m_counters.waiting = 0; // Костыль, чтобы не изменять функцию stopPlusoners()

      // Пауза, чтобы успели завершиться
      QEventLoop loop;
      QTimer::singleShot(0.5 * 1000, &loop, SLOT(quit()));
      loop.exec();

      printMessage("Остановлено.", 0);

      // Меняем текст кнопки на "Старт" и разблокируем гуй
      ui.button_start->setText("Старт");
      enableGUI();

      // Счётчики
      m_counters.reset();
      m_counters.waiting = m_plusoners.count();
      updateCounters();

      ui.button_start->setEnabled(true);
   }
}


/*
 * Изменён голос (плюс/минус)
 */
void GUI::slRateChanged(int index)
{
   switch(index)
   {
      case 0: m_rate = 1; break;
      case 1: m_rate = 0; break;
      default: m_rate = 1;
   }

   setPlusoners();
}

/*
 * Изменён тред
 */
void GUI::slThreadChanged(QString thread)
{
   thread.remove(' ');

   if(!thread.isEmpty())
      m_thread = thread.toInt();
   else
      m_thread = -1;

   setPlusoners();
}

/*
 * Изменено количество попыток
 */
void GUI::slAttemptsChanged(int value)
{
   m_attempts = value;

   setPlusoners();
}

/*
 * Изменён таймаут
 */
void GUI::slTimeoutChanged(int value)
{
   m_timeout = value;

   setPlusoners();
}

/*
 * Изменён лимит соединений
 */
void GUI::slMaxConnectionsChanged(int value)
{
   Q_UNUSED(value);
}

/*
 * Изменён уровень вывода сообщений
 */
void GUI::slLogLevelChanged(int index)
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
void GUI::slCaptchaEntered()
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
      plusoner->setAttempts(m_attempts);
      QMetaObject::invokeMethod(plusoner, "slSendVoteRequest");

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
void GUI::slTryVoteRequestFinished(Plusoner * plusoner)
{
   // Успешно
   if(plusoner->tryVoteIsSuccess())
   {
      m_counters.try_vote--;
      m_counters.success++;
   }

   // Нужна капча
   else if(plusoner->needCaptcha())
   {
      // Отправляем запрос капчи
      plusoner->setAttempts(m_attempts);
      QMetaObject::invokeMethod(plusoner, "slSendCaptchaRequest");

      m_counters.try_vote--;
      m_counters.captcha++;
   }

   // Нудача, остались попытки
   else if(plusoner->hasAttempts())
   {
      // Отправляем повторный запрос
      QMetaObject::invokeMethod(plusoner, "slSendTryVoteRequest");
   }

   // Неудача, попыток не осталось
   else
   {
      m_counters.try_vote--;
      m_counters.error++;
   }

   updateCounters();

   // Если все плюсонеры завершились, нажимаем кнопку "Стоп"
   if(m_counters.error + m_counters.success == m_plusoners.count())
      ui.button_start->click();
}

/*
 * Один из плюсонеров завершил запрос капчи
 */
void GUI::slCaptchaRequestFinished(Plusoner * plusoner)
{
   // Успешно
   if(plusoner->captchaIsSuccess())
   {
      // Вставляем капчу в очередь
      m_captcha_queue.enqueue(plusoner);

      // Если капча в данный момент не отображается,
      // отображаем новую капчу
      if(!m_captcha_displayed)
         displayCaptcha(plusoner->getCaptchaImage(), plusoner->proxyToString());

      m_counters.captcha--;
      m_counters.enter++;
   }

   // Нудача, остались попытки
   else if(plusoner->hasAttempts())
   {
      // Отправляем повторный запрос
      QMetaObject::invokeMethod(plusoner, "slSendCaptchaRequest");
   }

   // Неудача, попыток не осталось
   else
   {
      m_counters.captcha--;
      m_counters.error++;
   }

   updateCounters();

   // Если все плюсонеры завершились, нажимаем кнопку "Стоп"
   if(m_counters.error + m_counters.success == m_plusoners.count())
      ui.button_start->click();
}

/*
 * Один из плюсонеров завершил голосование с капчей
 */
void GUI::slVoteRequestFinished(Plusoner * plusoner)
{
   // Успешно
   if(plusoner->voteIsSuccess())
   {
      m_counters.vote--;
      m_counters.success++;
   }

   // Нудача, остались попытки
   else if(plusoner->hasAttempts())
   {
      // Отправляем повторный запрос
      QMetaObject::invokeMethod(plusoner, "slSendVoteRequest");
   }

   // Неудача, попыток не осталось
   else
   {
      m_counters.vote--;
      m_counters.error++;
   }

   updateCounters();

   // Если все плюсонеры завершились, нажимаем кнопку "Стоп"
   if(m_counters.error + m_counters.success == m_plusoners.count())
      ui.button_start->click();
}

/*
 * Один из плюсонеров прислал сообщение
 */
void GUI::slNewMessage(QString msg)
{
   printMessage(msg, 1);
}

/*
 * Один из плюсонеров получил куки
 */
void GUI::slNewCookie(QString proxy, QString raw_cookies)
{
   m_cookies_db.insert(proxy, raw_cookies);
}

/*
 * Нажата кнопка "Принять" прокси
 */
void GUI::slAcceptProxiesButtonPressed()
{
   // Если прокси ранее не были приняты, принимаем
   if(!m_proxies_is_accepted)
   {
      m_proxylist.clear();
      m_proxylist.addFromText(ui.editor_proxies->toPlainText());

      if(!m_proxylist.isEmpty())
      {
         acceptProxies();
         updateCounters();
      }
      else
      {
         ui.editor_proxies->clear();
      }
   }

   // Иначе очищаем имеющиеся прокси
   else
   {
      clearProxies();
   }
}

void GUI::slLoadProxiesButtonPressed()
{
   QString fname = QFileDialog::getOpenFileName(this, "Окрыть файл", QApplication::applicationDirPath());

   if(fname.isEmpty())
      return;

   QFile file(fname);
   if(file.open(QIODevice::ReadOnly))
      ui.editor_proxies->setPlainText(file.readAll());
}
