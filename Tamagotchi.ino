#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544.h>
#include <EEPROM.h>

// Mit extern deklarieren, dass die Bitmaps sich in der anderen Datei befinden
extern const unsigned char GrosseKatze[];
extern const unsigned char Herzen[];
extern const unsigned char Burger[];
extern const unsigned char Pommes[];
extern const unsigned char Eis[];
extern const unsigned char Kuchen[];
extern const unsigned char Cookie[];
extern const unsigned char Fisch[];
extern const unsigned char Pizza[];
extern const unsigned char Bretzeln[];
extern const unsigned char Tod[];
extern const unsigned char SleepingCat[];
extern const unsigned char KleineKatze[];
extern const unsigned char Fun[];
extern const unsigned char Hunger[];
extern const unsigned char Sleep[];
extern const unsigned char Schleife[];
extern const unsigned char Hut[];
extern const unsigned char HutShop[];

// initialisieren
Adafruit_PCD8544 display = Adafruit_PCD8544(7, 6, 5, 4, 3);

int tasterPinUp = 11;
int tasterPinMiddle = 12;
int tasterPinDown = 13;


String menuPunkte[] = { "Essen", "Schlafen", "Spielen", "Shop", "Speichern", "Zurueck" };
int menuSize = 6;
int menuSelektiert = 0;

String essenNamen[] = { "Burger", "Pommes", "Eis", "Kuchen", "Cookie", "Fisch", "Pizza", "Bretzeln", "Zurueck" };
int essenPreis[] = { 21, 15, 12, 14, 8, 15, 19, 13, 0 };
int essenPoints[] = { 17, 10, 7, 9, 5, 12, 15, 9, 0 };
int essenSize = 9;
int essenSelektiert = 0;

String shopNamen[] = { "Schleife", "Hut" , "Zurueck" };
int shopPreis[] = { 70, 130, 0 };
int shopSize = 3;
int shopSelektiert = 0;

bool itemsBesessen[] = { false, false, false };

int hungerstatus = 100;
int schlafstatus = 100;
int spassstatus = 100;
int geld = 300;

// intervall wann der Status immer sinkt
const unsigned long intervall = 300000;

unsigned long nowMillis;
unsigned long lastMillis;

// wieviel an Status verloren wird
int minusProzent = 5;

// enum für die verschiedenen Spielzustände
enum MusterZustand { MUSTER_ANZEIGEN,
                     WARTE_AUF_EINGABE,
                     ZEIGE_ERGEBNIS };
MusterZustand musterZustand = MUSTER_ANZEIGEN;

// Muster Eigenschaften
int muster[20];
int musterLaenge = 3;
int eingabeIndex = 0;
bool hatVerloren = false;
int score = 0;
unsigned long letzteAktion = 0;


// enum für die verschiedenen Anzeigezustände
enum Zustand { STARTBILDSCHIRM,
               HAUPTMENUE,
               ESSENMENUE,
               SHOPMENUE,
               TODBILDSCHIRM,
               SCHLAFENBILDSCHIRM,
               SPIEL_MUSTER };
Zustand aktuellerZustand = STARTBILDSCHIRM;


void setup() {
  pinMode(tasterPinUp, INPUT_PULLUP);
  pinMode(tasterPinMiddle, INPUT_PULLUP);
  pinMode(tasterPinDown, INPUT_PULLUP);

  Serial.begin(9600);
  display.begin();
  display.setContrast(57);
  display.setTextSize(1);
  display.setTextColor(BLACK);

  // anfangs gleich Spielstand laden und 
  laden();
}

void loop() {

  nowMillis = millis();

  // Prüfung ob die 5 Minuten schon um sind damit man die Prozente abzieht
  if (nowMillis - lastMillis >= intervall) {
    lastMillis = nowMillis;

    if (schlafstatus > 0 && hungerstatus > 0 && spassstatus > 0) {
      schlafstatus = schlafstatus - minusProzent;
      hungerstatus = hungerstatus - minusProzent;
      spassstatus = spassstatus - minusProzent;
    } else {
      // Katze gehts nicht gut also tot und Spielstand löschen
      aktuellerZustand = TODBILDSCHIRM;
      loeschen();
    }
  }

    // Ich weiß das ist doppelt gemoppelt aber ich finds übersichtlicher
    // Je nach Zustand wird die passende Methode für die Anzeige aufgerufen
  if (aktuellerZustand == SCHLAFENBILDSCHIRM) {
    schlafenbildschirm();
  }

  if (aktuellerZustand == TODBILDSCHIRM) {
    todbildschirm();
  }

  if (aktuellerZustand == ESSENMENUE) {
    essenMenue();
  }

  if (aktuellerZustand == SHOPMENUE) {
    shopMenue();
  }

  if (aktuellerZustand == SPIEL_MUSTER) {
    spiel_MusterMerken();
  }

  if (aktuellerZustand == STARTBILDSCHIRM) {
    startbildschirm();
  }

  if (aktuellerZustand == HAUPTMENUE) {
    menu();
  }

  // Wenn der Zustand = Startbildschirm, dann kommt man mit dem Middle Button ins Hauptmenü
  if (aktuellerZustand == STARTBILDSCHIRM) {
    if (digitalRead(tasterPinMiddle) == LOW) {
      aktuellerZustand = HAUPTMENUE;
      menu();
      delay(200);
    }
  }

  // Wenn Zustand = Hauptmenü, dann kann man mit Up und Down die Punkte auswählen und der Zähler geht hoch oder runter
  if (aktuellerZustand == HAUPTMENUE) {
    if (digitalRead(tasterPinDown) == LOW) {
      menuSelektiert++;
      if (menuSelektiert >= menuSize) {
        menuSelektiert = 0;
      }
      menu();
      delay(200);
    }

    // Mit dem Middle Button wird einfach der Zähler überprüft und je nachdem auf den Zustand gesetzt, damit die Anzeige geändert wird und auch die Knopffunktionen sich ändern
    if (digitalRead(tasterPinMiddle) == LOW) {
      if (menuSelektiert == 0) {
        aktuellerZustand = ESSENMENUE;
        essenMenue();
      } else if (menuSelektiert == 1) {
        aktuellerZustand = SCHLAFENBILDSCHIRM;
        schlafenbildschirm();
      } else if (menuSelektiert == 5) {
        aktuellerZustand = STARTBILDSCHIRM;
        startbildschirm();
      } else if (menuSelektiert == 2) {
        musterLaenge = 3;
        eingabeIndex = 0;
        score = 0;
        hatVerloren = false;
        musterZustand = MUSTER_ANZEIGEN;
        aktuellerZustand = SPIEL_MUSTER;
        spiel_MusterMerken();
      } else if (menuSelektiert == 3) {
        aktuellerZustand = SHOPMENUE;
        shopMenue();
      } else if (menuSelektiert == 4) {
       speichern();
        }
        // falls auf mysteriöse Weise irgendwas unter 0 oder über 5 rauskommt wird halt eine süße Katze angezeigt könnte man aber weglassen
       else {
        display.clearDisplay();
        display.drawBitmap(30, 12, GrosseKatze, 29, 32, BLACK);
        display.drawBitmap(15, 10, Herzen, 9, 16, BLACK);
        display.display();
        delay(2000);
        menu();
      }
      delay(200);
    }

    if (digitalRead(tasterPinUp) == LOW) {
      menuSelektiert--;
      if (menuSelektiert < 0) {
        menuSelektiert = menuSize - 1;
      }
      menu();
      delay(200);
    }
  }

  // Wenn Zustand = Essenmenü, dann werden die Bitmaps mit dem Essen angezeigt
  if (aktuellerZustand == ESSENMENUE) {
    // So wie im Menü mit up und down auswählbar
    if (digitalRead(tasterPinUp) == LOW) {
      essenSelektiert--;
      if (essenSelektiert < 0) essenSelektiert = essenSize - 1;
      essenMenue();
      delay(200);
    }

    if (digitalRead(tasterPinDown) == LOW) {
      essenSelektiert++;
      if (essenSelektiert >= essenSize) essenSelektiert = 0;
      essenMenue();
      delay(200);
    }

    // Wenn mit Middle bestätigt wird, dann wird das Geld abgezogen
    if (digitalRead(tasterPinMiddle) == LOW) {
      display.clearDisplay();
      if (geld >= essenPreis[essenSelektiert]) {
        // Wenns Zurück ist soll natürlich nichts passieren
        if (essenNamen[essenSelektiert] != "Zurueck") {
          // Geld wird abgezogen sowie das Min aus den beiden Werten 100 und dem Wert nachdem die Katze gefüttert wurde genommen, damit es nicht über 100 geht
          // Katze kann also unendlich gefüttert werden, vielleicht kann man sie zukünftig fetter machen
          geld -= essenPreis[essenSelektiert];
          hungerstatus = min(100, hungerstatus + (essenPoints[essenSelektiert]));
          display.drawBitmap(30, 12, GrosseKatze, 29, 32, BLACK);
          display.drawBitmap(15, 10, Herzen, 9, 16, BLACK);
          display.display();
          delay(1000);
        }
      } else {
        display.print("Nicht genuegend Geld!");
        display.display();
        delay(2000);
      }

      aktuellerZustand = HAUPTMENUE;
      delay(200);
    }
  }

  // Genau gleich wie beim Essensmenü
  if (aktuellerZustand == SHOPMENUE) {
    if (digitalRead(tasterPinUp) == LOW) {
      shopSelektiert--;
      if (shopSelektiert < 0) shopSelektiert = shopSize - 1;
      shopMenue();
      delay(200);
    }

    if (digitalRead(tasterPinDown) == LOW) {
      shopSelektiert++;
      if (shopSelektiert >= shopSize) shopSelektiert = 0;
      aktuellerZustand = SHOPMENUE;
      shopMenue();
      delay(200);
    }

    if (digitalRead(tasterPinMiddle) == LOW) {

      display.clearDisplay();
      if (geld >= shopPreis[shopSelektiert]) {
        if (shopNamen[shopSelektiert] != "Zurueck") {
          if (itemsBesessen[shopSelektiert]) {
            display.setCursor(0, 0);
            display.print("Schon gekauft!");
            display.display();
            delay(2000);
          } else if (geld >= shopPreis[shopSelektiert]) {
            geld -= shopPreis[shopSelektiert];
            itemsBesessen[shopSelektiert] = true;

            display.drawBitmap(30, 12, GrosseKatze, 29, 32, BLACK);
            display.drawBitmap(15, 10, Herzen, 9, 16, BLACK);
            display.display();
            delay(2000);

          } else {
            display.setCursor(0, 0);
            display.print("Nicht genug Geld!");
            display.display();
            delay(2000);
          }
        }
      }

      aktuellerZustand = HAUPTMENUE;
      menu();
      delay(200);
    }
  }
}


void menu() {
  display.clearDisplay();

  // Anzeige ist immer in Vierer Gruppen, also ergebnis 4 mal die 0, 4 mal die 4, 4 mal die 8, ...
  int anzeigeStart = menuSelektiert - (menuSelektiert % 4);  

  // Pfeil nach oben wird gezeigt wenn es größer als die erste 4er Gruppe is
  if (anzeigeStart > 0) {
    display.setCursor(76, 0);
    display.print("^");
  }

  // Für jede Gruppe wird der Index mit dem anzeigestart verrechnet und dann angezeigt
  for (int i = 0; i < 4; i++) {
    int index = anzeigeStart + i;
    if (index >= menuSize) break;

    display.setCursor(0, i * 10);
    if (index == menuSelektiert) {
      display.print("> ");
    } else {
      display.print("  ");
    }

    display.println(menuPunkte[index]);
  }

  // In dem Fall wenn wir bei anzeigeStart = 0 bei der ersten Seite sind und +4 gerechnet wird sind wir bei 4
  // 4 < 6 == true also wird v angezeigt
  if (anzeigeStart + 4 < menuSize) {
    display.setCursor(76, 38);
    display.print("v");
  }

  display.display();
}

void essenMenue() {
  display.clearDisplay();

  // Ähnlich wie oben wird es in zweier gruppen geteilt
  int startIndex = (essenSelektiert / 2) * 2;

  // auch ähnlich wie oben
  for (int i = 0; i < 2; i++) {
    int index = startIndex + i;
    if (index >= essenSize) break;

    // ganz oben und mitte des bildschirm bei i = 0 und i = 1
    int y = i * 24;
    // für den Index wird aus der Hilfmethode der Name der jeweiligen Bitmap geholt
    const unsigned char* bmp = getEssenBitmapByIndex(index);

    // zeichnet dann eine schwarze Umrandung um das selektierte Items
    if (index == essenSelektiert) {
      display.drawRect(0, y, 84, 24, BLACK);
    }

    // Bei allem außer zurück wird dann die Bitmap gezeichnet (funktioniert auch nur mit der Hilfmethode, da alle Grafiken 16x16 gezeichnet wurden, sonst wird ein Pixelhaufen angezeigt)
    if (essenNamen[index] != "Zurueck") {
      display.drawBitmap(2, y + 4, bmp, 16, 16, BLACK);
    }

    // Name wird dazu angezeigt
    display.setCursor(22, y + 2);
    display.print(essenNamen[index]);

    // Preis und Hungerpoints werden gezeichnet
    if (essenNamen[index] != "Zurueck") {
      display.setCursor(22, y + 12);
      display.print("$");
      display.print(essenPreis[index]);

      display.setCursor(60, y + 12);
      display.print("+");
      display.print(essenPoints[index]);
    }
  }

  display.display();
}

void shopMenue() {
  // 1 zu 1 wie oben beim Essensmenü, nur zusätzlich wird angezeigt, ob etwas bereits gekauft wurde
  display.clearDisplay();

  int startIndex = (shopSelektiert / 2) * 2;

  for (int i = 0; i < 2; i++) {
    int index = startIndex + i;
    if (index >= shopSize) break;

    int y = i * 24;
    const unsigned char* bmp = getShopBitmapByIndex(index);

    if (index == shopSelektiert) {
      display.drawRect(0, y, 84, 24, BLACK);
    }

    if (shopNamen[index] != "Zurueck") {
      display.drawBitmap(2, y + 4, bmp, 16, 16, BLACK);
    }

    display.setCursor(22, y + 2);
    display.print(shopNamen[index]);

    if (shopNamen[index] != "Zurueck") {
      if (itemsBesessen[index]) {
        display.setCursor(22, y + 12);
        display.print("Gekauft");
      } else {
        display.setCursor(22, y + 12);
        display.print("$");
        display.print(shopPreis[index]);
      }
    }
  }
  display.display();
}


void startbildschirm() {
  display.clearDisplay();

  display.drawBitmap(42, 8, GrosseKatze, 29, 32, BLACK);
  // display.drawBitmap(15, 10, Herzen, 9, 16, BLACK);


  // Maps für den Hungerstatus, damit die Leisten nicht so groß werden
  int hungerWidth = map(hungerstatus, 0, 100, 0, 20);
  int schlafWidth = map(schlafstatus, 0, 100, 0, 20);
  int spassWidth = map(spassstatus, 0, 100, 0, 20);

  // Zeichnen der Statusleisten und des Gelds
  display.drawRect(1, 1, 20, 4, BLACK);
  display.fillRect(1, 1, hungerWidth - 1, 4, BLACK);
  display.drawBitmap(22, 1, Hunger, 3, 4, BLACK);

  display.drawRect(1, 6, 20, 4, BLACK);
  display.fillRect(1, 6, schlafWidth - 1, 4, BLACK);
  display.drawBitmap(22, 6, Sleep, 3, 4, BLACK);

  display.drawRect(1, 11, 20, 4, BLACK);
  display.fillRect(1, 11, spassWidth - 1, 4, BLACK);
  display.drawBitmap(22, 11, Fun, 3, 4, BLACK);

  display.setCursor(0, 40);
  display.print("$:");
  display.print(geld);

  // Hier wird manuell durch den Array durchgegangen, ob die Items schon in Besitz sind und gezeichnet werden
  // Konnte nicht in einer Schleife durchgegangen werden wie beim Shopmenü, da es bei der Startbildschirm-Anzeige unterschiedliche Größen hat
  if (itemsBesessen[0]) {
    display.drawBitmap(46, 16, Schleife, 16, 16, BLACK);
  }

  if (itemsBesessen[1]) {
    display.drawBitmap(42, 6, Hut, 24, 8, BLACK);
  }

  display.display();
}

// Todesbildscirm wird gezeichnet, es kann dann nichts mehr gemacht werden
void todbildschirm() {
  display.clearDisplay();
  display.drawBitmap(0, 0, Tod, 84, 48, BLACK);
  display.display();
}


// Hilfsmethode zum holen der Bitmapnamen im Essensmenü wie oben erwähnt
const unsigned char* getEssenBitmapByIndex(int index) {
  switch (index) {
    case 0: return Burger;
    case 1: return Pommes;
    case 2: return Eis;
    case 3: return Kuchen;
    case 4: return Cookie;
    case 5: return Fisch;
    case 6: return Pizza;
    case 7: return Bretzeln;
  }
}

// Hilfsmethode zum holen der Bitmapnamen im Shopmenü wie oben erwähnt
const unsigned char* getShopBitmapByIndex(int index) {
  switch (index) {
    case 0: return Schleife;
    case 1: return HutShop;
  }
}

// Schlafanzeige der Katze, zurzeit schläft sie 1 Minute lang, währenddessen kann man nichts machen und muss warten
void schlafenbildschirm() {

  const unsigned long zzzIntervall = 500;
  const unsigned long dauer = 60000;

  const char* zzzStufen[] = { "Z", "Zz", "Zzz" };
  const int stufenAnzahl = 3;
  int aktuelleStufe = 0;

  unsigned long startZeit = millis();
  unsigned long letzteAnimZeit = 0;

  // solange die Minute noch nicht um ist, wird mit dem zzzIntervall jede halbe Sekunde eine kleine Animation von Zzz angezeigt, damit man nicht denkt, dass es hängt
  while (millis() - startZeit < dauer) {
    if (millis() - letzteAnimZeit >= zzzIntervall) {
      letzteAnimZeit = millis();

      display.clearDisplay();
      display.drawBitmap(27, 8, SleepingCat, 27, 16, BLACK);

      display.setTextSize(1);
      display.setCursor(60, 0);
      display.print(zzzStufen[aktuelleStufe]);

      display.display();

      aktuelleStufe++;
      if (aktuelleStufe >= stufenAnzahl) aktuelleStufe = 0;
    }
  }


  // Ebenfalls wie beim Hunger damit es nicht über 100 geht
  schlafstatus = min(100, schlafstatus + 20);
  aktuellerZustand = HAUPTMENUE;
  menu();
}

void speichern() {
  // Markieren, dass Daten vorhanden sind
  EEPROM.update(0, 1);  
  
  // Update ist eine tolle Methode zum abspeichert von 1-Byte Werten und da ein Status nicht über 255 geht sondern nur bis 100 kann man das einfach so machen
  EEPROM.update(1, hungerstatus);
  EEPROM.update(2, schlafstatus);
  EEPROM.update(3, spassstatus);
  // put ist zum abspeichern von primitiven datentypen und schreibt nur so viele Zellen wie er auch braucht
  // Da ich aber davon ausgehe, dass das geld nicht mehrere 10 Tausend übersteigt sind 2 Byte reserviert
  EEPROM.put(4, geld);

  // wieder ein update da es nur Booleans sind
  for (int i = 0; i < 3; i++) {
    EEPROM.update(6 + i, itemsBesessen[i]);
  }

  display.clearDisplay();
  display.setCursor(10, 20);
  display.print("Gespeichert!");
  display.display();
  delay(1500);
  aktuellerZustand = HAUPTMENUE;
}

void laden() {
  // Prüfen ob ein Spielstand vorhanden ist
  if (EEPROM.read(0) == 1) {  
    // Lesen von einer Zellen
    hungerstatus = EEPROM.read(1);
    schlafstatus = EEPROM.read(2);
    spassstatus = EEPROM.read(3);
    // Lesen von mehrern Zellen und Werten, die mit put reingeschrieben wurden
    EEPROM.get(4, geld);


    for (int i = 0; i < 3; i++) {
      itemsBesessen[i] = EEPROM.read(6 + i);
    }
  } else {
    //wenn kein Speicherstand vorhanden wird alles auf Standard gesetzt
    hungerstatus = 100;
    schlafstatus = 100;
    spassstatus = 100;
    geld = 300;
    for (int i = 0; i < 3; i++) {
      itemsBesessen[i] = false;
    }
  }
}

// Spielstand auf 0 setzen
void loeschen() {
  EEPROM.update(0, 0);
}



void spiel_MusterMerken() {
  static unsigned long letzteAnzeige = 0;
  static int zeigeIndex = 0;
  const unsigned long zeigeDauer = 1000;
  const unsigned long pauseDauer = 400;

  // Anzeigen des Mustern
  if (musterZustand == MUSTER_ANZEIGEN) {
    if (zeigeIndex == 0) {
      // es wird für die Anzahl der zu merkenden muster Random Zahlen generiert
      for (int i = 0; i < musterLaenge; i++) {
        muster[i] = random(0, 3);
      }
      display.clearDisplay();
      display.setCursor(0, 0);
      display.print("Merken...");
      display.display();
      delay(1000);
    }
    
    // Nach jeder Sekunde und einer kurzen Pause von 400 millisekunden werden immer die jeweiligen muster gezeigt
    if (millis() - letzteAnzeige >= zeigeDauer + pauseDauer) {
      // Solange nicht die Musteranzahl erreicht wurde wird etwas angezeigt
      if (zeigeIndex < musterLaenge) {
        display.clearDisplay();  
        display.display();      
        delay(250);             
        
        display.clearDisplay();
        display.setTextSize(2);
        display.setCursor(10, 15);

        // Je nach generierter Zahl wir Up, Middle oder Down angezeigt
        if (muster[zeigeIndex] == 0) display.print("Up");
        else if (muster[zeigeIndex] == 1) display.print("Middle");
        else if (muster[zeigeIndex] == 2) display.print("Down");

        display.display();

        // Aktualisierung des letzten mal Anzeigen
        letzteAnzeige = millis();
        zeigeIndex++;
      } else {
        // Wenn jedes Muster gezeicgt wurde wird alles zurückgesetzt und der Zustand auf die Eingabe gesetzt
        musterZustand = WARTE_AUF_EINGABE;
        zeigeIndex = 0;
        eingabeIndex = 0;

        display.clearDisplay();
        display.setTextSize(1);
        display.setCursor(0, 0);
        display.print("Jetzt eingeben");             
        display.setCursor(0, 40);
        display.print("U / M / D");
        display.display();
        delay(500);
      }
    }
  }

  else if (musterZustand == WARTE_AUF_EINGABE) {
    static int letzterTaster = -1;
    static unsigned long tasterZeit = 0;
    const unsigned long debounceTime = 150;
    static bool tasterLosgelassen = true;

    // Prüfung welcher Taster gedrückt wurde und das Setzen des Tasters auf Losgelassen == false
    int taster = -1;
    if (tasterLosgelassen) {
      if (digitalRead(tasterPinUp) == LOW) {
        taster = 0;
        tasterLosgelassen = false;
      } else if (digitalRead(tasterPinMiddle) == LOW) {
        taster = 1;
        tasterLosgelassen = false;
      } else if (digitalRead(tasterPinDown) == LOW) {
        taster = 2;
        tasterLosgelassen = false;
      }
    }

    // Da die Werte teilweise sonst doppelt aufgenommen wurde wird auch noch geprüft, ob alle Taster losgelassen wurden und damit High sind
    // bevor die nächste Eingabe geschieht
    if (digitalRead(tasterPinUp) == HIGH && digitalRead(tasterPinMiddle) == HIGH && digitalRead(tasterPinDown) == HIGH) {
      tasterLosgelassen = true;
    }

    // Wenn eine Eingabe getätigt wurde und das Tastenprellen beachtet wurde, damit die Eingabe nicht doppelt zählt, kann geprüft werden
    if (taster != -1 && (millis() - tasterZeit > debounceTime)) {
      tasterZeit = millis();

      display.clearDisplay();
      display.setTextSize(2);
      display.setCursor(35, 0);
      display.print(eingabeIndex + 1);

      // Kleine Anzeige damit man sieht, was man eingegeben hat
      display.setTextSize(1);
      display.setCursor(0, 24);
      display.print("U / M / D");

      display.setCursor(0, 38);
      if (taster == 0) display.print("Up");
      else if (taster == 1) display.print("Middle");
      else if (taster == 2) display.print("Down");

      display.display();

      // Prüfung, ob das Richtige eingegeben wurde nacheinander und wenn die letzte Eingabe getätigt wurde 
      //wird die Anzahl erhöht sowie der score und das geld
      if (taster == muster[eingabeIndex]) {
        eingabeIndex++;
        if (eingabeIndex == musterLaenge) {
          musterLaenge++;
          tasterLosgelassen = true;
          score++;
          geld = geld + ( score * 8 );
          musterZustand = MUSTER_ANZEIGEN;
          delay(300);
        }
        // Wenn verloren dann wird das Ergebnis gezeigt
      } else {
        hatVerloren = true;
        musterZustand = ZEIGE_ERGEBNIS;
        delay(300);
      }
    }
  }

  // Anzeigen der Daten mit Wahl, ob man Weiterspielen möchte oder wieder zurück ins Menü will
  else if (musterZustand == ZEIGE_ERGEBNIS) {
    display.clearDisplay();
    display.setCursor(0, 0);
    display.print("Falsch!");

    display.setCursor(0, 12);
    display.print("Score:");
    display.print(score);

    display.setCursor(0, 22);
    display.print("Win:");
    display.print(score * 5);

    display.setCursor(0, 34);
    display.print("^:Neu  v:Menu");
    display.display();

    if (digitalRead(tasterPinUp) == LOW) {
      musterLaenge = 3;
      score = 0;
      hatVerloren = false;
      musterZustand = MUSTER_ANZEIGEN;
      delay(500);
    }

    if (digitalRead(tasterPinDown) == LOW) {
      aktuellerZustand = HAUPTMENUE;
      menu();
      delay(500);
    }
  }
}
