#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>

#define S_IFWHT 0160000

/*****************************************************/
/*           FUNKCJA KONTROLI BLEDOW                 */
/*****************************************************/
int errorFunction(char *error_info)
{
	fprintf(stderr, "%s\n", error_info);
	perror("Tresc bledu");
	
	exit(1);
}

/******************************************************/
/*      	       UTILITIES                      */
/******************************************************/

/* wyswietla typ pliku*/
void printFileType(struct stat *file_info)
{
	if(((file_info -> st_mode) & S_IFMT) == S_IFREG)
		printf("-");
	else if(((file_info -> st_mode) & S_IFMT) == S_IFBLK)
		printf("b");
	else if(((file_info -> st_mode) & S_IFMT) == S_IFCHR)
		printf("c");
	else if(((file_info -> st_mode) & S_IFMT) == S_IFDIR)
		printf("d");
	else if(((file_info -> st_mode) & S_IFMT) == S_IFLNK)
		printf("l");
	else if(((file_info -> st_mode) & S_IFMT) == S_IFIFO)
		printf("p");
	else if(((file_info -> st_mode) & S_IFMT) == S_IFSOCK)
		printf("s");
	else if(((file_info -> st_mode) & S_IFMT) == S_IFWHT)
		printf("w");
}

/* wyswietla uprawnienia do pliku */
void printRights(struct stat *file_info)
{
	/* WLASCICIEL */
	/*R*/
	if((file_info -> st_mode ) & S_IRUSR)
		printf("r");
	else
		printf("-");

	/*W*/
	if((file_info -> st_mode) & S_IWUSR)
		printf("w");
	else
		printf("-");

	/*X*/
	if((file_info -> st_mode) & S_IXUSR){
		if((file_info -> st_mode) & S_ISUID)
			printf("s");
		else
			printf("x");
	} else {
		if((file_info -> st_mode) & S_ISUID)
			printf("S");
		else
			printf("-");	
	}

	/*GRUPA*/
	/*R*/
	if((file_info -> st_mode ) & S_IRGRP)
		printf("r");
	else
		printf("-");

	/*W*/
	if((file_info -> st_mode) & S_IWGRP)
		printf("w");
	else
		printf("-");

	/*X*/
	if((file_info -> st_mode) & S_IXGRP){
		if((file_info -> st_mode) & S_ISGID)
			printf("s");
		else
			printf("x");
	} else {
		if((file_info -> st_mode) & S_ISGID)
			printf("S");
		else
			printf("-");	
	}

	/*INNI*/
	/*R*/
	if((file_info -> st_mode ) & S_IROTH)
		printf("r");
	else
		printf("-");

	/*W*/
	if((file_info -> st_mode) & S_IWOTH)
		printf("w");
	else
		printf("-");

	/*X*/
	if((file_info -> st_mode) & S_IXOTH){
		if((file_info -> st_mode) & S_ISVTX)
			printf("t");
		else
			printf("x");
	} else {
		if((file_info -> st_mode) & S_ISVTX)
			printf("T");
		else
			printf("-");	
	}
}

/* wyswietla nazwe wlasciciela */
int printOwnerName(struct stat *file_info)
{
	errno = 0; 									/* poniewaz getpwuid zwraca NULL gdy jest blad lub gdy nie moze odczytac nazwy uzytkownika */
	struct passwd *user = getpwuid(file_info -> st_uid); 				/* struktura zawieraja informacje o uzytkowniku */

	if(user == NULL && errno != 0)
		errorFunction("Blad w funkcji printOwnerName");
	else if(user == NULL)
		printf("%15u", file_info -> st_uid);
	else
		printf("%15s", user -> pw_name);
	
	return 0;
}

/* wyswietla nazwe grupy */
int printGroupName(struct stat *file_info)
{
	errno = 0;  									/* poniewaz getpwuid zwraca NULL gdy jest blad lub gdy nie moze odczytac nazwy grupy */
	struct group *group = getgrgid(file_info -> st_gid); 				/* struktura zawierajaca informacje o grupie */

	if(group == NULL && errno != 0)
		errorFunction("Blad w funkcji printGroupName");
	else if(group == NULL)
		printf("%15u", file_info -> st_gid);
	else
		printf("%15s", group -> gr_name);

	return 0;
}

/* wyswietla ostatnia date modyfikacji */
int printLastModificationDate(struct stat *file_info)
{
	struct tm *mod_time;  															/* struktura przechowywujaca date ostatniej modyfikacji pliku*/
	mod_time = localtime(&(file_info -> st_mtime));
	char tab_miesiace[12][4] = {"sty\0", "lut\0", "mar\0", "kwi\0", "maj\0", "cze\0", "lip\0", "sie\0", "wrz\0", "paz\0", "lis\0", "gru\0"}; 	/* tablica przechowywujaca nazwy miesiecy */
	time_t currTime; 																/*przechowuje biezacy czas*/

	if((currTime = time(NULL)) == -1)
		errorFunction("Blad w funkcji printLastModificationDate - nie mozna pobrac biezacej daty");

	/*jesli wystapil blad w localtime zwroc wyjatek*/
	if(mod_time == NULL){
		errorFunction("Blad w funkcji printLastModificationDate - blad localtime()");
	}

	/* DZIEN */
	printf("  %2d", mod_time -> tm_mday);

	/* MIESIAC */
	printf(" %3s", tab_miesiace[(mod_time -> tm_mon)]);

	/* ROK LUB GODZINA */
	/*jezeli data modyfikacji jest sprzed pol roku wyswietl rok jesli nie - wyswietl godzine*/
	if(difftime(currTime, (file_info -> st_mtime)) < 15552000){
		/* wypisywanie godziny */
		if((mod_time -> tm_hour) < 10)
			printf(" 0%d", mod_time -> tm_hour);
		else 
			printf(" %d", mod_time -> tm_hour);

		printf(":");

		if((mod_time -> tm_min) < 10)
			printf("0%d", mod_time -> tm_min);
		else 
			printf("%d", mod_time -> tm_min);
	} else {
		printf(" %d ", mod_time -> tm_year+1900); 			/* Lata numeruje sie od 1900 (!) */
	}

	return 0;
}

/* wyswietla nazwe pliku */
int printFileName(struct dirent *dirent, struct stat file_info)
{
	char path[PATH_MAX + 1]; 						/*zmienna przechowujaca sciezke do pliku na ktory wskazuje link*/
	ssize_t size; 								/*przechowuje dlugosc sciezki aby dodac '\0' na koncu - gdyby tego nie wyswietlalyby sie czesci sciezki poprzednich linkow symbolicznych*/
	printf("  %s", dirent -> d_name); 					/* wypisuje nazwe pliku*/

	/*jezeli plik jest linkiem symbolicznym podejmij odpowiednie dzialania*/
	if(S_ISLNK(file_info.st_mode)){
		if((size = readlink(dirent -> d_name, path, sizeof(path))) < 0){
			errorFunction("Blad w printFileName");
		} else {
			path[size] = '\0';
			printf(" -> %s", path);
		}
	}

	return 0;
}

/*********************************************************************************/
/*                           GLOWNY KOD TRYBU LS                                 */
/*********************************************************************************/
int ls()
{
	struct dirent **dirent; 	/* przechowuje informacje o pliku (w danej iteracji) m.in. nazwe i typ pliku*/
	struct stat file_info; 		/*szczegolowe informacje o pliku (uprawnienia, informacje o wlascicielu i inne) char - !![wyrzuca bad adres kiedy jest inicjalizowana jako wskaznik - lstat]*/
	int n; 				/*przechowuje ilosc plikow (to co zwraca scandir)*/
	int i = 0;			/*iterator*/
	
	/*"otwieranie" katalogu*/
	if((n = scandir(".", &dirent, NULL, alphasort)) == -1){
		errorFunction("Blad funkcji scandir() w funkcji ls");
	}
	
	/***********************************************************/
	/*       glowna petla iterujaca po otworzonym katalogu     */
	/***********************************************************/
	while(i < n){
		if((lstat(dirent[i] -> d_name, &file_info)) == -1)
			errorFunction("Blad funkcji lstat w funkcji ls");

		/*wyswietl typ pliku*/
		printFileType(&file_info);

		/*wyswietl uprawnienia*/
		printRights(&file_info);

		/*wyswietl ilosc hard linkow*/
		printf("%3d", (int)file_info.st_nlink);

		/*wyswietl wlasciciela - funkcja w razie bledu wyswietla komunikat (nie trzeba stosowac errorFunction())*/
		printOwnerName(&file_info);

		/*wyswietl nazwe grupy - funkcja w razie bledu wyswietla komunikat (nie trzeba stosowac errorFunction()*/
		printGroupName(&file_info);

		/*wyswietl wielkosc pliku*/
		printf("%10ld", file_info.st_size);

		/*wyswietl date ostatniej modyfikacji*/
		printLastModificationDate(&file_info);

		/*wyswietl nazwe pliku - szczegolny przypadek - linku symbolicznego*/
		printFileName(dirent[i], file_info);

		printf("\n");
		
		free(dirent[i]);      		/* zwalnianie zaalokowanej pamieci */
		i++;
	}
	
	free(dirent); 				/* zwalnianie zaalokowanej pamieci */

	return 0;
}

int main(int argc, char **argv)
{
	errno = 0; 	/*bez tego wyrzuca blad pod koniec while w trybie ls*/

	/*wybieranie trybu dzialania programu - jesli argumentow jest za duzo program sie nie wykona*/
	if(argc == 1){
		ls();
	} else {
		fprintf(stderr, "Podano niepoprawna liczbe argumentow!\n");
	}

	return 0;
}
