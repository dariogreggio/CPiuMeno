// CPiuMenoDoc.h : interface of the CPiuMenoDoc class
//
/////////////////////////////////////////////////////////////////////////////

//#include <afxrich.h>
//troppo incasinato... per ora faccio cosi': (v. anche View)
//#define CRichEditDoc CDocument

// non va più IsModified!!

class CPiuMenoDoc;
class CPiuMenoView;

class CExRichDocument : public CRichEditDoc {
public:
	char *prfSection;
	DWORD Opzioni;
// Operations
public:
	CView *getView() { POSITION pos=GetFirstViewPosition(); return GetNextView(pos); }
	void getWindow(RECT *);
	void move(int, int, int x2=0, int y2=0);
	int GetPrivateProfileInt(int k) { return theApp.GetPrivateProfileInt(prfSection,k);};
	int WritePrivateProfileInt(int k, int v)  { return theApp.WritePrivateProfileInt(prfSection,k,v);};
	int GetPrivateProfileString(int k,char *s,int l,char *def=NULL) { return theApp.GetPrivateProfileString(prfSection,k,s,l,def ? def : "");};
	int WritePrivateProfileString(int k, const char *s)  { return theApp.WritePrivateProfileString(prfSection,k,s);};
	CTime GetPrivateProfileTime(int k) { return theApp.GetPrivateProfileTime(prfSection,k);};
	CTimeSpan GetPrivateProfileTimeSpan(int k) { return theApp.GetPrivateProfileTimeSpan(prfSection,k);};
	int WritePrivateProfileTime(int k, CTime t) { return theApp.WritePrivateProfileTime(prfSection,k,t);};
	int WritePrivateProfileTime(int k, CTimeSpan t) { return theApp.WritePrivateProfileTime(prfSection,k,t);};
	};


class CPiuMenoCntrItem : public CRichEditCntrItem {
	DECLARE_SERIAL(CPiuMenoCntrItem)

// Constructors
public:
	CPiuMenoCntrItem(REOBJECT* preo = NULL, CPiuMenoDoc* pContainer = NULL);
		// Note: pContainer is allowed to be NULL to enable IMPLEMENT_SERIALIZE.
		//  IMPLEMENT_SERIALIZE requires the class have a constructor with
		//  zero arguments.  Normally, OLE items are constructed with a
		//  non-NULL document pointer.

// Attributes
public:
	CPiuMenoDoc* GetDocument()
		{ return (CPiuMenoDoc*)COleClientItem::GetDocument(); }
	CPiuMenoView* GetActiveView()
		{ return (CPiuMenoView*)COleClientItem::GetActiveView(); }

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPiuMenoCntrItem)
	public:
	protected:
	//}}AFX_VIRTUAL

// Implementation
public:
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif
};

/////////////////////////////////////////////////////////////////////////////

class CPiuMenoDoc : public CExRichDocument {
public:
protected: // create from serialization only
	CPiuMenoDoc();
	DECLARE_DYNCREATE(CPiuMenoDoc)

// Attributes
public:
	char myPrfSection[128];

protected:
	uint32_t m_nDocLines;
//	BOOL m_bIsSavingSelf;
	DWORD m_dwLastSelfSaveTime;

// Overrides
	virtual CRichEditCntrItem* CreateClientItem(REOBJECT* preo) const;
	virtual void OnDeactivateUI(BOOL bUndoable);
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPiuMenoDoc)
	public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);
	virtual BOOL OnOpenDocument(LPCTSTR lpszPathName);
	virtual BOOL OnSaveDocument(LPCTSTR lpszPathName);
	virtual void OnCloseDocument();
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CPiuMenoDoc();
  UINT GetDocumentLength() { return m_nDocLines; }		// cmq non viene usata... v. skypic
	BOOL OpenIncludeFile(LPCTSTR lpszIncludeName);

public:
  CUIntArray m_bookmarks; // Memorizza gli indici delle righe (0-based)
  CUIntArray m_breakpoints; // Memorizza 

  // Aggiunge o rimuove un segnalibro mantenendo l'array ordinato
	void SetBookmark(UINT nLine);
  void ToggleBookmark(UINT nLine);
  BOOL HasBookmark(UINT nLine) const;
	void SetBreakpoint(UINT nLine);
  void ToggleBreakpoint(UINT nLine);
  BOOL HasBreakpoint(UINT nLine) const;

	void UpdateMarkers(int nCaretLine, int nDelta);

  virtual void SetModifiedFlag(BOOL bModified = TRUE);
  virtual void SetPathName(LPCTSTR lpszPathName, BOOL bAddToMRU = TRUE);
	void UpdateFrameTitle();

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	//{{AFX_MSG(CPiuMenoDoc)
	afx_msg void OnCompilaFile();
	afx_msg void OnUpdateCompilaFile(CCmdUI* pCmdUI);
	afx_msg void OnModificaInseriscisegnalibro();
	afx_msg void OnDebugTogglebreakpoint();
	afx_msg void OnUpdateDebugTogglebreakpoint(CCmdUI* pCmdUI);
	afx_msg void OnModificaVaialprossimosegnalibro();
	afx_msg void OnModificaVaialsegnalibroprecedente();
	afx_msg void OnUpdateModificaVaialsegnalibroprecedente(CCmdUI* pCmdUI);
	afx_msg void OnUpdateModificaVaialprossimosegnalibro(CCmdUI* pCmdUI);
	afx_msg void OnModificaVaiallariga();
	afx_msg void OnModificaEliminatuttiisegnalibri();
	afx_msg void OnUpdateModificaEliminatuttiisegnalibri(CCmdUI* pCmdUI);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	friend class CPiuMenoView;
	};

/////////////////////////////////////////////////////////////////////////////

class CExDocument : public CDocument {
public:
	char *prfSection;
	DWORD Opzioni;
// Operations
public:
	CView *getView() { POSITION pos=GetFirstViewPosition(); return GetNextView(pos); }
	void getWindow(RECT *);
	void move(int, int, int x2=0, int y2=0);
	int GetPrivateProfileInt(int k) { return theApp.GetPrivateProfileInt(prfSection,k);};
	int WritePrivateProfileInt(int k, int v)  { return theApp.WritePrivateProfileInt(prfSection,k,v);};
	int GetPrivateProfileString(int k,char *s,int l,char *def=NULL) { return theApp.GetPrivateProfileString(prfSection,k,s,l,def ? def : "");};
	int WritePrivateProfileString(int k, const char *s)  { return theApp.WritePrivateProfileString(prfSection,k,s);};
	CTime GetPrivateProfileTime(int k) { return theApp.GetPrivateProfileTime(prfSection,k);};
	CTimeSpan GetPrivateProfileTimeSpan(int k) { return theApp.GetPrivateProfileTimeSpan(prfSection,k);};
	int WritePrivateProfileTime(int k, CTime t) { return theApp.WritePrivateProfileTime(prfSection,k,t);};
	int WritePrivateProfileTime(int k, CTimeSpan t) { return theApp.WritePrivateProfileTime(prfSection,k,t);};
	};


/////////////////////////////////////////////////////////////////////////////
