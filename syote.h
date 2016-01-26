/*
syote.h on kirjastontapainen näppäimistösyotteen lukufunktio, jolle voi antaa parametreiksi
ehtoja, jotka käyttäjän antaman syötteen on toteutettava.

Käyttöohje ja lisää dokumentaatiota tiedostossa syotedokumentaatio.pdf

Tuukka Kurtti
*/

//////////////////////////////
//// STANTARDIKIRJASTOT /////
////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <limits.h>
#include <float.h>

///////////////////////////////////////
//// MAKROT JA TYYPPIMÄÄRITELMÄT /////
/////////////////////////////////////

typedef char* string;
#define DATATYPE_int			1
#define DATATYPE_double			2
#define DATATYPE_string			3

#define INPUT_STRING_LENGHT_MAX 60
#define DEFAULT_int INT_MIN
#define DEFAULT_double 0
#define DEFAULT_string NULL

#define syote( datatype, viesti, checkconditionals ) (lue_ ## datatype( viesti, datatype ## checkconditionals ))
#define poke( datatype, ptr, position ) ((datatype*)ptr)[position]

// syote (lueSyote) -funkiton tarkastusparaterimakrot kaikille eri tuetuille tieotyypeille:
#define MITA_TAHANSA NULL, NULL
#define intMITA_TAHANSA MITA_TAHANSA
#define doubleMITA_TAHANSA MITA_TAHANSA
#define stringMITA_TAHANSA MITA_TAHANSA

#define MONIEHTO( ... ) tarkistusLista, (Tarkastaja[]){ __VA_ARGS__, {NULL} }
#define intMONIEHTO( ... ) MONIEHTO( __VA_ARGS__ )
#define doubleMONIEHTO( ... ) MONIEHTO( __VA_ARGS__ )
#define stringMONIEHTO( ... ) MONIEHTO( __VA_ARGS__ )

#define SALLITUT_ARVOT( datatype, ... ) sallitutArvot, (SallitutArvot[]){{ DATATYPE_ ## datatype, VA_NUM_ARGS ( __VA_ARGS__ ), (datatype[]){__VA_ARGS__} }}
#define intSALLITUT_ARVOT( ... ) SALLITUT_ARVOT( int, __VA_ARGS__ )
#define doubleSALLITUT_ARVOT( ... ) SALLITUT_ARVOT( double, __VA_ARGS__ ) 
#define stringSALLITUT_ARVOT( ... ) SALLITUT_ARVOT( string, __VA_ARGS__ ) 

#define intPOSITIIVINEN intVALISSA( 0, INT_MAX )
#define intNEGATIIVINEN intVALISSA( INT_MIN, -1 )
#define intSUUREMPI_KUIN( val ) intVALISSA( val + 1, INT_MAX )
#define intPIENEMPI_KUIN( val ) intVALISSA( INT_MIN, val - 1)
#define intVALISSA( min, max ) intValissa, (int[]){min, max}

#define doublePOSITIIVINEN doubleVALISSA( -DBL_MIN, DBL_MAX )
#define doubleNEGATIIVINEN doubleVALISSA( -DBL_MAX, 0 )
#define doubleSUUREMPI_KUIN( val ) doubleVALISSA( val, DBL_MAX )
#define doublePIENEMPI_KUIN( val ) doubleVALISSA( -DBL_MAX, val )
#define doubleVALISSA( min, max ) doubleValissa, (double[]){min, max}

#define stringPITUUS( min, max ) tarkistaStringPituus, (int[]){min, max}
#define stringPIDEMPI_KUIN( val ) stringPITUUS( val + 1, INT_MAX )
#define stringLYHYEMPI_KUIN( val ) stringPITUUS( INT_MIN, val - 1 )
#define stringSALLITUT_MERKIT( charlist ) stringSallitutMerkit, charlist
#define stringKIELLETYT_MERKIT( charlist ) stringKielletytMerkit, charlist

// VA_NUM_ARGS makrot. Kertoo kuinka monta parametria makrolle on annettu (kahteenkymmeneen asti)
// Lähde: http://efesx.com/2010/07/17/variadic-macro-to-count-number-of-arguments/
#define VA_NUM_ARGS(...) VA_NUM_ARGS_IMPL(__VA_ARGS__, 20,19,18,17,16,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1)
#define VA_NUM_ARGS_IMPL(_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,_17,_18,_19,_20,N,...) N

// Tulostettavien virheilmoitusten koodit makroilla, jotta koodi olisi helpompaa lukea.
#define SUCCESS									0
#define INVISIBLE_ERROR				 			1
#define ERROR_NEGATIVE_NUMBERS_NOT_ALLOWED 		2
#define ERROR_POSITIVE_NUMBERS_NOT_ALLOWED 		3
#define ERROR_INPUT_HAS_TO_BE_GREATER			4
#define ERROR_INPUT_HAS_TO_BE_LESS			 	5
#define ERROR_INPUT_IS_NOT_IN_THE_EXTENT		6
#define ERROR_INPUT_DBL_HAS_TO_BE_GREATER		7
#define ERROR_INPUT_DBL_HAS_TO_BE_LESS			8
#define ERROR_INPUT_DBL_IS_NOT_IN_THE_EXTENT	9
#define ERROR_INPUT_IS_NOT_AN_ALLOWED_VALUE		10
#define ERROR_CANT_UNDERSTAND_INPUT 			11
#define ERROR_CANT_READ_INPUT		 			12
#define ERROR_MALLOC_FAIL			 			13
#define ERROR_STRING_TOO_SHORT			 		14
#define ERROR_STRING_TOO_LONG			 		15
#define ERROR_STRING_HAS_TO_BE_EXACT_LENGHT		16
#define ERROR_ILLEGAL_STRING_LENGHT				17
#define ERROR_FORBIDDEN_CHARS_IN_STRING			18
#define ERROR_NONALLOWED_CHARS_IN_STRING		19
#define ERROR_NO_SPACES_ALLOWED_IN_STRING		20

typedef struct Tarkastaja_{
	int (*tarkastusFunktio)( void*, void* );
	void* parametriData;
} Tarkastaja;

typedef struct SallitutArvot_{
	int tietotyyppi;
	int arvojenMaara;
	void* data;
} SallitutArvot;

///////////////////////////////
//// FUNKTIODEN ESITTELY /////
/////////////////////////////

int 	lue_int	   ( char[], int( void*, void* ), void* );
double 	lue_double ( char[], int( void*, void* ), void* );
string 	lue_string ( char[], int( void*, void* ), void* );

void tulostaIlmoitus( int, void* );
void tulostaMerkkeja( char, int );

int tarkistusLista( void*, void* );
int intValissa( void*, void* );
int doubleValissa( void*, void* );
int tarkistaStringPituus( void*, void* );
int sallitutArvot( void*, void* );
int stringSallitutMerkit( void*, void* );
int stringKielletytMerkit( void*, void* );
