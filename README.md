# publishSubscribe

pliki mozna skompilowac uzywajac skryptu ./skrypt,
moze byc potrzebne wczesniejsze nadanie uprawnien wykonywania
za pomoca polecenia chmod -x skrypt



plik s.c odpowiada za serwer
plik k.c odpowiada za klienta



klientow i serwer mozna uruchomic w dowolnej kolejnosci, przy czym
uruchomiony moze byc tylko jeden serwer i wielu klientow.



klienci domyslnie uruchamiaja sie jako odbiorcy synchroniczni, jednak
mozna to zmienic na etapie uruchamiania podajac parametr asynchr



Jezeli aktualnie nie ma serwera, klienci czekaja na mozliwosc rejestracji.
Gdy serwer sie pojawi klient proszony jest o podanie hasla
uzywajac tego hasla moze sie zalogowac do na serwer (wczesniej wszystkie inne
opcje beda blokowane przez serwer). Po trzech nieudanych probach logowania 
uzytkownik jest blokowany i juz nie moze sie zalogowac.



Z poziomu klienta dostepne opcje to:

    1. Logowanie

    2. Rejestracja na konkretne wiadomosci - uzytkownik moze podac typ wiadomosci

        ktory chce subskrybowac i w jaki sposob (czy powiadamiac serwer itp.)

    3. Blokada innego uzytkownika - podajac jego pid (pid wyswietla sie tuz po zalogowaniu)

    4. Stworzenie nowego typu wiadomosci - recznie lub automatycznie, nie mozna stworzyc

        dwa razy tego samego typu

    5. Rozglaszanie wiadomosci - wiadomosc o podanym typie i priorytecie zostanie przyjeta

        przez serwer tylko jezeli dany typ istnieje

    6. Odbior wiadomosci synchronicznie - opcja dostepna tylko dla klientow synchronicznych,

        poniewaz dla asynchronicznych wiadomosci odbierane sa od razu po wyslaniu przez serwer

    0. Wyjscie z programu


Serwer wylaczamy przy uzyciu kombinacji klawiszy ctrl+c, po otrzymaniu sygnalu serwer usuwa wszystkie kolejki ipc

Projekt zawiera dodadkowo mozliwosc blokowania uzytkownika po 3 probach logowania oraz na zyczenie klienta.
