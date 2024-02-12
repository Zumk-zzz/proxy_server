# proxy_server
Данный прокси сервер перехватывает sql запросы.
Исходное задание - Разработать на C++ TCP прокси-сервер для СУБД Postgresql с возможностью логирования всех SQL запросов, проходящих через него.

Настройка:
Что бы данный прокси сервер работал, нужно произвести настроку самого Postgresql
1) Нужео настроить подключение, что бы Postgresql отправлял свои запросв через порт, на котором идет прослушка прокси сервера
для этого мы заходим в файл postgresql.conf
и изменяем пареметр port = 9999
2) Если прокси запущен на другом ip то нужно выполнить подключение, для этого нужно-
перейти в файл postgresql.conf. и установить значения listen_addresses = 'ip_proxy_server,ip_Postgresql'
Также нужно внести изменения в pg_hba.conf
нужно прописать ip proxy_server
host    all             all           ip_proxy_server/32         md5
после всез изменений перезапустиь postgresql.
Запуск
Для запуска proxy нужно перейти в папку Build и написать
Cmake ..
make
после чего программа запустится
для очтновки достаточно нажать в консоли в который вы ее запустили, сочетание клавишь CTRL + C
В программе присутвсвуют оьработчики ощибок, которые не дадут выйти программе за пределы возможного.



