/**
*
* (C) 2013 Theanonym
*
* https://github.com/theanonym/yoba-onechan-plusoner
*
*/

#include "yobadb.hpp"

YobaDB::YobaDB(QObject * parent)
   : QObject(parent)
{

}

YobaDB::YobaDB(const QString & fname, QObject * parent)
   : QObject(parent)
{
   m_fname = fname;
}

bool YobaDB::saveToFile(const QString & fname) const
{
   if(isEmpty())
      return false;

   QFile file(fname);
   if(file.open(QFile::WriteOnly))
   {
      QMap<QString, QString>::const_iterator it = m_db.begin();
      while(it != m_db.end())
      {
         file.write(QString("KEY{[(\"%1\")]}=VALUE{[(\"%2\")]}").arg(it.key(), it.value()).toAscii());
         file.putChar('\n');
         it++;
      }

      return true;
   }
   else
   {
      return false;
   }
}

bool YobaDB::saveToFile() const
{
   if(!hasFileName())
      return false;

   return saveToFile(m_fname);
}

bool YobaDB::loadFromFile(const QString & fname)
{
   clear();

   static QRegExp regex("KEY\\{\\[\\(\"(.*)\"\\)\\]\\}=VALUE\\{\\[\\(\"(.*)\"\\)\\]\\}");
   regex.setMinimal(true);

   QFile file(fname);
   if(file.open(QFile::ReadOnly))
   {
      QString data = file.readAll();

      int pos = 0;
      while((pos = data.indexOf(regex, pos)) != -1)
      {
         QString key   = regex.cap(1);
         QString value = regex.cap(2);

         m_db.insert(key, value);

         pos += regex.cap(0).length() + 1;
      }

      return !isEmpty();
   }
   else
   {
      return false;
   }
}

bool YobaDB::loadFromFile()
{
   if(!hasFileName())
      return false;

   return loadFromFile(m_fname);
}
