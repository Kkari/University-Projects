#ifndef STRING_H
#define STRING_H

#include <iostream> // F�r cout, <iosfwd> waere �brigens genug hier
#include <cstring>
using namespace std;

class String
{
	// Der Pointer auf dem dynamischen Speicherbereich wo die Zeichen sind
	char *pData;
	unsigned int elementsNum;
public:
	// Konstruktoren:
	// Parameterloser Konstruktor
	String();
	// Kopierkonstruktor
	String(const String &theOther);
	// Konstruktor mit nullterminierten C-Zeichenkette
	String(const char *str);
	// Konstruktor mit einem Zeichen und mit einem unsigned int (times)
	// unser String wird mit "times" St�ck von dem Zeichen "c" initialisiert
	String(const char c, unsigned int times);
		
	// Destruktor:
	~String() {if(pData) delete [] pData;}
		
	// Operatoren �berladen
	const String& operator=(const String & theOther);
	bool operator==(const String &theOther);
	bool operator!=(const String &theOther);
	String operator+(String & theOther);
	String operator+(char c);
	
	char operator[](int i);
			
	// Weitere funktionen:
	// Inhalt des String-Objektes in den Puffer pBuff kopieren und mit Nullzeichen terminieren
	// (Sie sollen annehmen, da� der Puffer wird von dem Anwender allokiert und freigegeben)
	void getStr(char *pBuff);
	// L�nge des Strings ermitteln
	unsigned int getLength();

	// F�r auschreiben brauchen wir ein globale Funktion
	friend ostream & operator<<(ostream &, String & S);
	friend String operator+(const char c, String S);
};

#endif /* STRING_H */