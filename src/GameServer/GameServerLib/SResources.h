/***********************************************************
 * 
 * Projekt: Strategická hra
 *    Èást: Server hry
 *   Autor: Karel Zikmund
 * 
 *   Popis: Datový typ surovin
 * 
 ***********************************************************/

#ifndef __SERVER_RESOURCES__HEADER_INCLUDED__
#define __SERVER_RESOURCES__HEADER_INCLUDED__

#include "Common\Map\MapDataTypes.h"
#include "Common\PersistentStorage\PersistentStorage.h"

//////////////////////////////////////////////////////////////////////
// Datový typ surovin
typedef int TSResources[RESOURCE_COUNT];

// Trida pro snadnou praci s resourcy
class CSResources {

// Metody
public:
	// Konstruktor
	CSResources() {
		for(int i=0; i<RESOURCE_COUNT; i++) m_Resources[i]=0;
	}
	
	// Konstruktor
	CSResources(TSResources &resources) {
		Create(resources);
	}

	void Create(TSResources &resources) {
		memcpy(&m_Resources, &resources, sizeof(resources));
	}

	void Empty() {
		for(int i=0; i<RESOURCE_COUNT; i++) m_Resources[i]=0;
	}

	// Copy konstruktor
	CSResources(const CSResources &resources) {
		// Zavolame operator =
		*this = resources;
	}

	// Operator =
	CSResources& operator=(const CSResources &resources) {
		memcpy(&m_Resources, &resources.m_Resources, sizeof(resources.m_Resources));
		return *this;
	}

	// Operator TSResources&
	operator TSResources&() {
		return m_Resources;
	}

// Aritmetika
public:
	// Scitani
	CSResources operator+(const CSResources &resources) {
		CSResources result;
		for(int i=0; i<RESOURCE_COUNT; i++) {
			result.m_Resources[i]=m_Resources[i]+resources.m_Resources[i];
		}
		return result;
	}

	// Odecitani
	CSResources operator-(const CSResources &resources) {
		CSResources result;
		for(int i=0; i<RESOURCE_COUNT; i++) {
			result.m_Resources[i]=m_Resources[i]-resources.m_Resources[i];
		}
		return result;
	}

	// Scitani s prirazenim
	CSResources& operator+=(const CSResources &resources) {
		for(int i=0; i<RESOURCE_COUNT; i++) {
			m_Resources[i]+=resources.m_Resources[i];
		}
		return *this;
	}

	// Odecitani s prirazenim
	CSResources& operator-=(const CSResources &resources) {
		for(int i=0; i<RESOURCE_COUNT; i++) {
			m_Resources[i]-=resources.m_Resources[i];
		}
		return *this;
	}

// Porovnavani
public:

	// Operator ==
	BOOL operator==(const CSResources &resources) {
		for(int i=0; i<RESOURCE_COUNT; i++)
			if(m_Resources[i]!=resources.m_Resources[i]) return FALSE;
		return TRUE;
	}

	// Operator !=
	BOOL operator!=(const CSResources &resources) {
		return !(*this==resources);
	}

	// Operator <
	BOOL operator<(const CSResources &resources) {
		for(int i=0; i<RESOURCE_COUNT; i++)
			if(m_Resources[i]>=resources.m_Resources[i]) return FALSE;
		return TRUE;
	}

	// Operator <=
	BOOL operator<=(const CSResources &resources) {
		for(int i=0; i<RESOURCE_COUNT; i++)
			if(m_Resources[i]>resources.m_Resources[i]) return FALSE;
		return TRUE;
	}

	// Operator >
	BOOL operator>(const CSResources &resources) {
		return !(*this<=resources);
	}

	// Operator >=
	BOOL operator>=(const CSResources &resources) {
		return !(*this<resources);
	}

	BOOL IsEmpty() {
		for(int i=0; i<RESOURCE_COUNT; i++) {
			if(m_Resources[i]>0) return FALSE;
		}
		return TRUE;
	}

// Speciality
public:
	// Nastavi hodnoty na min(this, resources)
	void Min(const CSResources &resources) {
		for(int i=0; i<RESOURCE_COUNT; i++) {
			if(m_Resources[i]>resources.m_Resources[i])
				m_Resources[i]=resources.m_Resources[i];
		}
	}

	// Nastavi hodnoty na max(this, resources)
	void Max(const CSResources &resources) {
		for(int i=0; i<RESOURCE_COUNT; i++) {
			if(m_Resources[i]<resources.m_Resources[i])
				m_Resources[i]=resources.m_Resources[i];
		}
	}

// Save a load
public:
	// Save
	void Save(CPersistentStorage &storage) {
		BRACE_BLOCK(storage);
		for(int i=0; i<RESOURCE_COUNT; i++) {
			storage << m_Resources[i];
		}
	}

	// Load
	void Load(CPersistentStorage &storage) {
		BRACE_BLOCK(storage);
		for(int i=0; i<RESOURCE_COUNT; i++) {
			storage >> (int &)m_Resources[i];
		}
	}

// Data
public:
	TSResources m_Resources;
};

#endif //__SERVER_RESOURCES__HEADER_INCLUDED__
