#ifndef __EAPPENDMODES_H__
#define __EAPPENDMODES_H__

// Jak pridavat novy soubor do archivu pomoci fce Append?
enum EAppendModes {
	appendAuto=0,			// Automaticky zvolit komprimovanou/nekomprimovanou verzi
	appendCompressed=1,		// Vzdycky ho pri pridani pakovat
	appendUncompressed=2,	// Nikdy pri pridani nepakovat
    appendRecursive=0,      // Rekurzivni pridavani v AppendDir (prazdny flag, pouzito automaticky)
    appendNonRecursive=4,   // Nerekurzivni pridavani v AppendDir
};

#endif // __EAPPENDMODES_H__