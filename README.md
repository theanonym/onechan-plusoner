![Скриншот](https://github.com/theanonym/yoba-onechan-plusoner/raw/master/screenshot.png)

Это незаконченный проект. Вы можете присоединиться и доделать его.

## Загрузки
* [Windows](https://raw.github.com/theanonym/yoba-onechan-plusoner/master/windows.zip) (.dll файлы в комплекте)

## Как собрать

    $ git clone git://github.com/theanonym/yoba-onechan-plusoner.git
    $ cd yoba-onechan-plusoner
    $ qmake
    $ make
    $ ./plusoner

Для сборки лучше иметь свежую версию Qt, гарантированно собирается под 4.8.2 и выше.

## Баги
* У некоторых под виндой почему-то не отображается капча.
* Под виндой работает хуже, чем под линуксом, меньше прокси оказываются рабочими.
* Счётчики иногда отображают хуиту (на работу это не влияет).
* При долгой работе иногда крашится с ошибкой сегментирования.
