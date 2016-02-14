#include <iostream>
#include "String.h"

using namespace std;

int main()
{
	// den parameterlosen Konstruktor testen
	String s1;
	
	//den Konstruktor testen, der einen String erwartet
	String s2 = "Hello!";

	String s3 = s2; // Das ist ein Kopierkonstruktor: s3(s2)

	// den "Muster" Konstruktor testen
	String s4('-', 15);

	// die Ergebnisse ausschreiben
	cout << "s1: "<< s1 << endl;
	cout << "s2: "<< s2 << endl;
	cout << "s3: "<< s3 << endl;
	cout << "s4: "<< s4 << endl;

	//den Zuweisungsoperator testen
	s1 = s2;
	s1 = s1; // Das soll nichts machen!

	cout << "s1: "<< s1 << endl;
	cout << "s2: "<< s2 << endl;
	cout << "s3: "<< s3 << endl;
	
	//den Vergleichsoperatoren testen
	if (s1 == s2)
		cout << "s1 and s2 are the same!!!" << endl;
	
	if (s1 != s4)	
		cout << "s1 and s4 are not the same!!!" << endl;

	//den Additionsoperator (Konkatenation) testen
	String s5 = s2 + s4;
	cout << "s5=s2+s4:" << s5 << endl;

	//den Additionsoperator mit char + string (Konkatenation) testen
	String s6 = '!' + s4;
	cout << "s6:'!'+s4:" << s6 << endl;

	//den Additionsoperator mit string + char (Konkatenation) testen
	s6 = s6 + '!';
	cout << "s6 = s6 + '!':" << s6 << endl;
	//den Indexoperator testen
	for (unsigned int i = 0; i < s5.getLength(); i++)
	{
		cout << s5[i];
	}
	cout << endl;

	//die getStr Funktion testen
	char* pStr = new char[s4.getLength() + 1]; // Hely a sztringnek és a lezáró nullának
	s4.getStr(pStr);
	cout << pStr << endl;	// A cout ismeri a sztringet is...
	delete[] pStr;		// Felszabadítjuk a buffert.
	
return 0;
}