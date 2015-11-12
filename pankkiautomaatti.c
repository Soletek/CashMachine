/* PANKKIAUTOMAATTI
Johdatus ohjelmointiin C-kielellä harjoitustyö.
Tuukka Kurtti
28.12.2014
Oulun Yliopisto
 
Funktiosta on pyritty tekemään optimoituja ja yleiskäyttöisiä.
Joiltain osiltaan koodi ei ole kaikkein yksinkertaisinta, varsinkin seteleihinjako-algoritmin ja
syötteentarkistuksen void-pointteri ja makrorakennelman tulkinta saattaa aiheuttaa päänvaivaa.

Koodi alkaa mielestäni olla jo sellainen, että se on hyvä jakaa useampaan tiedostoon.
Header-tiedostojen tekemistä, ja useamman tiedoston samanaikaista kääntämistä en kuitenkaan jaksanut tässä tehtävässä alkaa opetella.
Sen sijaan lisään includella toisen tiedoston sisällön koodiin sellaisenaan.

Kääntäminen vaatii tietääkseni C99-stantardin käyttöä.
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

// Omat laajennukset (hieman huono toteutus, koska ei käytä headeria)

#include "syote.c"


///////////////////////////////////////
//// MAKROT JA TYYPPIMÄÄRITELMÄT /////
/////////////////////////////////////

// VA_NUM_ARGS() on syote.c -tiedostossa
#define alustaRahanLajittelija( ... ) alustaRahanLajittelija_( VA_NUM_ARGS( __VA_ARGS__ ), __VA_ARGS__ )

/* STRUCT FrobeniusPalikka
Tietorakenne, mitä hyödynnetään Frobeniuksen luvun etsimisessä.

Kentät:
	iteraatio	= monenessa palikassa (iteraatiossa) kyseinen alkio voidaan esittää summana
	tarkastettu	= sisältää tiedon siitä, onko kyseisen alkion mahdollistamia jatkeita tarkastettu
*/
typedef struct FrobeniusPalikka_ {
	int iteraatio;
	bool tarkastettu;
} FrobeniusPalikka;

/* STRUCT FrobeniusTaulukko
Tietorakenne, mitä hyödynnetään Frobeniuksen luvun etsimisessä.
Tähän tietorakenteeseen tallenetaan jokainen tietoa jokaisesta paitsi pienimmästä luvusta.

Kentät:
	askel		= kuinka monta askelta siirrytään eteenpäin palikan sisällä
	väli		= kuinka monta iteraatiota siirrytään eteenpäin palikkaa täytettäessä
*/
typedef struct FrobeniusTaulukko_ {
	int askel;
	int vali;
} FrobeniusTaulukko;

/* STRUCT FrobeniusData
Frobeniuksen luvun hakufunktion palautusarvon tyyppi.

Kentät:
	fnum		= Frobeniuksen luku
	palikka		= palikka, jota luvun etsimesessä käytettiin, jota voidaan hyödyntaa frobeniuksen lukua pienempien lukujen jaollisuuden tutkimiseen
	pohja		= pienin arvo, ja samalla palikan koko
*/
typedef struct FrobeniusData_ {
	int* palikka;
	int fnum;
	int pohja;
} FrobeniusData;

/* STRUCT RahanLajittelija
Ikään kuin eräänlainen luokka, jonka avulla rahasumma voidaan jakaa seteleihin.
Sisältää myös tiedon siitä, minkälaisia seteleitä automaatti voi antaa.

Kentät:
	setelit		= taulukko seteleistä
	maara		= taulukon koko
	rsyt		= rahojen suurin yhteinen tekijä, optimoinnin vuoksi lasketaan etukäteen ja tallennetaan muistiin
	rfdata		= rahojen Frobeniuksen luku ja data, jolla voidaan määritellä, pystytääkö annettu summa jakamaan tasan seteleihin.
				  http://www.cis.upenn.edu/~cis511/Frobenius-number.pdf

Huomio:				  
	RahanLajittejan lähettäminen osoittimena moniin funktiohin olisi varmaan optimoinut koodin suorituskykyä. Ongelmaa tästä ei kuitenkaan
	muodostu, koska tätä ei käytetä nopeuden kannalta kriittisissä paikoissa.
*/
typedef struct RahanLajittelija_ {
	int *setelit;
	int maara;

	int rsyt;
	FrobeniusData rfdata;
} RahanLajittelija;

/* STRUCT Asiakas
Sisältää kaiken yhteen asiakkaaseen liittyvän tiedon, sekä tiedoston, johon tili tallennetaan.

Kentät:
	tilinumero	= asikkaan tilinumero, 0-9999
	tunnusluku	= asikkaan tunnusluku, 0-9999
	saldo		= asikkaan tilin saldo
	tiedosto	= FILE*, joka sisältää asiakkaaseen liittyvän tiedosto-osoittimen
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
//// PÄÄOHJELMA /////
////////////////////

int main ( void ) {	
	
	int luku;
	bool isRunning = true;
	RahanLajittelija perusRahat;
	Asiakas asiakas;
	
	/* Alustetaan RahanLajittelija antamaan 20, 50 ja 100 euron seteleitä. Tätä riviä muuttamalla voidaan pankkiautomaatin antamien
	setelien arvoja hallita vapaasti. (setelit tulee esitellä pienimmästä suurimpaan)(max 20 erilaista seteliä) */
	perusRahat = alustaRahanLajittelija( 20, 50, 100 );
	
	//perusRahat = alustaRahanLajittelija( 1474, 1664, 1999, 6773, 19843, 57683, 360000 );
	
	printf("Tama on pankkiautomaatti, jolla voi nostaa ja tallettaa rahaa seka tarkistaa\n");
	printf("tilin saldon. Automaatin voi sulkea syottamalla tilinumeroksi '0000'.\n\n\n");
	
	do {
		// Kysytään käyttäjältä tilinumeroa, kunnes syötetään kelvollinen tilinumero, jota vastaava tili on olemassa.
		do {
			luku = kysyTunnusluku("ANNA TILINUMERO --> ");
			asiakas = lataaAsiakas( luku );
		} while ( (asiakas.tiedosto == NULL) && (luku != 0) );
		
		// Syöte "0000" lopettaa ohjelman.
		if (luku == 0) break;
		
		// Kysytään tunnusluku, sekä tarkastetaan, onko se oikein.
		if (kysyTunnusluku("ANNA TUNNUSLUKU --> ") != asiakas.tunnusluku){
			printf("Virheellinen tunnusluku!\n\n");
			continue;
		}
		
		// Tulostetaan valikko ja kysytään käyttäjältä hänen haluamaansa toimintoa.
		tulostaValikko();
		luku = kysyValikkoKomento();
		
		// Tehdään annettua komentoa vastaavat toimenpiteet.
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
		
		// Tulostetaan lopetusviesti, jonka jälkeen uusi asiakas voi käyttää automaattia.
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

Syötteet:
	nun		= asiakkaan tilinumero.
	
Palautusarvo:
	Palauttaa Asiakastietueen, joka sisältää kaikki asikkaaseen liittyvät tarpeelliset tiedot.
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
Tallentaa asiakkaaseen tehdyt muutokset sekä sulkee siihen liittyvän tiedoston.

Syötteet:
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
Kysyy käyttäjältä summaa, joka sitten annetaan asiakkaalle virtuaaliseteleinä ja vähennetään hänen tilinsä saldosta.
Nostettavan summa on minimissään nolla ja enintään tilin saldo. Jos valittua summaa ei pystytä antamaan tasan seteleillä,
annetaan valittua summaa pienempi summa, joka on kuitenkin mahdollisimman lähellä haluttua summaa.

Syötteet:
	asiakas		= osoitin asiakkaaseen, joka nostaa rahaa.
	lajittelija	= RahanLajitteja, jota käytetään summan jakamiseen seteleiksi.
*/
void nostaRahaa( Asiakas* asiakas, RahanLajittelija lajittelija ){

	int* setelienMaarat;
	setelienMaarat = jaaSeteleiksi( syote( int, "ANNA NOSTETTAVA SUMMA --> ", VALISSA(0, asiakas->saldo) ), lajittelija);
	asiakas->saldo = asiakas->saldo - tulostaJaLaskeSetelit(setelienMaarat, lajittelija);
	free(setelienMaarat);
}

/* FUNKTIO talletaRahaa( Asiakas* )
Kysyy käyttäjältä summaa, joka lisätään hänen tilinsä saldoon.
Ylivuotojen välttämiseksi suurin mahdollinen summa talletettava on 999999 euroa.

Syötteet:
	asiakas		= osoitin asiakkaaseen, joka tallettaa rahaa.
*/
void talletaRahaa( Asiakas* asiakas ){

	asiakas->saldo = asiakas->saldo + syote( int, "ANNA TALLETETTAVA SUMMA --> ", MONIEHTO( {intPOSITIIVINEN}, {intPIENEMPI_KUIN(1000000)} ) );
	printf("Talletus onnistui.\n");
}


///////////////////////////////////////
//// SYÖTTÖ- JA TULOSTUSFUNKTIOT /////
/////////////////////////////////////

/* FUNKTIO kysyTunnusluku()
Tulostaa näytölle pankkiautomaatin päävalikon.
*/
void tulostaValikko( void ){

	printf("\nValitse toiminto:\n");
	printf("N - Nosto\n");
	printf("T - Talletus\n");
	printf("S - Saldo\n\n");
}

/* FUNKTIO kysyTunnusluku( char[] )
Kysyy käyttäjältä neljä numeromerkkiä pitkän tilinumeron tai tunnusluvun.

Syötteet:
	kysymys		= käyttäjälle syötekentän eteen tulostettava merkkijono.
	
Palautusarvo:
	Int, joka sisältää anniteun syötteen (ilman etunollia)
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
Kysyy käyttäjältä valikkokomennon, joka on joko: N, T tai S.
Funktio hyväksyy sekä pienet että suuret kirjaimet.
	
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
Etsii pienimmän positiivisen kokonaisluvun, jonka kaikki taulukon alkiot jakavat tasan.

Syötteet:
	n		= setelien lukumäärä.
	...		= pilkulla eroteltu lista setelien rahasummista, joita on n kappaletta.
			  Setelit on esiteltävä kasvavassä järjestyksessa eikä samankokoista seteliä saa olla kahdesti.
	
Palautusarvo:
	Palauttaa rahanLajittelijan, jonka avulla rahasumma voidaan jakaa setelien lukumääriksi.
*/
RahanLajittelija alustaRahanLajittelija_( int n, ... ) {

	RahanLajittelija uusiLajittelija;
	
	// Alustetaan funktion argumenttilista.
	va_list varArgs;
	va_start(varArgs, n);
	
	// Varataan dynaamisesti tilaa taulukolle, jonne setelit tallennetaan.
	uusiLajittelija.setelit = (int*) malloc( n * sizeof(int) );
	uusiLajittelija.maara = n;
	
	// Siirretään argumenttilistasta arvot rahanlajittelian.
	for (int i = 0; i < n; i++){
		uusiLajittelija.setelit[i] = va_arg(varArgs, int);
	}
	
	// Lasketaan etukäteen arvoja, joita hyödynnetään rahanjakoalgoitmissä
	uusiLajittelija.rsyt = SYTlist(n, uusiLajittelija.setelit);
	uusiLajittelija.rfdata = frobeniusNumber(n, uusiLajittelija.setelit, uusiLajittelija.rsyt);
	
	// Suljetaan argumenttilista.
	va_end(varArgs);
	
	return uusiLajittelija;
}

/* FUNKTIO poistaRahanLajittelija( RahanLajittelija* )
Vapauttaa rahanlajittelijan varaaman dynaamisen muistin.

Syötteet:
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
Jakaa annetun summan seteleiksi mahdollisimman tarkasti. Annettujen setelien summa ei koskaan ole suurempi kuin syötteenä oleva summa,
mutta voi kuitenkin olla pienempi, jos syötettyä summaa ei voida jakaa seteleiksi tasan. 

Algoritmi priorisoi seuraavia asioita:
1. 	Setelien summa on mahdollisimman lähellä syötettyä rahasummaa.
2.	Rahat annetaan mahdollisimman suurina seteleinä.

Algoritmi ei priorisoi antamaan mahdollisimman pientä määrää seteleitä.

Syötteet:
	summa			= rahamäärä, mikä muunnetaan seteleiksi.
	lajittelija		= RahanLajittelija, mitä käytetään summan lajittelemiseen. (Sisältää setelien koot ja valmiiksi lasketut arvot algoritmin optimoimiseksi)
	
Palautusarvo:
	Palauttaa taulukon, joka sisältää setelien lukumäärät. Taulukko pitää poistaa muistista silloin, kun sitä ei enää tarvita.
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

	// Lasketaan osa, joka on helppo antaa suurimpina seteleinä ja vähennetään se summasta.
	// Tämä vaihe on mukana ainoastaan optimointisyistä, vaikka en ole varma, paljon aikaa (jos ollenkaan) tämä itseasiassa säästää,
	// koska rahojen jako pelkällä rekursiivisella simukalla on perin nopeaa.
	kokonaiset = (summa - lajittelija.rfdata.fnum - 1) / lajittelija.setelit[lajittelija.maara - 1];
	summa = summa - kokonaiset * lajittelija.setelit[lajittelija.maara - 1];

	// Vähennetään summasta pois osa, jota ei voida jakaa tasan seteleiksi.
	summa = summa / lajittelija.rsyt;
	iteraatio = summa / lajittelija.rfdata.pohja;
	paikka = summa % lajittelija.rfdata.pohja;

	while ( iteraatio < lajittelija.rfdata.palikka[paikka] ) {
		paikka = paikka - 1;
		summa = summa - 1;
	}

	// Iteroidaan läpi mahdollsia setelikombinaatiota, kunnes löydetään sellainen, joka jakaa jäljellä olevan summan tasan.
	// Tämä arvo tallentuu taulukkoon setelienMaarat.
	rSilmukka(0, summa, lajittelija.maara - 1, tempTaulukko, setelienMaarat);
	
	// Lisätään loppusummaan aikaisemmin vähennetyt suurimmat setelit.
	setelienMaarat[lajittelija.maara - 1] += kokonaiset;

	// Vapautetaan funktion toimintaa varten varattu dynaaminen muisti.
	free(tempTaulukko);
	
	return setelienMaarat;
}

/* FUNKTIO rSilmukka( int, int, int, const int*, int* )
Apufunktio funktiolle jaaSeteleiksi.
Rekursiivinen silmukka, jolla käydään läpi kaikki järkevät ja mahdolliset seteliyhdistelmät.
Silmukan suoritus päättyy heti, kun ratkaisu on löytynyt.
Suurimmat setelit tarkastetaan ensin, joten jos ratkaisu löytyy, niin se sisältää suurimmat mahdollisimmat setelit.

Palautusarvo:
	Totuusarvo, joka kertoo, onko silmukka suoritettu loppuun ja tulos näin löydetty.
*/
bool rSilmukka( int n, int summa, int syvyys, const int* setelit, int* maarat ){

	bool returnValue;
	int iteraatiot;
	
	// Lasketaan kuinka monta läpikäyntiä maksimissaan tarvitaan
	iteraatiot = summa / setelit[syvyys - n];
	
	// Jos ollaan syvimmällä tasolla, tarkastetaan, meneekö jäljellä oleva summa tasan pienimpiin rahoihin.
	if (n == syvyys){
		if ( summa == (iteraatiot * setelit[syvyys - n]) ) {
			maarat[0] = iteraatiot;
			return true;
		} else {
			return false;
		}
	}

	// Kutsutaan funktiota rekursiivisesti silmukan sisällä. Jos arvo true palautuu, poistutaan rekursiosta tallentaen samalla tulos taulukkoon maarat.
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
Tulostaa listan seteleistä sekä laskee niiden rahasumman.

Syötteet:
	setelienMaarat	= taulukko, johon on tallennettu setelien lukumäärät.
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

Syötteet:
	n 			= taulukon koko, positiivinen kokonaisluku
	taulukko	= kokonaislukutaulukko, joka sisältää positiivisa lukuja
	
Palautusarvo:
	Taulukon alkioiden suurin yhteinen tekijä. Palauttaa nollan, jos epäonnistuttiin.
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
Etsii pienimmän positiivisen kokonaisluvun, jonka kaikki taulukon alkiot jakavat tasan.

Syötteet:
	n 			= taulukon koko, positiivinen kokonaisluku
	taulukko	= kokonaislukutaulukko, joka sisältää positiivisa lukuja
	lukujenSyt	= taulokun suurin yhteinen tekijä. Jos arvo on nolla, funktio laskee arvon.
	
Palautusarvo:
	Taulukon alkioiden pienin yhteinen jaettava. Palauttaa nollan, jos epäonnistuttiin.
	
Huomio:
	Tämä funktio ei ole enää käytössä tässä koodissa. Frobeniuksen luvun avulla rahanlajittelualgoritmia voi optimoida tehokkaammin ja tarkemmin.
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
Etsii listan alkioiden Frobeniuksen luvun, eli pienimmän luvun, jota ei voida ilmoittaa taulukon alkioden arvoja lisäämällä yhteen.

Idea ja teoria: http://www.cis.upenn.edu/~cis511/Frobenius-number.pdf
Algoritmi suunniteltu ja toteutettu kuitenkin itse.

Syötteet:
	n 			= taulukon koko, positiivinen kokonaisluku
	taulukko	= kokonaislukutaulukko, joka sisältää positiivisa lukuja. Algoritmi toimii parhaiten silloin, kun pienin alkio on taulukon alussa.
	lukujenSyt	= taulokun arvojen suurin yhteinen tekijä. Jos arvo on nolla, funktio laskee arvon.
	
Palautusarvo:
	FrobeniusData, joka sisältää taulukon alkioiden Frobeniuksen luvun kerrottuna alkioden suurimmalla yhteisellä tekijällä sekä palikan, jonka avulla luku muodostettiin.
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
	
	// Tarkastetaan syötteet
	if (lukujenSyt == 0) lukujenSyt = SYTlist( n, taulukko );
	
	// Varataan aputaulukolle (eli palikalle) ja alustetaan se. 
	// Tilaa varataan syötetaulukon ensimmäisen alkion verran (joka toivottavasti on taulukon pienin alkio).
	pohjaKoko = taulukko[0] / lukujenSyt;
	palikka = (FrobeniusPalikka*) malloc( pohjaKoko * sizeof( FrobeniusPalikka ) );
	
	palikka[0].iteraatio 	= 0;
	palikka[0].tarkastettu 	= false;
	for (int i = 1; i < pohjaKoko; i++){
		palikka[i].iteraatio 	= -1;
		palikka[i].tarkastettu 	= false;
	}
	
	// Varataan tilaa toiselle taulukolle, joka sisältää tarpeelliset tiedot muista syötetaulukon luvuista ja alustetaan se.
	luvut = (FrobeniusTaulukko*) malloc( (n - 1) * sizeof( FrobeniusTaulukko ) );
	for (int i = 0; i < n - 1; i++){
		luvut[i].askel 	= (taulukko[i + 1] / lukujenSyt) % pohjaKoko;
		luvut[i].vali 	= (taulukko[i + 1] / lukujenSyt) / pohjaKoko;
	}
	
	// Täytetään palikka etsien, monennella iteraatiolla mikäkin palikan kohta täyttyy.
	do {		

		// Etsitään nykyisen palikan pienin iteraatio ja sitä vastaava paikka
		nykyinenIteraatio = -1;
		for (int i = 0; i < pohjaKoko; i++){
			if (palikka[i].tarkastettu == false && palikka[i].iteraatio != -1){
				if( nykyinenIteraatio == -1 || palikka[i].iteraatio < nykyinenIteraatio ){
					nykyinenPaikka = i;
					nykyinenIteraatio = palikka[i].iteraatio;
				}
			}
		}

		// Tarkastetaan kyseisen paikka-iteraatio -parin mahdollistamat "jatkeet", joilla palikkaa voidaan täydentää lisää.
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
	
	// Kun palikka on täytetty, etsitään suurin alkio.
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
Etsii ja palauttaa kahden kokonaisluvun suurimman yhteisen tekijän.

Algoritmin alkuperäinen toteutus: http://www.math.wustl.edu/~victor/mfmm/compaa/gcd.c
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





































