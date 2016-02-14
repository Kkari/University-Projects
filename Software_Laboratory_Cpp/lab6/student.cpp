#include<iostream>
#include<string>
#include "student.hpp"
using namespace std;

int Student::IstanzCnt = 0;
enum Format Student::Typ = FULL;
const double Student::GeldFur5 = 40000;

bool Student::isValidNeptun(const string str)
{
	if (str.length()!=6) return false;
	for (unsigned int i=0; i<str.length();i++)
		if (!isalpha(str[i]) && !isdigit(str[i])) return false;
	return true;
}


bool Student:: isValidNeptun() //Soll entscheiden ob das Neptunkode gültig ist 
{
	return isValidNeptun(Neptun);
}

ostream & operator<<(ostream &os, Student & S)
{
	switch(Student::Typ)
	{
	case FULL:
		os << "Student:" << S.Name << ',' << S.Neptun << ", mit Durchschnitt: "<< S.Durchschnitt <<", bekommt: ";
		os << S.Durchschnitt/5 * S.GeldFur5 <<" HUF" << endl;
		break;
	case SHORT:
		cout << "Student:" << S.Name << endl;
		break;
	}
return os;
}

istream & operator>>(istream &is, Student **S)
{
	char Name[40], Neptun[10];
	double Durchschnitt;
	cout<< "Neue Student einlesen!" << endl;

	cout<< "Name:";	
	is.getline(Name,40);
	
	cout<< "Neptun:"; 
	is >> Neptun;
	
	cout<< "Durchschnitt:";
	is >> Durchschnitt;
	*S = new Student(Name, Neptun, Durchschnitt);
	return is;
}