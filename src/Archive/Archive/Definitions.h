//////////////////////////////////////////////
// Soubor s definicemi pouzivanymi pro archivy
//////////////////////////////////////////////

#ifndef __DATA_ARCHIVE_DEFINITIONS_H__INCLUDED__
#define __DATA_ARCHIVE_DEFINITIONS_H__INCLUDED__

// Uspech
#define SUCCESS TRUE

// Neuspech
#define FAILURE FALSE

// Buffer na kopirovani souboru
#define ARCHIVE_DIRECTORY_COPY_BUF_SIZE	65535

// Pri vytvareni archivu se pro roota naalokuje tolik bajtu:
// Hodnota by nemela byt mensi nez 100, aby se tam vesly data o '.' a '..'
#define DATA_ARCHIVE_FILE_DIRECTORY_ROOT_INITIAL_DATA_SIZE 1024

// Pri vytvareni noveho adresare se na zacatek nastavi jeho velikost na nasledujici hodnotu
// Hodnota by nemela byt mensi nez 100, aby se tam vesly data o '.' a '..'
#define DATA_ARCHIVE_FILE_DIRECTORY_INITIAL_DATA_SIZE 1024

// Pri zapisu dat do adresare jeho velikost vzrusta po blocich nasledujici velikosti
#define DATA_ARCHIVE_FILE_DIRECTORY_DATA_GRANULARITY 1024

// Pri vytvareni noveho souboru se na zacatek nastavi jeho velikost na nasledujici hodnotu:
#define DATA_ARCHIVE_FILE_FILE_INITIAL_DATA_SIZE 1024

// Pri pridavani dat do noveho souboru jeho velikost vzrusta po blocich nasledujici velikosti
#define DATA_ARCHIVE_FILE_FILE_DATA_GRANULARITY 1024

// Pri seekovani dopredu se jede po takhle velkych kusech
#define FORWARD_SEEK_BUFFER_SIZE 1024

// Pokud se seekuje dopredu o vice nez tolikhle bytu, soubor se vykopiruje ven
#define MAX_FORWARD_SEEK_IN_PLACE_LENGTH (FORWARD_SEEK_BUFFER_SIZE*50)

// Typ DWORD
typedef unsigned long DWORD;

// Minimum ze tri parametru
#define MIN3(a, b, c) min((a), min((b), (c)))

#endif