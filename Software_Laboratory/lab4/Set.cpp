#include "stdafx.h"


Set::Set() {
	pData = nullptr;
	elementNum = 0;
}

// Eine Menge aus einer anderen (bereits existierenden) Menge erzeugen (Kopierkonstruktor)
// Die dynamischen Datenelemente müssen auch kopiert werden!
Set::Set(Set &theOther) {
	if (this == &theOther) return;
	std::cout << "copy const activated ";
	elementNum = theOther.elementNum;
	// itt nem kell törölni a pDatát mert ez uj objektumot hoz létre, tehát semmi nincsen mögötte.
	pData = new int[elementNum];
	memcpy(pData, theOther.pData, elementNum*sizeof(int));
}

// Destruktor: die dynamischen Datenelemente freigeben
Set::~Set() {
	delete [] pData;
}

// Ein neues Element zufügen. Wenn die Menge voll ist, soll mit falschem Wert zurückkehren.
// Wenn das Element bereits in der Menge enthalten ist, wird nicht nochmal zugefügt, aber liefert
// die Funktion einen wahren Wert (kein Fehler, das Element ist in der Menge)
bool Set::insert(int element) {
	
	if (elementNum == INT_MAX) return false;
	
	if (isElement(element)) return false;

	int *temp = new int[elementNum + 1];
	memcpy(temp, pData, (elementNum)*sizeof(int));
	temp[elementNum++] = element;
	
	delete [] pData;

	pData = temp;

	return true;
}

// Ein Element aus der Menge entfernen (z. B. Element "5")
// Wenn das Element nicht in der Menge ist, soll ein Fehler signalisiert werden
bool Set::remove(int element) {
	
	for (int i = 0; i < elementNum; i++) {
		if (pData[i] == element) {
			int *temp = new int[elementNum - 1];

			for (int j = 0, k = 0; j < elementNum; j++) {
				if (j != i) {
					temp[k] = pData[j];
					k++;
				}
			}

			delete[] pData;
			pData = temp;
			elementNum--;
			return true;
		}
	}
	return false;
}

// Prüft, ob ein Element in der Menge enthalten ist. Wenn ja, dann true soll geliefert werden. Sonst false.
bool Set::isElement(int element) {
	if (pData == nullptr) return false;
	for (int i = 0; i < elementNum; i++) 
		if (pData[i] == element) return true;
	return false;
}

// Die Menge entleeren
void Set::empty() {
	delete [] pData;
	pData = nullptr;
	elementNum = 0;
}

// Inhalt der Menge (mit Leerstellen getrennt) ausgeben. Nach dem ersten und letzten Element
// ein Zeilenumbruch wird platziert.
void Set::print() {
	for (int i = 0; i < elementNum; i++)
		std::cout << pData[i] << " ";
	std::cout << std::endl;
}