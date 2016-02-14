#include <iostream>
#include "String.h"

using namespace std;

String::String() 
{
	pData = NULL;
	elementsNum = 0;
};

String::String(const String &theOther)
{
	pData = new char[theOther.elementsNum+1];
	for(unsigned int i=0 ; i < theOther.elementsNum+1 ; i++) pData[i] = theOther.pData[i];
	elementsNum = theOther.elementsNum;
}

String::String(const char *str)
{
	pData = new char[strlen(str)+1];
	for(unsigned int i=0 ; i<strlen(str) ; i++)	pData[i] = str[i];
	pData[strlen(str)] = '\0';
	elementsNum  = (unsigned int) strlen(str);
}

String::String(const char c, unsigned int times)
{
	pData = new char[times+1];
	for(unsigned int i=0 ; i<times ; i++)	pData[i] = c;
	
	pData[times] = '\0';
	elementsNum = times+1;
}

const String& String::operator=(const String & theOther)
{
	if (*this==theOther) return theOther;
	if (pData!=NULL) delete [] pData;
	pData = new char[theOther.elementsNum+1];
	for(unsigned  int i=0 ; i<theOther.elementsNum ; i++)	pData[i] = theOther.pData[i];
	pData[theOther.elementsNum] = '\0';
	elementsNum = theOther.elementsNum;
	return *this;
}

bool String::operator==(const String &theOther)
{
	if (theOther.elementsNum != elementsNum) return false;
	for (unsigned int i=0 ; i<getLength() ; i++) if(pData[i] != theOther.pData[i]) return false;
	return true;
}

bool String::operator!=(const String &theOther)
{
	return !(*this==(theOther));
}

String String::operator+(String & theOther)
{
	unsigned newsize = getLength() + theOther.getLength()+1;
	char *tmp = new char[newsize];
	
	unsigned int i=0;
	for (i=0 ; i < getLength() ; i++)	tmp[i] = pData[i];
	for (unsigned int j=0 ; j < theOther.getLength()+1; i++ , j++) tmp[i] = theOther.pData[j];
	

	String s(tmp);
	delete [] tmp;
	return s;
}

char String::operator[](int i)
{
return pData[i];
}
		
void String::getStr(char *pStr)
{
	for (unsigned int i=0 ; i<elementsNum+1 ; i++) pStr[i] = pData[i];
	return;
}

unsigned int String::getLength() 
{ 
	return elementsNum;
}


String String::operator+(char c)
{
	unsigned newsize = getLength() + 2;
	char *tmp = new char[newsize];
	
	for (unsigned int i=0 ; i < getLength() ; i++)	tmp[i] = pData[i];
	tmp[newsize-2]=c;
	tmp[newsize-1]='\0';
	

	String s(tmp);
	delete [] tmp;
	return s;


}


ostream & operator<<(ostream &os, String & S)
{
	for (unsigned int i=0; i<S.getLength();i++) os<<S.pData[i];
	return os;
}

String operator+(const char c, String S)
{
	char *tmp = new char[S.getLength()+2];
	tmp[0]=c;
	for(unsigned int i=0; i<S.getLength()+1;i++)
	tmp[i] = S.pData[i];
	String Stmp(tmp);
	delete [] tmp;
	return Stmp;
}