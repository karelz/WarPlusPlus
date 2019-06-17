// ZTimestampedVirtualConnection.h: interface for the CZTimestampedVirtualConnection class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_TIMESTAMPEDVIRTUALCONNECTION_H__6D9B2C03_0AC9_11D4_8030_0000B4A08F9A__INCLUDED_)
#define AFX_TIMESTAMPEDVIRTUALCONNECTION_H__6D9B2C03_0AC9_11D4_8030_0000B4A08F9A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Network/Network/VirtualConnection.h"

// Tohle je obalka na CVirtualVonnection, ktera si drzi aktualni cas,
// ktery se pribaluje na zacatek kazdeho baliku dat
class CZTimestampedVirtualConnection
{
// Konstrukce a destrukce
public:
	// Konstruktor
	CZTimestampedVirtualConnection();

	// Destruktor
	virtual ~CZTimestampedVirtualConnection();

// Vytvareni a ruseni
public:
    // Vytvoreni
    void Create();

    // Zruseni
    void Delete();

// Metody
public:
	// Nastaveni aktualniho casu
	void SetTime(DWORD dwTime);

	// Nastavi odkaz na spojeni
	void SetConnection(CVirtualConnection Connection) { m_VirtualConnection=Connection; }

	// Vrati odkaz na spojeni
	CVirtualConnection GetConnection() { return m_VirtualConnection; }

// Prime volani metod s balenim casu
public:    
	// Otevreme rucne compound block. Vhodne, pokud se jich v dany cas posila obecne
	// nekolik
    inline BOOL BeginSendCompoundBlock(BOOL bCautiousSend=FALSE, BYTE btPacketPriority = PACKETPRIORITY_ACTUAL);
	
	// Odesle blok dat (pokud je prvni v dany cas, pribali se mu na zacatek timestamp)
    inline BOOL SendBlock(const void *pBuffer, DWORD dwBlockSize, BOOL bCautiousSend=FALSE, BYTE btPacketPriority = PACKETPRIORITY_ACTUAL);

	// Ukonci balik dat pro dany cas
    inline BOOL EndSendCompoundBlock();
    
private:
	// Ukazatel na spojeni
	CVirtualConnection m_VirtualConnection;

	// Aktualni cas
	DWORD m_dwActualTime;

	// Flag, ze chceme posilat compound blok
	BOOL m_bCompoundWanted;

	// Flag, ze jsme momentalne mezi zavorkami BeginSendCompoundBlk a EndSendCompoundBlk
	BOOL m_bSending;

    // Flag, ze blok dat, ktery posilame, je prvni po BeginSendCompoundBloku
    BOOL m_bFirst;

#ifdef _DEBUG

    // Aktualni stav (pro debugovani)    
    DWORD m_dwState;

#endif // _DEBUG
};

#ifdef _DEBUG
#define SET_STATE(state) { \
    ASSERT(m_dwState==2 && (state)==0 || m_dwState==0 && (state)==1 || m_dwState==1 && (state)==2); \
    m_dwState=(state); \
    }
#else
#define SET_STATE(state) ;
#endif

/////////////////////////
// Inline metody

BOOL CZTimestampedVirtualConnection::BeginSendCompoundBlock(BOOL bCautiousSend, BYTE btPacketPriority)
{	
	if(m_bSending) {
		// Uz se posilalo, ted chceme, aby zas ne
        ASSERT(m_dwState==2);
		VERIFY(m_VirtualConnection.EndSendCompoundBlock());

#ifdef _DEBUG
        m_dwState=0;
#endif // _DEBUG

		m_bSending=FALSE;
	}
	ASSERT(!m_bSending);
	// Otevreno rucne
	m_bCompoundWanted=TRUE;
	// Otevreme si compound blok

    ASSERT(m_dwState==0);
	VERIFY(m_VirtualConnection.BeginSendCompoundBlock(bCautiousSend, btPacketPriority));
    m_bFirst=TRUE;
    SET_STATE(1);
	// Posilame
	m_bSending=TRUE;
	// A na jeho zacatek zapiseme aktualni cas
	
    ASSERT(m_dwState==1);
    BOOL bResult=m_VirtualConnection.SendBlock(&m_dwActualTime, sizeof(m_dwActualTime), bCautiousSend, btPacketPriority);		
    SET_STATE(2);

    return bResult;
}

BOOL CZTimestampedVirtualConnection::EndSendCompoundBlock()
{	
	// Bud jsme CHTELI slozeny blok, nebo jsme jiz poslali nejaka data
    // (uz odesel PRVNI paket)
    if((!m_bFirst || m_bCompoundWanted) && m_bSending) {
		// Uz neodesilame
		m_bSending=FALSE;
		m_bCompoundWanted=FALSE;
        m_bFirst=TRUE;
        ASSERT(m_dwState==2);
		BOOL bResult=m_VirtualConnection.EndSendCompoundBlock();
        SET_STATE(0);
        return bResult;
	} else {
		return TRUE;
	}
}

BOOL CZTimestampedVirtualConnection::SendBlock(const void *pBuffer, DWORD dwBlockSize, BOOL bCautiousSend, BYTE btPacketPriority)
{
	// Je vubec mozne packet odeslat?
    // Predpokladame, ze maximalni velikost bloku bude 10KB
	ASSERT(m_VirtualConnection.GetMaximumMessageSize()-sizeof(m_dwActualTime)>=10240);

	// Netrvame na slozenem bloku, a do paketu se to uz nevejde
	if(!m_bCompoundWanted && (m_VirtualConnection.GetMaximumMessageSize()-m_VirtualConnection.GetActualDataSize()<dwBlockSize)) { 
		ASSERT(m_bSending);
		// Nas blok se tam uz nevejde
		// Odesleme blok		
		ASSERT(m_dwState==2);
        VERIFY(m_VirtualConnection.EndSendCompoundBlock());
        SET_STATE(0);
		m_bSending=FALSE;
        // Paket, ktery ted prijde, bude prvni
        m_bFirst=TRUE;
	}

	if(m_bFirst && !m_bCompoundWanted) {
		// Otevreme si compound blok
        ASSERT(m_dwState==0);
		VERIFY(m_VirtualConnection.BeginSendCompoundBlock(bCautiousSend, btPacketPriority));
        SET_STATE(1);
		m_bSending=TRUE;
		// A na jeho zacatek zapiseme aktualni cas
        ASSERT(m_dwState==1);
		VERIFY(m_VirtualConnection.SendBlock(&m_dwActualTime, sizeof(m_dwActualTime), bCautiousSend, btPacketPriority));
        SET_STATE(2);
        // A paket, ktery ted posleme je prvni, takze dalsi uz prvni nebudou
        m_bFirst=FALSE;
	}
	
    ASSERT(m_dwState==2);
	return m_VirtualConnection.SendBlock(pBuffer, dwBlockSize, bCautiousSend, btPacketPriority);	
}

#ifdef SET_STATE
#undef SET_STATE
#endif

#endif // !defined(AFX_TIMESTAMPEDVIRTUALCONNECTION_H__6D9B2C03_0AC9_11D4_8030_0000B4A08F9A__INCLUDED_)
