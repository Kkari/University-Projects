#include <math.h>
#include <stdio.h>
#include <stdlib.h>

// Element des Baumes
typedef struct Messung{
    double Data;
    int Anzahl;
    struct Messung *Right, *Left;
} Knote;

// Funktiondeklarationen
// LaborAufgaben
int Baum_Add(Knote **Head, double Data);
void Baum_Print(Knote *Head);

double Baum_Min(Knote *Head);
double Baum_Max(Knote *Head);
void Baum_Delete(Knote **Head);

// Hausaufgabe 1
int Baum_Find(Knote *Head, double Data);
double Baum_Avg(Knote *Head);

// Hausaufgabe 2
int Baum_isBalanced(Knote *Head);
double *Baum_2Array(Knote *Head);


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

int main()
{
Knote *Head = NULL;
double DataMin, DataMax;
double Gesucht = 3.0;
double *array;
int i;

#ifdef Laboraufgabe

    Baum_Print(Head);
    puts("------------------------");
	puts("Baum wird Aufgebaut!");

	Baum_Add(&Head, 2.0);
	Baum_Add(&Head, 5.8);
	Baum_Add(&Head, 3.2);
	Baum_Add(&Head, 2.0);
	Baum_Add(&Head, 7.0);

	puts("Elemente im Baum:");
	Baum_Print(Head);
	puts("------------------------");

	DataMin = Baum_Min(Head);
	DataMax = Baum_Max(Head);

	printf("Kleinste Wert im Baum: %lf\n", DataMin);
	printf("Grosste Wert im Baum: %lf\n", DataMax);

#endif

#ifdef Hausaufgabe1
	puts("------------------------");
	printf("Gesuchte element: %lf, Anzahl = %d\n", Gesucht, Baum_Find(Head, Gesucht));
	printf("Gesuchte element: %lf, Anzahl = %d\n", DataMin, Baum_Find(Head, DataMin));

	puts("------------------------");
	printf("Durchschnitt der Messwerten: %lf\n", Baum_Avg(Head));
#endif


#ifdef Hausaufgabe2
	puts("------------------------");
	if(Baum_isBalanced(Head)) puts("Baum ist Balanziert!");
	else puts("Baum ist nicht Balanziert!");

	puts("------------------------");
	array = Baum_2Array(Head);
	for (i = 1; i<=array[0] ; i++) printf("%lf\t" , array[i]);
	puts("");    
#endif

#ifdef Laboraufgabe
    puts("------------------------");
    puts("Baum wird geloescht!");
    Baum_Delete(&Head);
    printf("Wert von Zeiger Head: %p\n", Head);
    Baum_Print(Head);
#endif

return 0;
}

int Baum_Add(Knote **Head, double Data)
{
    Knote *p = *Head;

    if (*Head == NULL){
        *Head = (Knote*) malloc(sizeof(Knote));
        (*Head)->Data = Data;
        (*Head)->Anzahl = 1;
    	(*Head)->Left = (*Head)->Right = NULL;
    	return 1;
    }
    else{
    	if (p->Data > Data) Baum_Add(&(p->Right), Data);
    	else if (p->Data < Data) Baum_Add(&(p->Left), Data);
    	else if (p->Data == Data)
        {
        (p->Anzahl)++; 
        return 0;
        }
    }
}

void Baum_Print(Knote *Head)
{
	if (Head == NULL){ 
		printf("Baum ist Leer!\n"); 
		return;
	}

	if(Head->Right)	Baum_Print(Head->Right);
	printf("Data: %lf, Anzahl: %d\n", Head->Data, Head->Anzahl);
	if(Head->Left)	Baum_Print(Head->Left);
    
	return;
}

double Baum_Min(Knote *Head)
{
    while (Head->Right != NULL) Head = Head->Right;
    return Head->Data;
}

double Baum_Max(Knote *Head)
{
    while (Head->Left != NULL) Head = Head->Left;
    return Head->Data;
}

void Baum_Delete(Knote **Head)
{
    if ((*Head)->Right)	Baum_Delete(&((*Head)->Right));
    if ((*Head)->Left)	Baum_Delete(&((*Head)->Left));
    free(*Head);
    *Head = NULL;
    return;
}

int Baum_Find(Knote *Head, double Data)
{
    if (Head == NULL) return 0;
    if (Head->Data == Data) return Head->Anzahl;
    else if (Head->Data > Data) return Baum_Find(Head->Right, Data);
    else if (Head->Data < Data) return Baum_Find(Head->Left, Data);   
}

double Baum_Avg(Knote *Head)
{
    static double sum = 0;
    static int num = 0;
    
    if(Head == NULL) return;
    if(Head->Right) Baum_Avg(Head->Right);
    if(Head->Left)  Baum_Avg(Head->Left);
    
    sum += Head->Data * Head->Anzahl;
    num += Head->Anzahl;
    return sum / num;
}

int Baum_isBalanced(Knote *Head)
{
   int lh; /* for height of left subtree */
   int rh; /* for height of right subtree */ 
 
   /* If tree is empty then return true */
   if(Head == NULL) return 1;
 
   /* Get the height of left and right sub trees */
   lh = Baum_Hohe(Head->Left);
   rh = Baum_Hohe(Head->Right);
 
   if(  abs(lh-rh) <= 1 &&  
        Baum_isBalanced(Head->Left) &&
        Baum_isBalanced(Head->Right))
    return 1;
 
   /* If we reach here then tree is not height-balanced */
   return 0;
}

/*  The function Compute the "height" of a tree. Height is the
    number of nodes along the longest path from the root node
    down to the farthest leaf node.*/
int Baum_Hohe(Knote* Head)
{
   /* base case tree is empty */
   if(Head == NULL) return 0;
 
	/* If tree is not empty then height = 1 + max of left
      height and right heights */
	if (Baum_Hohe(Head->Left) > Baum_Hohe(Head->Right)) return 1 + Baum_Hohe(Head->Left);
	else	return 1 + Baum_Hohe(Head->Right);
}

double* Baum_2Array(Knote *Head)
{
    static double *arr = NULL;
    static int size = 0;
    
	if (Head == NULL) return;
    if (Head->Left)		Baum_2Array(Head->Left);
    if (Head->Right)	Baum_2Array(Head->Right);
    
    size++;
    arr = (double *) realloc(arr , (size+1) * sizeof(double));
    arr[0] = size;
    arr[size] = Head->Data;
    return arr;
}