/* PANKKIAUTOMAATTI
Johdatus ohjelmointiin C-kielell� harjoitusty�.
Tuukka Kurtti
28.12.2014
Oulun Yliopisto
 
Funktiosta on pyritty tekem��n optimoituja ja yleisk�ytt�isi�.
Joiltain osiltaan koodi ei ole kaikkein yksinkertaisinta, varsinkin seteleihinjako-algoritmin ja
sy�tteentarkistuksen void-pointteri ja makrorakennelman tulkinta saattaa aiheuttaa p��nvaivaa.

Koodi alkaa mielest�ni olla jo sellainen, ett� se on hyv� jakaa useampaan tiedostoon.
Header-tiedostojen tekemist�, ja useamman tiedoston samanaikaista k��nt�mist� en kuitenkaan jaksanut t�ss� teht�v�ss� alkaa opetella.
Sen sijaan lis��n includella toisen tiedoston sis�ll�n koodiin sellaisenaan.

K��nt�minen vaatii tiet��kseni C99-stantardin k�ytt��.
*/

//////////////////////////////
//// STANTARDIKIRJASTOT /////
////////////////////////////

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>

// Omat laajennukset (hieman huono toteutus, koska ei k�yt� headeria)

#include "syote.c"


///////////////////////////////////////
//// MAKROT JA TYYPPIM��RITELM�T /////
/////////////////////////////////////

// VA_NUM_ARGS() on syote.c -tiedostossa
#define alustaRahanLajittelija( ... ) alustaRahanLajittelija_( VA_NUM_ARGS( __VA_ARGS__ ), __VA_ARGS__ )

/* STRUCT FrobeniusPalikka
Tietorakenne, mit� hy�dynnet��n Frobeniuksen luvun etsimisess�.

Kent�t:
	iteraatio	= monenessa palikassa (iteraatiossa) kyseinen alkio voidaan esitt�� summana
	tarkastettu	= sis�lt�� tiedon siit�, onko kyseisen alkion mahdollistamia jatkeita tarkastettu
*/
typedef struct FrobeniusPalikka_ {
	int iteraatio;
	bool tarkastettu;
} FrobeniusPalikka;

/* STRUCT FrobeniusTaulukko
Tietorakenne, mit� hy�dynnet��n Frobeniuksen luvun etsimisess�.
T�h�n tietorakenteeseen tallenetaan jokainen tietoa jokaisesta paitsi pienimm�st� luvusta.

Kent�t:
	askel		= kuinka monta askelta siirryt��n eteenp�in palikan sis�ll�
	v�li		= kuinka monta iteraatiota siirryt��n eteenp�in palikkaa t�ytett�ess�
*/
typedef struct FrobeniusTaulukko_ {
	int askel;
	int vali;
} FrobeniusTaulukko;

/* STRUCT FrobeniusData
Frobeniuksen luvun hakufunktion palautusarvon tyyppi.

Kent�t:
	fnum		= Frobeniuksen luku
	palikka		= palikka, jota luvun etsimesess� k�ytettiin, jota voidaan hy�dyntaa frobeniuksen lukua pienempien lukujen jaollisuuden tutkimiseen
	pohja		= pienin arvo, ja samalla palikan koko
*/
typedef struct FrobeniusData_ {
	int* palikka;
	int fnum;
	int pohja;
} FrobeniusData;

/* STRUCT RahanLajittelija
Ik��n kuin er��nlainen luokka, jonka avulla rahasumma voidaan jakaa seteleihin.
Sis�lt�� my�s tiedon siit�, mink�laisia seteleit� automaatti voi antaa.

Kent�t:
	setelit		= taulukko seteleist�
	maara		= taulukon koko
	rsyt		= rahojen suurin yhteinen tekij�, optimoinnin vuoksi lasketaan etuk�teen ja tallennetaan muistiin
	rfdata		= rahojen Frobeniuksen luku ja data, jolla voidaan m��ritell�, pystyt��k� annettu summa jakamaan tasan seteleihin.
				  http://www.cis.upenn.edu/~cis511/Frobenius-number.pdf

Huomio:				  
	RahanLajittejan l�hett�minen osoittimena moniin funktiohin olisi varmaan optimoinut koodin suorituskyky�. Ongelmaa t�st� ei kuitenkaan
	muodostu, koska t�t� ei k�ytet� nopeuden kannalta kriittisiss� paikoissa.
*/
typedef struct RahanLajittelija_ {
	int *setelit;
	int maara;

	int rsyt;
	FrobeniusData rfdata;
} RahanLajittelija;

/* STRUCT Asiakas
Sis�lt�� kaiken yhteen asiakkaaseen liittyv�n tiedon, sek� tiedoston, johon tili tallennetaan.

Kent�t:
	tilinumero	= asikkaan tilinumero, 0-9999
	tunnusluku	= asikkaan tunnusluku, 0-9999
	saldo		= asikkaan tilin saldo
	tiedosto	= FILE*, joka sis�lt�� asiakkaaseen liittyv�n tiedosto-osoittimen
*/
typedef struct Asiakas_{
	int tilinumero;
	int tunnusluku;
	int saldo;
	FILE* tiedosto;
} Asiakas;




///////////////////////////////
//// FUNKTIODEN ESITTELY /////
/////////////////////////////

Asiakas lataaAsiakas( int );
void suljeJaTallennaAsiakas( Asiakas );
void nostaRahaa( Asiakas*, RahanLajittelija );
void talletaRahaa( Asiakas* );

void tulostaValikko( void );
int kysyTunnusluku( char[] );
int kysyValikkoKomento( void );

RahanLajittelija alustaRahanLajittelija_( int, ... );
void poistaRahanLajittelija( RahanLajittelija* );
int* jaaSeteleiksi( int, RahanLajittelija );
int tulostaJaLaskeSetelit( int*, RahanLajittelija );

int SYTlist( int, const int[] );
int PYJlist( int, const int[], int );
FrobeniusData frobeniusNumber( int, const int[], int );
int syt( int, int );


//////////////////////
//// P��OHJELMA /////
////////////////////

int main ( void ) {	
	
	int luku;
	bool isRunning = true;
	RahanLajittelija perusRahat;
	Asiakas asiakas;
	
	/* Alustetaan RahanLajittelija antamaan 20, 50 ja 100 euron seteleit�. T�t� rivi� muuttamalla voidaan pankkiautomaatin antamien
	setelien arvoja hallita vapaasti. (setelit tulee esitell� pienimm�st� suurimpaan)(max 20 erilaista seteli�) */
	perusRahat = alustaRahanLajittelija( 20, 50, 100 );
	
	//perusRahat = alustaRahanLajittelija( 1474, 1664, 1999, 6773, 19843, 57683, 360000 );
	
	printf("Tama on pankkiautomaatti, jolla voi nostaa ja tallettaa rahaa seka tarkistaa\n");
	printf("tilin saldon. Automaatin voi sulkea syottamalla tilinumeroksi '0000'.\n\n\n");
	
	do {
		// Kysyt��n k�ytt�j�lt� tilinumeroa, kunnes sy�tet��n kelvollinen tilinumero, jota vastaava tili on olemassa.
		do {
			luku = kysyTunnusluku("ANNA TILINUMERO --> ");
			asiakas = lataaAsiakas( luku );
		} while ( (asiakas.tiedosto == NULL) && (luku != 0) );
		
		// Sy�te "0000" lopettaa ohjelman.
		if (luku == 0) break;
		
		// Kysyt��n tunnusluku, sek� tarkastetaan, onko se oikein.
		if (kysyTunnusluku("ANNA TUNNUSLUKU --> ") != asiakas.tunnusluku){
			printf("Virheellinen tunnusluku!\n\n");
			continue;
		}
		
		// Tulostetaan valikko ja kysyt��n k�ytt�j�lt� h�nen haluamaansa toimintoa.
		tulostaValikko();
		luku = kysyValikkoKomento();
		
		// Tehd��n annettua komentoa vastaavat toimenpiteet.
		switch( (char) luku ){
			case 'n':	// NOSTO
				nostaRahaa( &asiakas, perusRahat );
				break;
			case 't':	// TALLETUS
				talletaRahaa( &asiakas );
				break;
			case 's':	// SALDO
				printf("Nykyinen saldosi on %d euroa.\n", asiakas.saldo);
				break;
		}
		
		// Tulostetaan lopetusviesti, jonka j�lkeen uusi asiakas voi k�ytt�� automaattia.
		suljeJaTallennaAsiakas( asiakas );
		printf("\nTERVETULOA UUDELLEEN\n\n");
		
	} while (isRunning);
	
	// Vapautetaan dynaaminen muisti ennen ohjelman sulkemista.
	printf("Automaatti sulkeutuu.\n");
	poistaRahanLajittelija( &perusRahat );
	
	return 0;
}


///////////////////////////
//// ASIAKASFUNKTIOT /////
/////////////////////////

/* FUNKTIO lataaAsiakasa( int )
Lataa asiakkaan tiedot muistiin tiedostosta.

Sy�tteet:
	nun		= asiakkaan tilinumero.
	
Palautusarvo:
	Palauttaa Asiakastietueen, joka sis�lt�� kaikki asikkaaseen liittyv�t tarpeelliset tiedot.
*/
Asiakas lataaAsiakas( int num ){

	Asiakas uusiAsiakas;
	char tiedostoNimi[10] = {'\0'};
	
	uusiAsiakas.tilinumero = num;
	uusiAsiakas.tiedosto = NULL;
	sprintf(tiedostoNimi, "%d.tili", uusiAsiakas.tilinumero);
	
	if (num != 0) {
		if ( (uusiAsiakas.tiedosto = fopen(tiedostoNimi, "r")) == NULL ){
			printf("Virheellinen tilinumero; tilia ei ole olemassa!\n\n");
		} else {
			fscanf(uusiAsiakas.tiedosto, "%d", &uusiAsiakas.tunnusluku);
			fscanf(uusiAsiakas.tiedosto, "%d", &uusiAsiakas.saldo);
		}
	}

	return uusiAsiakas;
}

/* FUNKTIO suljeJaTallennaAsiakasa( Asiakas )
Tallentaa asiakkaaseen tehdyt muutokset sek� sulkee siihen liittyv�n tiedoston.

Sy�tteet:
	asiakas	= suljettava ja tallennettava asikas.
*/
void suljeJaTallennaAsiakas( Asiakas asiakas ){

	char tiedostoNimi[10] = {'\0'};
	
	fclose(asiakas.tiedosto);
	
	sprintf(tiedostoNimi, "%d.tili", asiakas.tilinumero);
	asiakas.tiedosto = fopen(tiedostoNimi, "w");
	
	fprintf(asiakas.tiedosto, "%d\n", asiakas.tunnusluku);
	fprintf(asiakas.tiedosto, "%d\n", asiakas.saldo);
	fclose(asiakas.tiedosto);
}

/* FUNKTIO nostaRahaa( Asiakas* )
Kysyy k�ytt�j�lt� summaa, joka sitten annetaan asiakkaalle virtuaalisetelein� ja v�hennet��n h�nen tilins� saldosta.
Nostettavan summa on minimiss��n nolla ja enint��n tilin saldo. Jos valittua summaa ei pystyt� antamaan tasan seteleill�,
annetaan valittua summaa pienempi summa, joka on kuitenkin mahdollisimman l�hell� haluttua summaa.

Sy�tteet:
	asiakas		= osoitin asiakkaaseen, joka nostaa rahaa.
	lajittelija	= RahanLajitteja, jota k�ytet��n summan jakamiseen seteleiksi.
*/
void nostaRahaa( Asiakas* asiakas, RahanLajittelija lajittelija ){

	int* setelienMaarat;
	setelienMaarat = jaaSeteleiksi( syote( int, "ANNA NOSTETTAVA SUMMA --> ", VALISSA(0, asiakas->saldo) ), lajittelija);
	asiakas->saldo = asiakas->saldo - tulostaJaLaskeSetelit(setelienMaarat, lajittelija);
	free(setelienMaarat);
}

/* FUNKTIO talletaRahaa( Asiakas* )
Kysyy k�ytt�j�lt� summaa, joka lis�t��n h�nen tilins� saldoon.
Ylivuotojen v�ltt�miseksi suurin mahdollinen summa talletettava on 999999 euroa.

Sy�tteet:
	asiakas		= osoitin asiakkaaseen, joka tallettaa rahaa.
*/
void talletaRahaa( Asiakas* asiakas ){

	asiakas->saldo = asiakas->saldo + syote( int, "ANNA TALLETETTAVA SUMMA --> ", MONIEHTO( {intPOSITIIVINEN}, {intPIENEMPI_KUIN(1000000)} ) );
	printf("Talletus onnistui.\n");
}


///////////////////////////////////////
//// SY�TT�- JA TULOSTUSFUNKTIOT /////
/////////////////////////////////////

/* FUNKTIO kysyTunnusluku()
Tulostaa n�yt�lle pankkiautomaatin p��valikon.
*/
void tulostaValikko( void ){

	printf("\nValitse toiminto:\n");
	printf("N - Nosto\n");
	printf("T - Talletus\n");
	printf("S - Saldo\n\n");
}

/* FUNKTIO kysyTunnusluku( char[] )
Kysyy k�ytt�j�lt� nelj� numeromerkki� pitk�n tilinumeron tai tunnusluvun.

Sy�tteet:
	kysymys		= k�ytt�j�lle sy�tekent�n eteen tulostettava merkkijono.
	
Palautusarvo:
	Int, joka sis�lt�� anniteun sy�tteen (ilman etunollia)
*/
int kysyTunnusluku( char kysymys[] ){
	
	int arvo;
	string jono;
	
	jono = syote(string, kysymys, MONIEHTO(
					{stringSALLITUT_MERKIT("0123456789")},
					{stringPITUUS(4, 4)}
					));
	
	sscanf(jono, "%d", &arvo);
	free(jono);
	
	return arvo;
}

/* FUNKTIO kysyValikkoKomento()
Kysyy k�ytt�j�lt� valikkokomennon, joka on joko: N, T tai S.
Funktio hyv�ksyy sek� pienet ett� suuret kirjaimet.
	
Palautusarvo:
	Palauttaa annettua komentoa vastaavan pienen kirjaimen char-koodin int-muodossa.
*/
int kysyValikkoKomento( void ){

	string komentoJono;
	char komento;
	
	komentoJono = syote( string, "VALINTASI --> ", MONIEHTO(
					{ stringPITUUS(1, 1) },
					{ stringSALLITUT_ARVOT( "N", "T", "S", "n", "t", "s" ) }
					) );
					
	komento = tolower(komentoJono[0]);
	free(komentoJono);
	printf("\n");

	return (int)komento;
}


//////////////////////////////////////
//// RAHANLAJITTELIJAN FUNKTIOT /////
////////////////////////////////////

/* FUNKTIO alustaRahanLajittelija( int, ... )
Etsii pienimm�n positiivisen kokonaisluvun, jonka kaikki taulukon alkiot jakavat tasan.

Sy�tteet:
	n		= setelien lukum��r�.
	...		= pilkulla eroteltu lista setelien rahasummista, joita on n kappaletta.
			  Setelit on esitelt�v� kasvavass� j�rjestyksessa eik� samankokoista seteli� saa olla kahdesti.
	
Palautusarvo:
	Palauttaa rahanLajittelijan, jonka avulla rahasumma voidaan jakaa setelien lukum��riksi.
*/
RahanLajittelija alustaRahanLajittelija_( int n, ... ) {

	RahanLajittelija uusiLajittelija;
	
	// Alustetaan funktion argumenttilista.
	va_list varArgs;
	va_start(varArgs, n);
	
	// Varataan dynaamisesti tilaa taulukolle, jonne setelit tallennetaan.
	uusiLajittelija.setelit = (int*) malloc( n * sizeof(int) );
	uusiLajittelija.maara = n;
	
	// Siirret��n argumenttilistasta arvot rahanlajittelian.
	for (int i = 0; i < n; i++){
		uusiLajittelija.setelit[i] = va_arg(varArgs, int);
	}
	
	// Lasketaan etuk�teen arvoja, joita hy�dynnet��n rahanjakoalgoitmiss�
	uusiLajittelija.rsyt = SYTlist(n, uusiLajittelija.setelit);
	uusiLajittelija.rfdata = frobeniusNumber(n, uusiLajittelija.setelit, uusiLajittelija.rsyt);
	
	// Suljetaan argumenttilista.
	va_end(varArgs);
	
	return uusiLajittelija;
}

/* FUNKTIO poistaRahanLajittelija( RahanLajittelija* )
Vapauttaa rahanlajittelijan varaaman dynaamisen muistin.

Sy�tteet:
	lajittelija		= RahanLajittelija, joka poistetaan.
*/
void poistaRahanLajittelija( RahanLajittelija* lajittelija ) {

	free(lajittelija->setelit);
	lajittelija->setelit = NULL;
	lajittelija->maara = 0;
	
	free(lajittelija->rfdata.palikka);
	lajittelija->rfdata.palikka = NULL;
	
	return;
}

/* FUNKTIO jaaSeteleiksi( int, RahanLajittelija )
Jakaa annetun summan seteleiksi mahdollisimman tarkasti. Annettujen setelien summa ei koskaan ole suurempi kuin sy�tteen� oleva summa,
mutta voi kuitenkin olla pienempi, jos sy�tetty� summaa ei voida jakaa seteleiksi tasan. 

Algoritmi priorisoi seuraavia asioita:
1. 	Setelien summa on mahdollisimman l�hell� sy�tetty� rahasummaa.
2.	Rahat annetaan mahdollisimman suurina setelein�.

Algoritmi ei priorisoi antamaan mahdollisimman pient� m��r�� seteleit�.

Sy�tteet:
	summa			= raham��r�, mik� muunnetaan seteleiksi.
	lajittelija		= RahanLajittelija, mit� k�ytet��n summan lajittelemiseen. (Sis�lt�� setelien koot ja valmiiksi lasketut arvot algoritmin optimoimiseksi)
	
Palautusarvo:
	Palauttaa taulukon, joka sis�lt�� setelien lukum��r�t. Taulukko pit�� poistaa muistista silloin, kun sit� ei en�� tarvita.
*/
int* jaaSeteleiksi( int summa, RahanLajittelija lajittelija ) {

	int* setelienMaarat;
	int* tempTaulukko;
	
	int kokonaiset = 0;
	int iteraatio;
	int paikka;
	
	bool rSilmukka( int, int, int, const int*, int* );
	
	// Varataan dynaamisesti tilaa taulukoille ja alustetaan ne;
	setelienMaarat 	= (int*) malloc( lajittelija.maara * sizeof(int) );
	tempTaulukko 	= (int*) malloc( lajittelija.maara * sizeof(int) );
	for (int i = 0; i < lajittelija.maara; i++){
		setelienMaarat[i] = 0;
		tempTaulukko[i] = lajittelija.setelit[i] / lajittelija.rsyt;
	}

	// Lasketaan osa, joka on helppo antaa suurimpina setelein� ja v�hennet��n se summasta.
	// T�m� vaihe on mukana ainoastaan optimointisyist�, vaikka en ole varma, paljon aikaa (jos ollenkaan) t�m� itseasiassa s��st��,
	// koska rahojen jako pelk�ll� rekursiivisella simukalla on perin nopeaa.
	kokonaiset = (summa - lajittelija.rfdata.fnum - 1) / lajittelija.setelit[lajittelija.maara - 1];
	summa = summa - kokonaiset * lajittelija.setelit[lajittelija.maara - 1];

	// V�hennet��n summasta pois osa, jota ei voida jakaa tasan seteleiksi.
	summa = summa / lajittelija.rsyt;
	iteraatio = summa / lajittelija.rfdata.pohja;
	paikka = summa % lajittelija.rfdata.pohja;

	while ( iteraatio < lajittelija.rfdata.palikka[paikka] ) {
		paikka = paikka - 1;
		summa = summa - 1;
	}

	// Iteroidaan l�pi mahdollsia setelikombinaatiota, kunnes l�ydet��n sellainen, joka jakaa j�ljell� olevan summan tasan.
	// T�m� arvo tallentuu taulukkoon setelienMaarat.
	rSilmukka(0, summa, lajittelija.maara - 1, tempTaulukko, setelienMaarat);
	
	// Lis�t��n loppusummaan aikaisemmin v�hennetyt suurimmat setelit.
	setelienMaarat[lajittelija.maara - 1] += kokonaiset;

	// Vapautetaan funktion toimintaa varten varattu dynaaminen muisti.
	free(tempTaulukko);
	
	return setelienMaarat;
}

/* FUNKTIO rSilmukka( int, int, int, const int*, int* )
Apufunktio funktiolle jaaSeteleiksi.
Rekursiivinen silmukka, jolla k�yd��n l�pi kaikki j�rkev�t ja mahdolliset seteliyhdistelm�t.
Silmukan suoritus p��ttyy heti, kun ratkaisu on l�ytynyt.
Suurimmat setelit tarkastetaan ensin, joten jos ratkaisu l�ytyy, niin se sis�lt�� suurimmat mahdollisimmat setelit.

Palautusarvo:
	Totuusarvo, joka kertoo, onko silmukka suoritettu loppuun ja tulos n�in l�ydetty.
*/
bool rSilmukka( int n, int summa, int syvyys, const int* setelit, int* maarat ){

	bool returnValue;
	int iteraatiot;
	
	// Lasketaan kuinka monta l�pik�ynti� maksimissaan tarvitaan
	iteraatiot = summa / setelit[syvyys - n];
	
	// Jos ollaan syvimm�ll� tasolla, tarkastetaan, meneek� j�ljell� oleva summa tasan pienimpiin rahoihin.
	if (n == syvyys){
		if ( summa == (iteraatiot * setelit[syvyys - n]) ) {
			maarat[0] = iteraatiot;
			return true;
		} else {
			return false;
		}
	}

	// Kutsutaan funktiota rekursiivisesti silmukan sis�ll�. Jos arvo true palautuu, poistutaan rekursiosta tallentaen samalla tulos taulukkoon maarat.
	for (int i = 0; i <= iteraatiot; i++){
		returnValue = rSilmukka(n + 1, summa - setelit[syvyys - n] * (iteraatiot - i), syvyys, setelit, maarat);
		if ( returnValue ) {
			maarat[syvyys - n] = (iteraatiot - i);
			return returnValue;
		}
	}
	
	return false;
}

/* FUNKTIO tulostaJaLaskeSetelit( int*, RahanLajittelija )
Tulostaa listan seteleist� sek� laskee niiden rahasumman.

Sy�tteet:
	setelienMaarat	= taulukko, johon on tallennettu setelien lukum��r�t.
	rahat			= RahanLajittelija, jolla summa on lajiteltu.
	
Palautusarvo:
	Palauttaa setelien summan.
*/
int tulostaJaLaskeSetelit( int* setelienMaarat, RahanLajittelija rahat ) {

	int summa = 0;

	printf("\n SAAT:\n\n");
	for (int i = 0; i < rahat.maara; i++){
		if (setelienMaarat[i] > 0){
			printf("%3d kpl %4d euron seteleita\n", setelienMaarat[i], rahat.setelit[i]);
		}
		
		summa += setelienMaarat[i] * rahat.setelit[i];
	}
	printf("\n YHTEENSA %d euroa\n", summa);
	
	return summa;
}


////////////////////////////////
//// MATEMATIIKKAFUNKTIOT /////
//////////////////////////////

/* FUNKTIO SYTlist( int, const int[] )
Etsii suurimman positiivisen kokonaisluvun, joka jakaa tasan kaikki taulukon alkiot.

Sy�tteet:
	n 			= taulukon koko, positiivinen kokonaisluku
	taulukko	= kokonaislukutaulukko, joka sis�lt�� positiivisa lukuja
	
Palautusarvo:
	Taulukon alkioiden suurin yhteinen tekij�. Palauttaa nollan, jos ep�onnistuttiin.
*/
int SYTlist( int n, const int taulukko[] ) {

	int pieninSyt = 0;
	int sytArvo;
	
	for (int i = 1; i < n; i++){
		sytArvo = syt(taulukko[i - 1], taulukko[i]);
		
		if (sytArvo < pieninSyt || pieninSyt == 0){
			pieninSyt = sytArvo;
			
			if (sytArvo == 1) return 1;
		}
	}
	
	return pieninSyt;
}

/* FUNKTIO PYJlist( int, const int[], int ) 
Etsii pienimm�n positiivisen kokonaisluvun, jonka kaikki taulukon alkiot jakavat tasan.

Sy�tteet:
	n 			= taulukon koko, positiivinen kokonaisluku
	taulukko	= kokonaislukutaulukko, joka sis�lt�� positiivisa lukuja
	lukujenSyt	= taulokun suurin yhteinen tekij�. Jos arvo on nolla, funktio laskee arvon.
	
Palautusarvo:
	Taulukon alkioiden pienin yhteinen jaettava. Palauttaa nollan, jos ep�onnistuttiin.
	
Huomio:
	T�m� funktio ei ole en�� k�yt�ss� t�ss� koodissa. Frobeniuksen luvun avulla rahanlajittelualgoritmia voi optimoida tehokkaammin ja tarkemmin.
*/
int PYJlist( int n, const int taulukko[], int lukujenSyt ) {

	int sytArvo;
	int pyjArvo;
	unsigned long long tulo = 1;
	unsigned long long sytTulo = 1;	
	
	if (n <= 0) return 0;
	if (lukujenSyt == 0) lukujenSyt = SYTlist( n, taulukko );
	
	for (int i = 0; i < n; i++){
		tulo = tulo * (taulukko[i] / lukujenSyt);
		
		for(int j = i + 1; j < n; j++){ 
			sytArvo = syt(taulukko[i], taulukko[j]) / lukujenSyt;
			sytTulo = sytTulo * sytArvo;
		}
	}

	pyjArvo = tulo / sytTulo;
	
	return pyjArvo * lukujenSyt;
}

/* FUNKTIO frobeniusNumber( int, const int[], int ) 
Etsii listan alkioiden Frobeniuksen luvun, eli pienimm�n luvun, jota ei voida ilmoittaa taulukon alkioden arvoja lis��m�ll� yhteen.

Idea ja teoria: http://www.cis.upenn.edu/~cis511/Frobenius-number.pdf
Algoritmi suunniteltu ja toteutettu kuitenkin itse.

Sy�tteet:
	n 			= taulukon koko, positiivinen kokonaisluku
	taulukko	= kokonaislukutaulukko, joka sis�lt�� positiivisa lukuja. Algoritmi toimii parhaiten silloin, kun pienin alkio on taulukon alussa.
	lukujenSyt	= taulokun arvojen suurin yhteinen tekij�. Jos arvo on nolla, funktio laskee arvon.
	
Palautusarvo:
	FrobeniusData, joka sis�lt�� taulukon alkioiden Frobeniuksen luvun kerrottuna alkioden suurimmalla yhteisell� tekij�ll� sek� palikan, jonka avulla luku muodostettiin.
*/
FrobeniusData frobeniusNumber( int n, const int taulukko[], int lukujenSyt ) {

	FrobeniusPalikka* palikka;
	FrobeniusTaulukko* luvut;
	FrobeniusData palautusArvo;

	int pohjaKoko;
	int nykyinenPaikka;
	int nykyinenIteraatio;
	int uusiPaikka;
	int uusiIteraatio;
	
	// Tarkastetaan sy�tteet
	if (lukujenSyt == 0) lukujenSyt = SYTlist( n, taulukko );
	
	// Varataan aputaulukolle (eli palikalle) ja alustetaan se. 
	// Tilaa varataan sy�tetaulukon ensimm�isen alkion verran (joka toivottavasti on taulukon pienin alkio).
	pohjaKoko = taulukko[0] / lukujenSyt;
	palikka = (FrobeniusPalikka*) malloc( pohjaKoko * sizeof( FrobeniusPalikka ) );
	
	palikka[0].iteraatio 	= 0;
	palikka[0].tarkastettu 	= false;
	for (int i = 1; i < pohjaKoko; i++){
		palikka[i].iteraatio 	= -1;
		palikka[i].tarkastettu 	= false;
	}
	
	// Varataan tilaa toiselle taulukolle, joka sis�lt�� tarpeelliset tiedot muista sy�tetaulukon luvuista ja alustetaan se.
	luvut = (FrobeniusTaulukko*) malloc( (n - 1) * sizeof( FrobeniusTaulukko ) );
	for (int i = 0; i < n - 1; i++){
		luvut[i].askel 	= (taulukko[i + 1] / lukujenSyt) % pohjaKoko;
		luvut[i].vali 	= (taulukko[i + 1] / lukujenSyt) / pohjaKoko;
	}
	
	// T�ytet��n palikka etsien, monennella iteraatiolla mik�kin palikan kohta t�yttyy.
	do {		

		// Etsit��n nykyisen palikan pienin iteraatio ja sit� vastaava paikka
		nykyinenIteraatio = -1;
		for (int i = 0; i < pohjaKoko; i++){
			if (palikka[i].tarkastettu == false && palikka[i].iteraatio != -1){
				if( nykyinenIteraatio == -1 || palikka[i].iteraatio < nykyinenIteraatio ){
					nykyinenPaikka = i;
					nykyinenIteraatio = palikka[i].iteraatio;
				}
			}
		}

		// Tarkastetaan kyseisen paikka-iteraatio -parin mahdollistamat "jatkeet", joilla palikkaa voidaan t�ydent�� lis��.
		if ( nykyinenIteraatio != -1 ){
			palikka[nykyinenPaikka].tarkastettu = true;
			for (int i = 0; i < n - 1; i++){
				uusiPaikka 		= nykyinenPaikka + luvut[i].askel;
				uusiIteraatio 	= nykyinenIteraatio + luvut[i].vali + (uusiPaikka >= pohjaKoko);
				uusiPaikka 		= uusiPaikka % pohjaKoko;
				
				if (uusiIteraatio < palikka[uusiPaikka].iteraatio || palikka[uusiPaikka].iteraatio == -1){
					palikka[uusiPaikka].iteraatio = uusiIteraatio;
				}
			}
		}
	
	} while( nykyinenIteraatio != -1 );
	
	// Kun palikka on t�ytetty, etsit��n suurin alkio.
	nykyinenPaikka = 0;
	nykyinenIteraatio = 0;
	
	for (int i = 0; i < pohjaKoko; i++){
		if (palikka[i].iteraatio >= nykyinenIteraatio){
			nykyinenPaikka = i;
			nykyinenIteraatio = palikka[i].iteraatio;
		}
	}
	
	// Lasketaan Frobeniuksen luvun arvo ja rakennetaan palautusarvo.
	palautusArvo.fnum = (nykyinenIteraatio - 1) * pohjaKoko + nykyinenPaikka;
	palautusArvo.fnum = palautusArvo.fnum * lukujenSyt;
	palautusArvo.pohja = pohjaKoko;
	palautusArvo.palikka = (int*) malloc( pohjaKoko * sizeof( int ) );
	
	for (int i = 0; i < pohjaKoko; i++){
		palautusArvo.palikka[i] = palikka[i].iteraatio;
	}

	// Vapautetaan dynaamisesti varattu muisti.
	free(palikka);
	free(luvut);
	
	return palautusArvo;
}

/* FUNKTIO syt( int, int )
Etsii ja palauttaa kahden kokonaisluvun suurimman yhteisen tekij�n.

Algoritmin alkuper�inen toteutus: http://www.math.wustl.edu/~victor/mfmm/compaa/gcd.c
*/
int syt( int a, int b ) {

	int c;
	
	while ( a != 0 ) {
		c = a; 
		a = b % a;  
		b = c;
	}
	
	return b;
}





































