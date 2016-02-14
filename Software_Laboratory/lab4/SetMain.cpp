#include "stdafx.h"

void printSet(Set s);
void printSet(Set *ps);


void printSet(Set s)
{
	s.print();
}


void printSet(Set *ps)
{
	ps->print();
}


int main(void)
{
	Set s1;			// Menge erzeugen

	// Einfügen testen
	s1.insert(1);
	s1.insert(1);  // Dupliziertes Element einfügen: elementNum darf nicht ändern
	s1.insert(2);
	s1.insert(3);
	s1.insert(4);
	s1.insert(5);
	s1.print();

	// Testen von isElement
	if(s1.isElement(1)&&s1.isElement(2)&&!s1.isElement(3))
	{
		printf("isElement seems OK.\n");
	}

	// !!! Hier sollte am Anfang des Kopierkonstruktors ein Breakpoint gesetzt werden,
	// damit wir sehen, wann er aufgerufen wird !!!
	
	// Kopierkonstruktor
	Set s2(s1);	// Selbstverständig: der Kpoierkonstruktor wird aktiviert 
	Set s3=s1;	// Das ist keine Zuweisung, sondern Initialisierung, ebenso Kopierkonstruktor!!!
	
	std::cout << " \n lol \n";
	
	printSet(s1);	// Wertübergabe an der Funktion, Kopierkonstruktor wird aktiviert
	printSet(s2);	// Wertübergabe an der Funktion, Kopierkonstruktor wird aktiviert
	printSet(&s3);	// Übergabe durch Adresse (Pointer), wird NICHT aktiviert!
	

	s2.remove(1);	// Entfernen ausprobieren
	printSet(s2);	// Entfernt?
	printSet(s1);	// Die andere Menge sollte nicht verändert werden, sonst Kopierkonstruktor schlecht geschrieben
	
	s2.remove(1);	// Entfernen ausprobieren. Im Debugger sollten Sie sehen können, daß "false" geliefert wird,
					// die Membervariablen werden aber nicht verändert
	return 0;
}

/* Miután eloször lefuttattuk a tesztet, alakítsuk át a 
	void printSet(Set s);
   függvényt, hogy ne érték szerint, hanem referencia szerint vegye át a halmazt!
	void printSet(Set &s);
   Figyeljünk arra, hogy ezt két helyen kell megtennünk, a függvény definíciójánál, és 
   a prototípusban is! Ezután helyezzünk el egy töréspontot a másoló konstruktor elején,
   és futtassuk le a másoló konstruktor tesztjeit! Mi a különbség? Miért?
*/