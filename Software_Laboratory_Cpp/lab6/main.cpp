#include <iostream>
#include <string>
#include "student.hpp"

using namespace std;

int main()
{
    Student Hallgato1("Kovacs Peter" , "ABCDEF" , 4.5); // Konstruktor testen
    Student Hallgato2("Torok Tibor"  , "XYZABC" , 3.5); // Konstruktor testen
	Student Hallgato3("Bruce Wayne"  , "BATMEN" , 5.5); // Konstruktor testen
	Student *pHallgato1, *pHallgato2;

	cout<< "Es existieren " << Student::GetInstanzes() << " Studenten!" << endl;
	cout << Hallgato1;
	// Soll "Student: Kovacs Peter, ABCDEF , mit Durchschnitt: 4.5, bekommt: 36000 HUF" auf dem Bildschirm schreiben
	cout << Hallgato2 << Hallgato3; 
		

	pHallgato1 = new Student("Horvath Mark"  , "AAA444" , 7.5); // Neue Student erzeugen auf dem freien Speicher
	cin >> &pHallgato2;											// Neue Student erzuegen durch einlesen
	cout << "Es existieren "<< Student::GetInstanzes() << " Studenten!" << endl;

	Student::SetFormatShort();
	cout << Hallgato1 << Hallgato2 << Hallgato3 << *pHallgato1 << *pHallgato2; // Nur die Name soll ausgeschrieben werden
	delete pHallgato1;
	delete pHallgato2;
	cout<< "Es existieren "<< Student::GetInstanzes() << " Studenten!" << endl;

	cout << "Neptun von Hallgato1 ist valid oder nicht:  "<< Hallgato1.isValidNeptun() << endl;
	cout << "ABCDEF ist valid oder nicht?:" << Student::isValidNeptun("ABCDEF") << endl;

	return 0;
}
