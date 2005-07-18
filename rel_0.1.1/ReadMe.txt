
KONNferencja - obs³uga konferencji dla protoko³u Gadu-Gadu

(c)2003 Rafa³ Lindemann / www.stamina.eu.org / www.konnekt.info

Kod udostêpniany jest na licencji GPL.
Licensed under GPL.

Grupa Stamina zastrzega sobie prawo wykorzystania kodu w³asnego autorstwa w
wersji zamkniêtej, poza licencj¹ GPL.

---------------------------------------------------------------------

Udostêpniamy kod g³ównie w celach edukacyjnych, aby pokazaæ, w jaki
sposób mo¿na rozszerzyæ mo¿liwoœci programu korzystaj¹c tylko z SDK.

Zasada dzia³ania wtyczki jest bardzo prosta:
  - po uruchomieniu dopinamy siê do gg.dll, które obs³uguje odpowiednie
    w³asne komunikaty.
  - ka¿da konferencja to osobny KONTAKT na liœcie. W ten sposób
    mo¿emy przechowywaæ grupy kontaktów do rozmowy na potem...
  - jako ¿e Interfejs na dzieñ dzisiejszy obs³uguje tylko zwyk³e
    wiadomoœci, a my nie chcemy pisaæ w³asnego okna rozmowy,
    konferencje bêd¹ obs³ugiwane tak, jak zwyk³e wiadomoœci.
  - lista odbiorców, oddzielonych znakiem ';' zapisywana jest jako UID kontaktu
    po znaku @ na koñcu zapisany jest numer sieci.

Szczegó³y w poszczególnych plikach Ÿród³owych.

---------------------------------------------------------------------

Proszê ZAZNACZAÆ KA¯D¥ modyfikacjê w kodzie krótkim komentarzem,
razem z inicja³ami autora. Rozpowszechnianie innych wersji, zgodnie
z lic. GPL jest mo¿liwe TYLKO razem z pe³nym kodem Ÿród³owym.

---------------------------------------------------------------------

Wszelkie pytania prosimy zadawaæ na http://forum.konnekt.info/


Przed skompilowanie z u¿yciem .vcproj proszê KONIECZNIE ustawiæ SWOJE œcie¿ki!