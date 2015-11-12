/*
syote.c on kirjastontapainen näppäimistösyotteen lukufunktio
Ei tässä versiossa käytä headeria, vaan koodi on liitettävä kokonaisuutenaan osaksi ohjelman lähdekoodia.

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

////////////////////////////////
//// SYÖTTEENLUKUFUNKTIOT /////
//////////////////////////////

/* FUNKTIOT lue_int, lue_double ja lue_string ( char[], int ( void*, void*), void* )
Lukee käyttäjältä kokonaisluvun/desimaaliluvun/merkkijonon, joka toteuttaa tarkistusfunktion toteuttaman ehdon.
Syötettä kysytään niin kauan, että kelvollinen vastaus saadaan. Vääränlaisista syötteistä annetaan virheilmoitus.
Jos luetaan merkkijono, varataan sille muistia dynaamisesti, joka pitää muistaa vapauttaa käytön jälkeen.
Funktioita on tarkoitus kutsua makron 'syote' kautta.

Syötteet:
	viesti		= käyttäjälle tulostettvava viestimerkkijono.
	tarkistus	= funktio, jolla syote tarkastetaan.
	checkData	= tarkistusfunktion ottamat parametrit.
	
Palautusarvo:
	Palauttaa käyttäjän syöttämän kokonaisluvun/desimaaliluvun tai osoittimen käyttäjän syöttämän merkkijonon alkuun.
*/
int lue_int( char viesti[], int tarkistus( void*, void* ), void* checkData ) {
	
	char syote[INPUT_STRING_LENGHT_MAX];
	int returnValue = DEFAULT_int;
	int error;

	do {
		error = 0;
		printf("%s", viesti);
		fflush( stdin );

		if (fgets( syote, INPUT_STRING_LENGHT_MAX, stdin ) == NULL) {
			tulostaIlmoitus(error = ERROR_CANT_UNDERSTAND_INPUT, NULL);
		} else {
			if ( ( sscanf( syote, "%d", &returnValue) ) != 1 ){
				tulostaIlmoitus(error = ERROR_CANT_UNDERSTAND_INPUT, NULL);
			} else if ( tarkistus != NULL){
				if( ( error = tarkistus( &returnValue, checkData )) != 0){
					tulostaIlmoitus(error, checkData);
				}
			}
		}
	} while (error);
	
	fflush( stdin );
	return returnValue;
}

double lue_double( char viesti[], int tarkistus( void*, void* ), void* checkData ) {
	
	char syote[INPUT_STRING_LENGHT_MAX];
	double returnValue = DEFAULT_double;
	int error;

	do {
		error = 0;
		printf("%s", viesti);
		fflush( stdin );

		if (fgets( syote, INPUT_STRING_LENGHT_MAX, stdin ) == NULL) {
			tulostaIlmoitus(error = ERROR_CANT_UNDERSTAND_INPUT, NULL);
		} else {
			if ( ( sscanf( syote, "%lf", &returnValue) ) != 1 ){
				tulostaIlmoitus(error = ERROR_CANT_UNDERSTAND_INPUT, NULL);
			} else if ( tarkistus != NULL){
				if( ( error = tarkistus( &returnValue, checkData )) != 0){
					tulostaIlmoitus(error, checkData);
				}
			}
		}
	} while (error);
	
	fflush( stdin );
	return returnValue;
}

string lue_string( char viesti[], int tarkistus( void*, void* ), void* checkData ) {
	
	char syote[INPUT_STRING_LENGHT_MAX];
	string returnValue;
	int pituus;
	int error;

	do {
		error = 0;
		printf("%s", viesti);
		fflush( stdin );

		if (fgets( syote, INPUT_STRING_LENGHT_MAX, stdin ) == NULL) {
			tulostaIlmoitus(error = ERROR_CANT_UNDERSTAND_INPUT, NULL);
		} else {
			pituus = strlen(syote);
			if ( ( returnValue = malloc( pituus * sizeof( char ) ) ) == NULL ){
				tulostaIlmoitus(error = ERROR_MALLOC_FAIL, NULL);
			} else {
				strncpy(returnValue, syote, pituus - 1);
				returnValue[pituus - 1] = '\0';
				if ( tarkistus != NULL){
					if( ( error = tarkistus( returnValue, checkData )) != 0){
						tulostaIlmoitus(error, checkData);
					}
				}
			}
		}
	} while (error);
	
	fflush( stdin );
	return returnValue;
}

////////////////////////////
//// TULOSTUSFUNKTIOT /////
//////////////////////////

/* FUNKTIO tulostaIlmoitus( int, void* )
Tulostaa viestin käyttäjälle.
Suurin osa käyttöliittymän teksteistä löytyy tästä funktiosta, mikä tekee käyttöliittymän muokkaamisesta helpompaa 
esimerkiksi silloin, jos sen haluaa kääntää muulle kielelle, kuten vaikka englanniksi.

Syötteet:
	id	 = tulostettavan viestin koodi.
	data = osoitin muistiin, mihin tarvittavat lisäparametrit on tallennettu.
*/
void tulostaIlmoitus( int id, void* data ) {
	
	switch( id ){
		case ERROR_NEGATIVE_NUMBERS_NOT_ALLOWED:
			printf("Syotteen on oltava positiivinen.\n");
			break;
		case ERROR_POSITIVE_NUMBERS_NOT_ALLOWED:
			printf("Syotteen on oltava negatiivinen.\n");
			break;
		case ERROR_INPUT_HAS_TO_BE_GREATER:
			printf("Syotearvon on oltava vahintaan %d.\n", poke(int, data, 0));
			break;
		case ERROR_INPUT_HAS_TO_BE_LESS:
			printf("Syotearvo voi olla korkeintaan %d.\n", poke(int, data, 1));
			break;	
		case ERROR_INPUT_IS_NOT_IN_THE_EXTENT:
			printf("Syotearvon on oltava valissa %d...%d\n", poke(int, data, 0), poke(int, data, 1));
			break;
		case ERROR_INPUT_DBL_HAS_TO_BE_GREATER:
			printf("Syotearvon on oltava suurempi kuin %.4f.\n", poke(double, data, 0));
			break;
		case ERROR_INPUT_DBL_HAS_TO_BE_LESS:
			printf("Syotearvon on oltava pienempi kuin %.4f.\n", poke(double, data, 1));
			break;	
		case ERROR_INPUT_DBL_IS_NOT_IN_THE_EXTENT:
			printf("Syotearvon on oltava valissa [%.2f, %.2f]\n", poke(double, data, 0), poke(double, data, 1));
			break;
		case ERROR_INPUT_IS_NOT_AN_ALLOWED_VALUE:
			printf("Syota jokin seuraavista arvoista:");	
			for (int i = 0; i < poke(SallitutArvot, data, 0).arvojenMaara; i++) {
				if (poke(SallitutArvot, data, 0).tietotyyppi == DATATYPE_int) 	 printf(" %d", poke(int	  , poke(SallitutArvot, data, 0).data, i));
				if (poke(SallitutArvot, data, 0).tietotyyppi == DATATYPE_double) printf(" %f", poke(double, poke(SallitutArvot, data, 0).data, i));
				if (poke(SallitutArvot, data, 0).tietotyyppi == DATATYPE_string) printf(" %s", poke(string, poke(SallitutArvot, data, 0).data, i));
				if ( i < poke(SallitutArvot, data, 0).arvojenMaara - 1) printf(",");
			}
			printf("\n");
			break;
		case ERROR_STRING_TOO_SHORT:
			printf("Syotteen pituuden on oltava vahintaan %d merkkia.\n", poke(int, data, 0));
			break;
		case ERROR_STRING_TOO_LONG:
			printf("Syotteen pituus voi olla korkeintaan %d merkkia.\n", poke(int, data, 1));
			break;
		case ERROR_STRING_HAS_TO_BE_EXACT_LENGHT:
			printf("Syotteen on oltava %d merkkia pitka.\n", poke(int, data, 0));
			break;
		case ERROR_ILLEGAL_STRING_LENGHT:
			printf("Syotteen on oltava %d - %d merkkia pitka.\n", poke(int, data, 0), poke(int, data, 1));
			break;
		case ERROR_FORBIDDEN_CHARS_IN_STRING:
			printf("Syotteessa ei saa olla seuraavia merkkeja:\n");
			for (int i = 0; poke(char, data, i) != '\0'; i++){
				if (i != 0) printf(" ");
				printf("[%c]", poke(char, data, i));
			}
			printf("\n");
			break;
		case ERROR_NONALLOWED_CHARS_IN_STRING:
			printf("Ainoastaan seuraavat merkit ovat sallittuja syotteessa:\n");
			for (int i = 0; poke(char, data, i) != '\0'; i++){
				if (i != 0) printf(" ");
				printf("[%c]", poke(char, data, i));
			}
			printf("\n");
			break;
		case ERROR_NO_SPACES_ALLOWED_IN_STRING:
			printf("Valilyonnit eivat ole sallittuja syotteessa.\n");
			break;	
		case ERROR_CANT_UNDERSTAND_INPUT:
			printf("Virheellinen syote.\n");
			break;
		case ERROR_CANT_READ_INPUT:
			printf("Virhe syotteen lukemisessa.\n");
			break;
		case ERROR_MALLOC_FAIL:
			printf("Muistinkäyttövirhe.\n");
			break;
		case INVISIBLE_ERROR:
			break;
		default:
			printf("Tuntematon virhe: %d\n", id);
	}
	if (id >= 2) printf("\n");
}

/* FUNKTIO tulostaMerkkeja( char merkki, int maara )
Tulostaa maara kappeletta merkkia.
*/
void tulostaMerkkeja( char merkki, int maara ){
	for (int i = 0; i < maara; i++) {
		printf("%c", merkki);
	}
}

/////////////////////////////////////
//// SYÖTTEENTARKISTUSFUNKTIOT /////
///////////////////////////////////

/* FUNKTIO intValissa ja doubleValissa( void*, void* )
Tarkistaa, onko kokonaisluku\desimaaliluku kahden luvun rajamaalla välillä

Syötteet:
	arvo	= tarkastettava arvo.
	limits	= kahden kokonaisluvun taulukko, joka kertoo lukualueen rajat. 

Palautusarvo:
	Palauttaa nollan, jos virheitä ei löytynyt. Muussa tapauksessa palautetaan virhekoodi.
*/
int intValissa( void* arvoPtr, void* limits ){

	int arvo = poke(int, arvoPtr, 0);
	int min  = poke(int, limits, 0);
	int max  = poke(int, limits, 1);
	
	if (arvo >= min && arvo <= max ){
		return SUCCESS;
	} else {
		if (arvo > max){
			if(min == INT_MIN){
				if (max == -1) {
					return ERROR_POSITIVE_NUMBERS_NOT_ALLOWED;
				} else {
					return ERROR_INPUT_HAS_TO_BE_LESS;
				}
			} 
		}
		if (arvo < min){
			if(max == INT_MAX){
				if (min == 0) {
					return ERROR_NEGATIVE_NUMBERS_NOT_ALLOWED;
				} else {
					return ERROR_INPUT_HAS_TO_BE_GREATER;
				}
			} 
		}
		return ERROR_INPUT_IS_NOT_IN_THE_EXTENT;
	}
}

int doubleValissa( void* arvoPtr, void* limits ){

	double arvo = poke(double, arvoPtr, 0);
	double min  = poke(double, limits, 0);
	double max  = poke(double, limits, 1);
	
	if (arvo > min && arvo < max ){
		return SUCCESS;
	} else {
		if (arvo >= max){
			if(min == -DBL_MAX){
				if (max == 0) {
					return ERROR_POSITIVE_NUMBERS_NOT_ALLOWED;
				} else {
					return ERROR_INPUT_DBL_HAS_TO_BE_LESS;
				}
			} 
		}
		if (arvo <= min){
			if(max == DBL_MAX){
				if (min == -DBL_MIN) {
					return ERROR_NEGATIVE_NUMBERS_NOT_ALLOWED;
				} else {
					return ERROR_INPUT_DBL_HAS_TO_BE_GREATER;
				}
			} 
		}
		return ERROR_INPUT_DBL_IS_NOT_IN_THE_EXTENT;
	}
}

/* FUNKTIO tarkistaStringPituus( void*, void* )
Tarkistaa, onko syötetyn merkkijonon pituus hyväksytyllä välillä.

Syötteet:
	arvo	= tarkastettava merkkijono.
	limits	= kahden kokonaisluvun taulukko, joka kertoo pituusalueen rajat. 

Palautusarvo:
	Palauttaa nollan, jos virheitä ei löytynyt. Muussa tapauksessa palautetaan virhekoodi.
*/
int tarkistaStringPituus( void* arvoPtr, void* limits ){

	int pituus = strlen( (string) arvoPtr );
	int min  = poke(int, limits, 0);
	int max  = poke(int, limits, 1);
	
	if (pituus >= min && pituus <= max ){
		return SUCCESS;
	} else {
		if (min == max) return ERROR_STRING_HAS_TO_BE_EXACT_LENGHT;
		if (pituus > max && min <= 1) return ERROR_STRING_TOO_LONG;
		if (pituus < min && max >= INPUT_STRING_LENGHT_MAX) return ERROR_STRING_TOO_SHORT;
		return ERROR_ILLEGAL_STRING_LENGHT;
	}
}

/* FUNKTIO stringSallitutMerkit( void*, void* )
Tarkistaa, sisältääkö merkkijono ainoastaan sallittuja merkkejä.

Syötteet:
	str		= tarkastettava merkkijono.
	chars	= merkkijono sallituista merkeistä. 

Palautusarvo:
	Palauttaa nollan, jos virheitä ei löytynyt. Muussa tapauksessa palautetaan virhekoodi.
*/
int stringSallitutMerkit( void* str, void* chars ){

	int currentCharAllowed;
	int j;
	
	for (int i = 0; poke(char, str, i) != '\0'; i++){
		currentCharAllowed = 0;
		j = 0;
		while ( ( poke(char, chars, j) != '\0') && (currentCharAllowed == 0) ){
			if ( poke(char, str, i) == poke(char, chars, j) ) currentCharAllowed = 1;
			j++;
		}
		if (currentCharAllowed == 0) return ERROR_NONALLOWED_CHARS_IN_STRING;
	}
	
	return SUCCESS;
}

/* FUNKTIO stringSallitutMerkit( void*, void* )
Tarkistaa, sisältääkö merkkijono kiellettyjä merkkejä.

Syötteet:
	str		= tarkastettava merkkijono.
	chars	= merkkijono kielletyistä merkeistä. 

Palautusarvo:
	Palauttaa nollan, jos virheitä ei löytynyt. Muussa tapauksessa palautetaan virhekoodi.
*/
int stringKielletytMerkit( void* str, void* chars ){
	
	for (int i = 0; poke(char, str, i) != '\0'; i++){
		for (int j = 0; poke(char, chars, j) != '\0'; j++){
			if ( poke(char, str, i) == poke(char, chars, j) ) {
				if (poke(char, chars, j) == ' '){
					return ERROR_NO_SPACES_ALLOWED_IN_STRING;
				} else {
					return ERROR_FORBIDDEN_CHARS_IN_STRING;
				}
			}
		}
	}
	
	return SUCCESS;
}

/* FUNKTIO sallituArvot( void*, void* )
Tarkastaa, onko syötteenä oleva arvo listassa.

Syötteet:
	arvo	= tarkastettava arvo.
	data	= lista sallituista arvoista. 	

Palautusarvo:
	Palauttaa nollan, jos arvo on listassa. Muussa tapauksessa palautetaan virhekoodi.
*/
int sallitutArvot( void* arvoPtr, void* data ){

	int tietotyyppi = poke(SallitutArvot, data, 0).tietotyyppi;
	int maara = poke(SallitutArvot, data, 0).arvojenMaara;
	void* arvot = poke(SallitutArvot, data, 0).data;
	int size = 0;
	
	if (tietotyyppi == DATATYPE_int) size = sizeof( int );
	if (tietotyyppi == DATATYPE_double) size = sizeof( double );
	
	if (tietotyyppi == DATATYPE_string) {
		for (int i = 0; i < maara; i++) {
			if ( strcmp( arvoPtr, poke(string, arvot, i) ) == 0) {
				return SUCCESS;
			}
		}
	} else {
		for (int i = 0; i < maara; i++) {
			if ( memcmp( arvoPtr, &poke(char, arvot, i * size), size ) == 0) {
				return SUCCESS;
			}
		}
	}
		
	return ERROR_INPUT_IS_NOT_AN_ALLOWED_VALUE;
}

/* FUNKTIO tarkistusLista( void*, void* )
Suorittaa tarkastuksia arvolle listan mukaan.

Syötteet:
	arvo	= tarkastettava arvo.
	data	= lista suoritettavista tarkastuksista ja niihin liittyvistä parametreista. 

Palautusarvo:
	Palauttaa nollan, jos virheitä ei löytynyt. Muussa tapauksessa palautetaan virhekoodi.
*/
int tarkistusLista( void* arvoPtr, void* data ){

	int error = 0;
	int i = 0;

	while ( poke(Tarkastaja, data, i).tarkastusFunktio != NULL ) {
	
		error = poke(Tarkastaja, data, i).tarkastusFunktio( arvoPtr, poke(Tarkastaja, data, i).parametriData );
		
		if (error) {
			tulostaIlmoitus(error, poke(Tarkastaja, data, i).parametriData);
			return INVISIBLE_ERROR;
		}
		
		i += 1;
	}
		
	return SUCCESS;
}













