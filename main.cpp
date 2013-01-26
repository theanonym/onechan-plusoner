 /**
 *
 * (C) 2013 Theanonym
 *
 * https://github.com/theanonym/yoba-onechan-plusoner
 *
 */

#include <QApplication>
#include <QTextCodec>
#include <QDir>

#include "sources/gui.hpp"

int main(int argc, char ** argv)
{
   QApplication app(argc, argv);
   QApplication::setWindowIcon(QIcon(":/1chan.ico"));

   QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"));
   QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));

   QDir cookies_dir = QDir(QApplication::applicationDirPath()).filePath("cookies");
   if(!cookies_dir.exists())
      cookies_dir.mkpath(".");

   GUI gui;
   QObject::connect(&gui, SIGNAL(destroyed()), &app, SLOT(quit()));
   gui.setWindowTitle("Плюсонер [beta]");
   gui.show();

   return app.exec();
}
