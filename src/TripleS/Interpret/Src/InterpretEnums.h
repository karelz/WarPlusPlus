/************************************************************
 *
 * Projekt: Strategicka hra
 *    Cast: Interpret
 *   Autor: Helena Kupková
 *  
 *   Popis: Navratova hodnota syscallu
 * 
 ***********************************************************/

#ifndef __INTERPRET_ENUMS__HEADER_INCLUDED__
#define __INTERPRET_ENUMS__HEADER_INCLUDED__

class CInterpret;

typedef enum { IMPL_NONE, IMPL_SCRIPT, IMPL_CPLUSPLUS } EImplementation;

typedef enum {  EVENT_UNKNOWN,
                EVENT_COMMAND,
                EVENT_NOTIFICATION
} EEventType;

// InterpretStatus

typedef enum { STATUS_SUCCESS,                      // vse v poradku
               STATUS_ARGUMENT_TYPE_MISMATCH,       // chyba v typech argumentu
			   STATUS_INVALID_ARGUMENT_COUNT,		// chybny pocet argumentu 
               STATUS_METHOD_NOT_FOUND,             //  metoda jednotky nebyla nenalezena
               STATUS_FUNCTION_NOT_FOUND,           // funkce nebyla nalezena
               STATUS_STRUCTURE_NOT_FOUND,          // struktura nebyla nalezena
               STATUS_MEMBER_NOT_FOUND,             // polozka jednotky nebo struktury nebyla
                                                    // nalezena
               STATUS_NO_SUCH_SYSTEM_OBJECT,        // neznamy systemovy objekt
               STATUS_SYSCALL_ARGUMENT_ERROR,       // chyba v argumentu syscallu
               STATUS_SYSCALL_ERROR,                // chyba v syscallu
			   STATUS_STRING_INDEX_OUT_OF_BOUNDS,   // index do stringu neni v mezich
			   STATUS_ARRAY_INDEX_OUT_OF_BOUNDS,    // index do pole neni v mezich
			   STATUS_CANNOT_CREATE_SYSTEM_OBJECT,  // nemohu vytvorit systemovy objekt
               STATUS_DIVISION_BY_ZERO,             // deleni nulou
               STATUS_MODULO_BY_ZERO,               // modulo nulou
               STATUS_INVALID_REFERENCE,            // neplatna reference 
               STATUS_UNIT_CONVERSION_ERROR,        // chyba pri konverzi jednotek z typu na typ,
                                                    // jednotku nelze konvertovat (neni predkem/potomkem...)
               STATUS_NESTED_SET_FOR_CYCLE,         // vnoreny for-cyklus pro stejnou mnozinu
               STATUS_UPDATE,                       // updatuje se -> zabiji se
               STATUS_INTERPRET_SHUTDOWN,           // interpret finished
               //STATUS_MANDATORY_EVENT_DEFFERED,     // mandatory event deffered (provadelo se neco z horni
                                                    // fronty a neco prislo do dolni -> to horni se muselo odstrelit
			   STATUS_COMMAND_NOTIFICATION_MISMATCH,// pokus o spusteni notification jako command a naopak
               STATUS_COMMAND_DEFFERED,             // provadel se prikaz z horni fronty a neco prislo do spodni fronty
               STATUS_EMPTYING_QUEUE,				// process byl zabit, protoze provadel neco z fronty prikazu, a nekdo ji vyprazdnil
               STATUS_PERMISSION_DENIED,            // process nema pravo na jednotce volat danou metodu
               STATUS_UNIT_IS_DEAD,                 // na jednotce, ktera zemrela, bylo provedeno nebo, nemelo
			   STATUS_GAME_CLOSING,						 // zavira se hra
			   STATUS_SKILL_COLLISION,					 // kolize skill
			   STATUS_INFINITE_CYCLE,				// nekonecny cyklus, process bezel MAX_QUANTUM instrukci
													// bez preplanovani nebo uspani
			   STATUS_FORBIDDEN_SEND_COMMAND,		// zakazane poslani prikazu - pokus o poslani zpravy
													// do dolni fronty prikazu z procesu interpretujici prikaz
													// z horni fronty (->ten by to zabilo a pak by se to zacyklilo)
               STATUS_INFINITE_COMMAND_CYCLE,       // prehlceni prikazu, nekdo nejspis posila prikazy v cyklu,
                                                    // vznikne pokud pocet prikazu prekroci MAX_COMMANDS_COUNT
               STATUS_INFINITE_NOTIFICATION_CYCLE,  // prehlceni noptifikaceni, nekdo nejspis posila notifikace v cyklu,
                                                    // vznikne pokud pocet prikazu prekroci MAX_COMMANDS_COUNT

			   // !!! pri pridavani statusu nezapomenout pridat i retezec do c_EInterpretStatusString
			   // v Interpret.cpp !!!

               STATUS_UNKNOWN_ERROR
} EInterpretStatus;

#define INTERPRET_STATUS_COUNT 32 
// tu musi byt omezene konstantou pro kontrolu, kdyz se prida novy status a neprida se do
// textovych popisu, aby to neslaplo do diry.
extern char* c_EInterpretStatusString[INTERPRET_STATUS_COUNT];

typedef enum { SYSCALL_SUCCESS,         // uspesne volani, pokracuj v interpretaci
               SYSCALL_SUCCESS_SLEEP,   // uspesne blokovaci volani, uspi process
               SYSCALL_ERROR,            // neuspesne volani, zabij process
               SYSCALL_ARGUMENT_ERROR  // spatny parametr syscallu, zabij process
} ESyscallResult;

#endif //__INTERPRET_SYSCALL_RESULT__HEADER_INCLUDED__
