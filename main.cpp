 /**
 *
 * (C) 2013 Theanonym
 *
 * https://github.com/theanonym/yoba-onechan-plusoner
 *
 */

#include <QApplication>
#include <QTextCodec>
#include <QtGlobal>

#include "sources/gui.hpp"

int main(int argc, char ** argv)
{
   QApplication app(argc, argv);
   QApplication::setWindowIcon(QIcon(":/1chan.ico"));

#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
   QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"));
#endif
   QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));

   GUI gui;
   gui.setWindowTitle("Плюсонер");
   gui.show();

   return app.exec();
}
