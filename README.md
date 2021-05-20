# FarmerMafia
Verktøy for å regulere såkorn - og gjødselsmengde på åkeren, ved hjelp av QGIS, Arduino, og GPS, pluss en liten skjerm og noen knapper (skjermen er for å ha en liten meny, og litt info). Prosjektet er laget for en Vaderstad Rapid maskin, med med tilhørende elektromotorer, som brukes for å regulere såkorn - og gjødselsmengde. Motorene styres i utgangspunktet manuelt ved å trykke på knapper (opp, ned og 0), men vi har automatisert denne prosessen ved å bruke releer. Mye av koden kan nok gjenbrukes om man ønsker å gjøre noe lignende på en annen type såmaskin.

Om det er spørsmål som ikke besvares i dette dokumentet, så er det bare å kontakte via f.eks. mail: mortensvendgaard@gmail.com, eller kontakte
Morten Svendgård på andre måter (f.eks. Facebook, eller mobil).

## Prosjektbeskrivelse
Prosjektet består av to deler. Første del er Arduino-koden, som ligger i Ardu-mappen, og andre del er et PC-program som ligger i Fielder-mappen.

PC-programet (konsollprogram), brukes for å konvertere GeoJson - filer (fra QGIS), til et eget filformat, som så kan leses av
Arduinoen. Arduinoen bruker så denne dataen, sammen med GPS-data, for å regulere såmaskinen, til å slippe ut ønskede mengder.

Foreløpig reguleres såkorn- og gjødselsmengden av samme tildelingsfil, men veldig lite jobb må til for at disse skal kunne
reguleres hver for seg.

## QGIS - guide
Liten guide for hvordan man lager GeoJson for dette prosjektet.

QGIS må være installert, og ønskede bakgrunnskart må være lagt til. F.eks. Google satelittbilder kan legges til, også mulig å
legge til bilder fra kartverket.

1. Velg "Nytt lag på ESRI Shapefile-format" oppe til venstre.
2. Velg hvor man ønsker å lagre Shapefilen. Velg geometritype "Flate". 
   Under "Nytt felt" - boksen, lag et felt med navn "value", og type "heltall".
   Trykk på "legg til i feltliste".
   Kan så slette "id"-feltet (ikke viktig, men "id"-feltet er bare i veien).
3. Anbefaler å bruke "EPSG:4326 - WGS 84" som koordinatsystem (så man er sikker på
   at det er dette formatet det konverteres til senere) (velges litt under feltet Geometritype).
4. Trykk "OK" - for å lage laget.
5. Sørg for at laget du nettopp lagde er valgt, og trykk på den gule pennen for å redigere laget. Velg så "Legg til flate" (litt til høyre for pennen),
   så lager du en flate, og gir den en ønsket "value". "value" er verdien som vil representere den såkorn- eller gjødselsmengden som skal gis ut
   på denne flaten. Denne verdien MÅ være mellom -128 og 126. Ønsker man andre verdier, så må man forandre koden, men man har tross alt mulighet
   for 254 ulike verdier her.
6. Når man har lagt til så mange flater/polygoner som man ønsker, så høyreklikker man på laget, går til "Eksporter" og "Lagre objekter som".
   Velg "GeoJSON" som Format. Velg hvilken mappe filen skal lagres i. VIKTIG at KRS er valgt til "EPSG:4326 - WGS 84". Viktig at "value"-feltet er huket av for eksportering 
   (skal være det i utgangspunktet). Velg geometritype "Polygon". På "RFC7946" velg "YES".
7. Trykk så "OK" - for å eksportere filen.
   
Da er QGIS - guiden ferdig.
Et tips er å lage et eget tildelingskart for de rundene man skal kjøre fotganger, så slipper man justeringer hver gang man kommer til enden av åkeren, når man kjører normalt.

## Konvertere GeoJson filen
For å konveretere GeoJson filen til vårt eget format, bruker vi programmet Fielder.exe, der kildekoden ligger i Fielder-mappen.
Fielder.exe må kjøres fra konsoll (CMD). Bruk "cd"-kommandoen for å gå til ønsket mappe. Anbefaler at Fielder.exe ligger i mappa.
Da kan man kjøre "Fielder.exe navn-på-geojson-fil navn-på-ny-fil", for å genere filen som skal legges på SD-kortet.

Man må følge 8.3-navnestandaren for filer. Det vil si at "navn-på-generert-fil" kan ha maks 8 bokstaver, og
eventuelt en filending på 3 bokstaver. F.eks. har vi en åker vi kaller "Bekkenget". Denne har 9 bokstaver,
så filnavnet for selvet åkeren, lar vi være "bekkeng", og filnavn for fotgangen til denne åkeren er "bekkeng.fot".

Når man har laget filer for alle åkrene, så legger man de på SD-kortet, slik at de kan leses av Arduinoen. (Ikke legg filene i mapper på SD-kortet, da vil ikke koden finne de).

## Utstyrliste
Her er en liste over utstyret vi brukte for prosjektet:
* GNSS Multi-Band Magnetic Mount Antenna https://www.sparkfun.com/products/15192
* SparkFun GPS Breakout - NEO-M9N, SMA (Qwiic) https://www.sparkfun.com/products/17285
* HiLetgo 2pcs 1.3" IIC I2C Serial 128x64 SSH1106 OLED https://www.amazon.com/gp/product/B07BHHV844/ref=ppx_yo_dt_b_asin_title_o00_s00?ie=UTF8&psc=1
* ARDUINO MEGA 2560 REV3
* 3 Pushbuttons
* HiLetgo 5pcs Micro SD TF Card Adater Reader Module 6Pin SPI Interface https://www.amazon.com/gp/product/B07BJ2P6X6/ref=ppx_yo_dt_b_asin_title_o01_s00?ie=UTF8&psc=1
* SanDisk® microSDHC™ 8GB Memory Card https://www.amazon.com/gp/product/B0012Y2LLE/ref=ppx_yo_dt_b_asin_title_o01_s00?ie=UTF8&psc=1

Flere av disse produktene kan nok godt erstattes med andre alternativer om man ønsker. (Håper jeg ikke glemte noe)

## GPS-setup
GPSen som brukes i dette prosjektet er en NEO-M9N, levert av SparkFun.
GPSen må kobles opp for UART/Serial kommunikasjon, for å fungere med koden i prosjektet.
I tillegg må GPSen være konfigurert for å sende NAV-PVT pakker, over UBX-protokollen (ikke NMEA).
Dette kan gjøres via U-center programmet, som kan lastes ned på PC (kan være litt vanskelig å sette seg inn i).
Her kan man også justere hvor ofte det sendes data fra GPSen. I dette prosjektet har vi intervaller på 1 sekund (tror det er originalverdien).

---

Koden ligger under GPLv3 - lisens (se LICENSE fil)
