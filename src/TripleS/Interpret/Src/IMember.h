/************************************************************
 *
 * Projekt: Strategicka hra
 *    Cast: Interpret
 *   Autor: Helena Kupková
 *  
 *   Popis: Trida CIMember uchovavajici informace o jedne polozce
 *          jednotky nebo struktury.
 *          
 ***********************************************************/

#if !defined(AFX_IMEMBER_H__E57A0495_9A94_11D3_AF79_004F49068BD6__INCLUDED_)
#define AFX_IMEMBER_H__E57A0495_9A94_11D3_AF79_004F49068BD6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CIMember  
{
public:
    CIMember();
    ~CIMember() { Clean(); }

    // naloadovani
    void Load(CFile &file, long nHeapSize);

    // Cleaning
    virtual void Clean();

    // Translation of types (code string table -> global string table)
    virtual void Translate(CICodeStringTable *pCodeStringTable);

    // porovnavani dvou memberu
    bool operator==(CIMember &anotherOne);

public:
    // jmeno memberu
    CStringTableItem *m_stiName;
    // datovy typ
    CIType m_DataType;
    // offset, kde se nachazi v pameti, kde jsou ulozene datove polozky 
    // struktury/jednotky
    int m_nHeapOffset;

protected:
    // ID jmena ve stringtable kodu
    long m_nCodeStringTableID;

};

#endif // !defined(AFX_IMEMBER_H__E57A0495_9A94_11D3_AF79_004F49068BD6__INCLUDED_)
