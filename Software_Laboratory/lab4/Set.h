#ifndef SET_H
#define SET_H

// Vereinbarung: wo bool als Ergebnis geliefert wird, bedeute "false" einen Fehler

// Set: Datenstruktur "Menge" mit ganzzahligen Elemente. 
class Set
{
	int elementNum;
	int *pData;
public:
	// Eine leere Menge (Set) erzeugen
	Set();

	// Eine Menge aus einer anderen (bereits existierenden) Menge erzeugen (Kopierkonstruktor)
	// Die dynamischen Datenelemente müssen auch kopiert werden!
	Set(Set &theOther);

	// Destruktor: die dynamischen Datenelemente freigeben
	~Set();

	// Ein neues Element zufügen. Wenn die Menge voll ist, soll mit falschem Wert zurückkehren.
	// Wenn das Element bereits in der Menge enthalten ist, wird nicht nochmal zugefügt, aber liefert
	// die Funktion einen wahren Wert (kein Fehler, das Element ist in der Menge)
	bool insert(int element);

	// Ein Element aus der Menge entfernen (z. B. Element "5")
	// Wenn das Element nicht in der Menge ist, soll ein Fehler signalisiert werden
	bool remove(int element);

	// Prüft, ob ein Element in der Menge enthalten ist. Wenn ja, dann true soll geliefert werden. Sonst false.
	bool isElement(int element);

	// Die Menge entleeren
	void empty();

	// Inhalt der Menge (mit Leerstellen getrennt) ausgeben. Nach dem ersten und letzten Element
	// ein Zeilenumbruch wird platziert.
	void print();
};
#endif /* SET_H */
