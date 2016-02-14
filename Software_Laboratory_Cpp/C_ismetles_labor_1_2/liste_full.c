#include <stdio.h>
#include <string.h>
#include <stdlib.h>

typedef struct Daten {
    char *Name;
    unsigned long Nummer;
    struct Daten *next;
} ListElem;

ListElem* TelefonBuch_Add(ListElem *Head, const char *Name, long Tnummer);
void TelefonBuch_Print(ListElem *Head);
ListElem* TelefonBuch_Delete(ListElem *Head);

// Hausaufgabe 
long TelefonBuch_Find(ListElem *Head, const char *Name);
ListElem* TelefonBuch_Loeschen(ListElem *Head, const char *Name);

// Hausaufgabe ++
ListElem* TelefonBuch_Sortieren(ListElem *Head);
ListElem* TelefonBuch_Umkehren(ListElem *Head);
ListElem* TelefonBuch_Split(ListElem *Head);


int main()
{
    ListElem *Head;
    ListElem *HeadNeu;
    long Tnum;
    char NameSuche1[] = "Vajta Laszlo";
    char NameSuche2[] = "Kovacs Laszlo";
    
    TelefonBuch_Print(Head);
    
    puts("------------------------------------");    
    Head = TelefonBuch_Add(Head, "Sujbert Laszlo", 4633213);
    Head = TelefonBuch_Add(Head, "Nagy Lajos", 4632225);
    Head = TelefonBuch_Add(Head, "Kovacs Peter", 2568275);
    Head = TelefonBuch_Add(Head, "Vajta Laszlo", 1223213);
    
    puts("Inhalt des Telefonbuches:");    
    TelefonBuch_Print(Head);
    
    puts("------------------------------------");    
    Head = TelefonBuch_Umkehren(Head);
    puts("Inhalt des umgekehrten Telefonbuches:");    
    TelefonBuch_Print(Head);
    
    puts("------------------------------------");    
    if(Tnum = TelefonBuch_Find(Head, NameSuche1))
    {
        printf("%s Gefunden & Geloscht !\n",NameSuche1);    
        Head = TelefonBuch_Loeschen(Head,NameSuche1);    
        printf("Telefonnummer = %lu\n",Tnum);

    }
    else printf("%s, Nicht gefunden!\n",NameSuche1);
    
    puts("Inhalt des Telefonbuches:");    
    TelefonBuch_Print(Head);
    
    puts("------------------------------------");    
    if(Tnum = TelefonBuch_Find(Head, NameSuche2))
    {
        printf("%s Gefunden & Geloscht !\n",NameSuche2);    
        Head = TelefonBuch_Loeschen(Head,NameSuche2);    
        printf("Telefonnummer = %lu\n",Tnum);
    }
    else printf("%s nicht gefunden!\n",NameSuche2);
    
    puts("Inhalt des Telefonbuches:");    
    TelefonBuch_Print(Head);
    
    
    puts("------------------------------------");    
    TelefonBuch_Sortieren(Head);
    puts("Inhalt des sortierten Telefonbuches:");    
    TelefonBuch_Print(Head);
     
    puts("------------------------------------");    
    HeadNeu = TelefonBuch_Split(Head);
    puts("Inhalt des erste Telefonbuches:");    
    TelefonBuch_Print(Head);
    puts(" ");
    puts("Inhalt des zweite Telefonbuches:");    
    TelefonBuch_Print(HeadNeu);
    
    puts("------------------------------------");   
    Head = TelefonBuch_Delete(Head);
    HeadNeu = TelefonBuch_Delete(HeadNeu);
    puts("Liste geloescht!");

return 0;
}

ListElem* TelefonBuch_Add(ListElem *Head, const char *Name, long Nummer)
{
    ListElem *p = Head;
    
    if (Head == NULL)
    {
        Head = (ListElem *) malloc(sizeof(ListElem));
        Head->Name = (char *) malloc(strlen(Name));
        Head->Nummer = Nummer;
        strcpy(Head->Name , Name);
    return Head;
    }
    
    while(p->next != NULL) p = p->next;
    
    p->next = (ListElem *) malloc(sizeof(ListElem));
    (p->next)->Name = (char *) malloc(strlen(Name));
	strcpy((p->next)->Name,Name);
	
	(p->next)->Nummer = Nummer;
    (p->next)->next = NULL;
    
    return Head;
}

void TelefonBuch_Print(ListElem *Head)
{
    ListElem *p;
    
    if (Head == NULL){
		printf("TelefonBuch ist lehr!\n");
		return; 
    }
    else{
        p = Head;
        while(p != NULL) 
        {
            printf("%s, %lu\n", p->Name, p->Nummer);
            p = p->next;
        }
    }
    return;
}

ListElem* TelefonBuch_Delete(ListElem *Head)
{
    ListElem *tmp;
    
    if (Head == NULL) return NULL;
    else{
        do{
            tmp = Head;    
            free(tmp->Name);
            free(tmp);
            Head = Head->next;
        } while(tmp->next!=NULL);
    return NULL;
    }
}

long TelefonBuch_Find(ListElem *Head, const char *Name)
{
    while(Head!=NULL){
        if (!strcmp(Head->Name, Name)) return Head->Nummer;
        Head = Head->next;
    }

return 0;
}

ListElem * TelefonBuch_Loeschen(ListElem *Head, const char *Name)
{
    ListElem *tmp = Head->next;
    ListElem *prev = Head;

    if (!strcmp(Head->Name,Name)){
        free(Head);
        return(tmp);
    }
    else{
        while(tmp!=NULL)
        {
            if (!strcmp(tmp->Name, Name))     
            {
                prev->next = tmp->next;
                free(tmp);
                return Head;
            }
            prev = tmp;
            tmp=tmp->next;
        }
        
        return Head;
    }
}

ListElem* TelefonBuch_Sortieren(ListElem *Head)
{
    ListElem *tmp;
    ListElem *tmp2;
    ListElem *prev; 
    char *tmpname;
    long tmpnum;
    if (Head->next==NULL) return Head;   
    
    else
    {
        prev = Head;
        tmp = Head->next;
        tmp2 = Head;
        while(tmp2!=NULL)
        {
            prev = Head;
            tmp = prev->next;
         
            while (tmp != NULL)    
            {
                if (strcmp(prev->Name , tmp->Name)>0)
                {
                    tmpnum = tmp->Nummer;
                    tmp->Nummer = prev->Nummer;
                    prev->Nummer = tmpnum;
                    
                    tmpname = tmp->Name;
                    tmp->Name = prev->Name;
                    prev->Name = tmpname;
                }
                
                prev = tmp;
                tmp = tmp->next;
            }
            tmp2 = tmp2->next;
        }
    
    return Head;
    }
}
ListElem* TelefonBuch_Umkehren(ListElem *Head)
{
    ListElem *prev;
    ListElem *curr;
    ListElem *next;
    if (Head == NULL || Head->next == NULL) return Head;
    if ((Head->next)->next == NULL) 
    {
        next = Head->next;
        next->next = Head;
        Head->next = NULL;
        return next;
    }
    else{
        prev = Head;
        curr = Head->next;
        next = curr->next;
        
        prev->next = NULL;
        curr->next = prev;
        
        while (next != 0)
        {
            prev = curr;
            curr = next;
            next = curr->next;
            curr->next = prev;
        }
        return curr;
    }
}
ListElem* TelefonBuch_Split(ListElem *Head)
{
    ListElem *p;
    ListElem *Head_neu;
    int anzahl = 0;
    
    if (Head == NULL || Head->next == NULL) return 0;
    else{
        for(p = Head ; p != NULL ; p = p->next) anzahl++;
    
        anzahl = anzahl/2 + anzahl%2;
        p = Head;
        for(p = Head ; anzahl>1 ; p = p->next, anzahl--); 
        Head_neu = p->next;
        p->next = NULL;
    
    return Head_neu;
    }
}

