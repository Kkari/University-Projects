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

	// Einf�gen testen
	s1.insert(1);
	s1.insert(1);  // Dupliziertes Element einf�gen: elementNum darf nicht �ndern
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
	Set s2(s1);	// Selbstverst�ndig: der Kpoierkonstruktor wird aktiviert 
	Set s3=s1;	// Das ist keine Zuweisung, sondern Initialisierung, ebenso Kopierkonstruktor!!!
	
	std::cout << " \n lol \n";
	
	printSet(s1);	// Wert�bergabe an der Funktion, Kopierkonstruktor wird aktiviert
	printSet(s2);	// Wert�bergabe an der Funktion, Kopierkonstruktor wird aktiviert
	printSet(&s3);	// �bergabe durch Adresse (Pointer), wird NICHT aktiviert!
	

	s2.remove(1);	// Entfernen ausprobieren
	printSet(s2);	// Entfernt?
	printSet(s1);	// Die andere Menge sollte nicht ver�ndert werden, sonst Kopierkonstruktor schlecht geschrieben
	
	s2.remove(1);	// Entfernen ausprobieren. Im Debugger sollten Sie sehen k�nnen, da� "false" geliefert wird,
					// die Membervariablen werden aber nicht ver�ndert
	return 0;
}

/* Miut�n elosz�r lefuttattuk a tesztet, alak�tsuk �t a 
	void printSet(Set s);
   f�ggv�nyt, hogy ne �rt�k szerint, hanem referencia szerint vegye �t a halmazt!
	void printSet(Set &s);
   Figyelj�nk arra, hogy ezt k�t helyen kell megtenn�nk, a f�ggv�ny defin�ci�j�n�l, �s 
   a protot�pusban is! Ezut�n helyezz�nk el egy t�r�spontot a m�sol� konstruktor elej�n,
   �s futtassuk le a m�sol� konstruktor tesztjeit! Mi a k�l�nbs�g? Mi�rt?
*/