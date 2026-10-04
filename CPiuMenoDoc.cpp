// CPiuMenoDoc.cpp : implementation of the CPiuMenoDoc class
//

#include "stdafx.h"
#include "CPiuMeno.h"

#include "mainfrm.h"
#include "CPiuMenoDoc.h"
#include "CPiuMenoview.h"
#include "CPiuMenoDlg.h"
//#include <afxext.h>		// per GetWindowFrame dice, ma ovviamente non è vero
//#include <afxpriv.h>		// idem
//#include "childfrm.h"



#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CPiuMenoDoc

IMPLEMENT_DYNCREATE(CPiuMenoDoc, CRichEditDoc)

BEGIN_MESSAGE_MAP(CPiuMenoDoc, CRichEditDoc)
	//{{AFX_MSG_MAP(CPiuMenoDoc)
	ON_COMMAND(ID_COMPILA_FILE, OnCompilaFile)
	ON_UPDATE_COMMAND_UI(ID_COMPILA_FILE, OnUpdateCompilaFile)
	ON_COMMAND(ID_MODIFICA_INSERISCISEGNALIBRO, OnModificaInseriscisegnalibro)
	ON_COMMAND(ID_DEBUG_TOGGLEBREAKPOINT, OnDebugTogglebreakpoint)
	ON_UPDATE_COMMAND_UI(ID_DEBUG_TOGGLEBREAKPOINT, OnUpdateDebugTogglebreakpoint)
	ON_COMMAND(ID_MODIFICA_VAIALPROSSIMOSEGNALIBRO, OnModificaVaialprossimosegnalibro)
	ON_COMMAND(ID_MODIFICA_VAIALSEGNALIBROPRECEDENTE, OnModificaVaialsegnalibroprecedente)
	ON_UPDATE_COMMAND_UI(ID_MODIFICA_VAIALSEGNALIBROPRECEDENTE, OnUpdateModificaVaialsegnalibroprecedente)
	ON_UPDATE_COMMAND_UI(ID_MODIFICA_VAIALPROSSIMOSEGNALIBRO, OnUpdateModificaVaialprossimosegnalibro)
	ON_COMMAND(ID_MODIFICA_VAIALLARIGA, OnModificaVaiallariga)
	ON_COMMAND(ID_MODIFICA_ELIMINATUTTIISEGNALIBRI, OnModificaEliminatuttiisegnalibri)
	ON_UPDATE_COMMAND_UI(ID_MODIFICA_ELIMINATUTTIISEGNALIBRI, OnUpdateModificaEliminatuttiisegnalibri)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPiuMenoDoc construction/destruction

CPiuMenoDoc::CPiuMenoDoc() {

	prfSection=myPrfSection;
	m_nDocLines=0;
	//m_bIsSavingSelf=FALSE; 
	m_dwLastSelfSaveTime = 0;
	}

CPiuMenoDoc::~CPiuMenoDoc() {
	
	}

BOOL CPiuMenoDoc::OnNewDocument() {

	if(!CExRichDocument::OnNewDocument())
		return FALSE;

	return TRUE;
	}

BOOL CPiuMenoDoc::OnOpenDocument(LPCTSTR lpszPathName) {
	char myBuf[256],myBuf2[64],*p;
	RECT rc;
	CPiuMenoView *w=(CPiuMenoView *)getView();

	if(!CExRichDocument::OnOpenDocument(lpszPathName))
		return FALSE;

	CWnd* pChildFrame = w->GetParentFrame();

//	CStringEx S,S1;
//	S.SplitPath(lpszPathName,5);

	p=strrchr(lpszPathName,'\\');
	if(p) {
		p++;
		strcpy(myPrfSection,p);
		}
	GetPrivateProfileString(IDS_COORDINATECHILD,myBuf,32);
	if(*myBuf) {
		sscanf(myBuf,"%d,%d,%d,%d",&rc.left,&rc.top,&rc.right,&rc.bottom);	// sono coord. client rispetto alla MDIFRAME madre della mia ChildFrame
		if(!IsRectEmpty(&rc)) {
      if(pChildFrame ) {
        pChildFrame->SetWindowPos(NULL, 
          rc.left, rc.top, 
          rc.right-rc.left, rc.bottom- rc.top, 
          SWP_NOZORDER | SWP_NOACTIVATE);
        }
			}

		}

	WIN32_FILE_ATTRIBUTE_DATA wfd;
	if(GetFileAttributesEx(lpszPathName, GetFileExInfoStandard, &wfd)) {
		// Registra la finestra corrente presso il monitor globale
		theApp.RegisterMonitoredFile(lpszPathName, pChildFrame->m_hWnd, wfd.ftLastWriteTime);
		}
		

//	          m_bookmarks.InsertAt(0, 2); // PROVA
	//          m_breakpoints.InsertAt(0, 5); // PROVA


	CMainFrame*f=((CMainFrame*)theApp.m_pMainWnd);
/*	HTREEITEM tp0,tp1;

	tp0=f->m_wndProjectTree.GetNextItem(f->projectTreeRoot,TVGN_CHILD);
	S1.SplitPath(lpszPathName,4);
	if(!S1.CompareNoCase(".c"))
		((CMainFrame*)theApp.m_pMainWnd)->m_wndProjectTree.InsertItem(S,2,2,tp0);
	else if(!S1.CompareNoCase(".h")) {
		tp0=f->m_wndProjectTree.GetNextSiblingItem(tp0);
		((CMainFrame*)theApp.m_pMainWnd)->m_wndProjectTree.InsertItem(S,2,2,tp0);
		}


//	tp1=m_wndProjectTree.InsertItem("Source",0,1,projectTreeRoot);
	f->m_wndProjectTree.Expand(tp0,TVE_EXPAND);
*/

//	((CMainFrame*)theApp.m_pMainWnd)->m_wndProjectTree.SetItemImage(tp0,0,0);


	return TRUE;
	}

BOOL CPiuMenoDoc::OnSaveDocument(LPCTSTR lpszPathName) {
	WIN32_FILE_ATTRIBUTE_DATA wfd;
	BOOL bRet;

//	m_bIsSavingSelf=TRUE;
	m_dwLastSelfSaveTime = ::GetTickCount();
	if(GetFileAttributesEx(GetPathName(), GetFileExInfoStandard, &wfd))
   	theApp.UpdateMonitoredFileTimestamp(GetPathName(), wfd.ftLastWriteTime);

	bRet=CDocument::OnSaveDocument(lpszPathName);
//	m_bIsSavingSelf=FALSE;
	return bRet;
	}

void CPiuMenoDoc::OnCloseDocument() {
	CString S,S1;
	char myBuf[64];
	CPiuMenoView *w=(CPiuMenoView *)getView();
	CWnd* pChildFrame = w->GetParentFrame();

	theApp.SaveProjectSection(theApp.nomeProgetto,this);		// per i segnalibri

	
	strcpy(myPrfSection,(LPCTSTR)GetTitle());

	if(w) {		// in chiusura o se file non trovato!
		CWnd* pFrame = w->GetParentFrame();
    if(pFrame) {
			// 3. Otteniamo posizione e stato (Normal, Minimized, Maximized)
			WINDOWPLACEMENT wp;
			wp.length = sizeof(WINDOWPLACEMENT);
    
			if(pFrame->GetWindowPlacement(&wp)) {
				// wp.rcNormalPosition contiene il CRect con le coordinate (Left, Top, Right, Bottom)
				// wp.showCmd contiene lo stato (SW_SHOWMAXIMIZED, SW_SHOWNORMAL, ecc.)
      
				// Salvi queste coordinate nel file di progetto!
				S.LoadString(IDS_OPZIONI);
				S1.LoadString(IDS_COORDINATECHILD);
				wsprintf(myBuf,"%d,%d,%d,%d",wp.rcNormalPosition.left,wp.rcNormalPosition.top,
					wp.rcNormalPosition.right,wp.rcNormalPosition.bottom);
				theApp. /*prStore->*/ WritePrivateProfileString(myPrfSection,S1,myBuf);
				}
			}
		}

//	SaveState(m_nDocType);
	theApp.UnregisterMonitoredFile(pChildFrame->m_hWnd);

	CExRichDocument::OnCloseDocument();
	}

BOOL CPiuMenoDoc::OpenIncludeFile(LPCTSTR lpszIncludeName) {

  if(!lpszIncludeName || !_tcslen(lpszIncludeName))
    return FALSE;

  CString strFoundPath;

  // 1. Cerca nella stessa directory di questo documento
  if(!GetPathName().IsEmpty())    {
    TCHAR szDrive[_MAX_DRIVE], szDir[_MAX_DIR];
    _splitpath(GetPathName(), szDrive, szDir, NULL, NULL);

    CString strCurrentDir;
    strCurrentDir.Format(_T("%s%s"), szDrive, szDir);

    CString strCandidate = strCurrentDir + lpszIncludeName;
		DWORD dwAttr = ::GetFileAttributes(strCandidate);
		if(dwAttr != 0xFFFFFFFF /*INVALID_FILE_ATTRIBUTES*/ && !(dwAttr & FILE_ATTRIBUTE_DIRECTORY)) {
			// Il file esiste ed è un file valido (non una cartella)
			strFoundPath = strCandidate;
			}
    }

  // 2. Fallback: cerca nelle sottocartelle del progetto / include
  if(strFoundPath.IsEmpty())    {
      // ... eventuale ricerca su cartelle relative ...
		}

  // 3. Se trovato, delega all'applicazione l'apertura MDI
  if(!strFoundPath.IsEmpty()) {
    ((CMainFrame*)theApp.m_pMainWnd)->ActivateViewByTitle/*OpenDocumentFile*/(strFoundPath);
    return TRUE;
    }

  // Notifica l'errore se non trovato
  CString strMsg;
  strMsg.Format(_T("Impossibile trovare il file include '%s'."), lpszIncludeName);
  AfxMessageBox(strMsg, MB_ICONEXCLAMATION);

  return FALSE;
	}

CRichEditCntrItem* CPiuMenoDoc::CreateClientItem(REOBJECT* preo) const {
	// cast away constness of this
	return new CPiuMenoCntrItem(preo, (CPiuMenoDoc*)this);
	}

void CPiuMenoDoc::OnDeactivateUI(BOOL bUndoable) {
	CPiuMenoView *w=(CPiuMenoView*)getView();

	if(w->m_bDelayUpdateItems)
		UpdateAllItems(NULL);
/*	SaveState(m_nDocType);			boh, v. wordpad nel caso
	CRichEditDoc::OnDeactivateUI(bUndoable);
	COIPF* pFrame = (COIPF*)m_pInPlaceFrame;
	if (pFrame)	{
		if (pFrame->GetMainFrame())
			ForceDelayed(pFrame->GetMainFrame());
		if (pFrame->GetDocFrame())
			ForceDelayed(pFrame->GetDocFrame());
		}*/
	}


// Aggiunge o rimuove un segnalibro mantenendo l'array ordinato
void CPiuMenoDoc::ToggleBookmark(UINT nLine) {

  for(INT_PTR i=0; i < m_bookmarks.GetSize(); i++)    {
    if(m_bookmarks[i] == nLine)        {
      m_bookmarks.RemoveAt(i); // Rimuove se presente
      return;
      }
    else if(m_bookmarks[i] > nLine)        {
      SetBookmark(nLine); // Inserisce in ordine crescente
      return;
      }
    }
  // Se è maggiore di tutti gli elementi
  SetBookmark(nLine);
	}

void CPiuMenoDoc::SetBookmark(UINT nLine) {

  for(INT_PTR i=0; i < m_bookmarks.GetSize(); i++)    {
    if(m_bookmarks[i] == nLine)
			return;
    if(m_bookmarks[i] > nLine)        {
      m_bookmarks.InsertAt(i,nLine); // Inserisce in ordine crescente
      return;
      }
    }
  m_bookmarks.Add(nLine);
	}

BOOL CPiuMenoDoc::HasBookmark(UINT nLine) const {

  for(INT_PTR i=0; i < m_bookmarks.GetSize(); i++)    {
    if(m_bookmarks[i] == nLine)
      return TRUE;
    if(m_bookmarks[i] > nLine)
      break; // Siccome è ordinato, possiamo interrompere la ricerca prima
		}
  return FALSE;
	}

void CPiuMenoDoc::ToggleBreakpoint(UINT nLine) {

  for(INT_PTR i=0; i < m_breakpoints.GetSize(); i++)    {
    if(m_breakpoints[i] == nLine)        {
      m_breakpoints.RemoveAt(i); // Rimuove se presente
      return;
      }
    else if (m_breakpoints[i] > nLine)        {
      SetBreakpoint(nLine); // Inserisce in ordine crescente
      return;
      }
    }
  // Se è maggiore di tutti gli elementi
  SetBreakpoint(nLine);
	}

void CPiuMenoDoc::SetBreakpoint(UINT nLine) {
	CStringEx S;

	S.SplitPath(GetTitle(),4);
	S.MakeUpper();

	if(S == _T(".C") || S == _T(".ASM")) {
		//isSourceFile(

		for(INT_PTR i=0; i < m_breakpoints.GetSize(); i++)    {
			if(m_breakpoints[i] == nLine)
				return;
			if(m_breakpoints[i] > nLine)        {
				m_breakpoints.InsertAt(i, nLine); // Inserisce in ordine crescente
				return;
				}
			}
		// Se è maggiore di tutti gli elementi
		m_breakpoints.Add(nLine);
		}
	}

BOOL CPiuMenoDoc::HasBreakpoint(UINT nLine) const    {

  for(INT_PTR i=0; i < m_breakpoints.GetSize(); i++)    {
    if(m_breakpoints[i] == nLine)
      return TRUE;
    if(m_breakpoints[i] > nLine)
      break; // Siccome è ordinato, possiamo interrompere la ricerca prima
		}
  return FALSE;
	}

void CPiuMenoDoc::UpdateMarkers(int nCaretLine, int nDelta) {

  // Esempio con un vector di int per i breakpoint
	int i=0;
  while(i < m_bookmarks.GetSize()) {
		int nBpLine = m_bookmarks[i];

    if(nDelta > 0) {
      // Aggiunte righe (es. ENTER o Incolla multiriga):
      // Spostiamo verso il basso tutti i breakpoint che si trovano SOTTO la riga dove stiamo scrivendo
      // --- RIGHE AGGIUNTE (es. Enter / Incolla multiriga) ---
      // Spostiamo verso il basso tutti i breakpoint SOTTO la riga corrente
      if(nBpLine > nCaretLine)
        m_bookmarks[i] += nDelta;
      i++; // Passiamo al prossimo
	    }
		else if (nDelta < 0) {
      // --- RIGHE RIMOSSE (es. Backspace / Delete / Cut) ---
      // nDelta è negativo (es. -3). La zona cancellata va da (nCaretLine + nDelta) a nCaretLine
      int nStartDeleted = nCaretLine + nDelta;
      int nEndDeleted   = nCaretLine;

      if(nBpLine > nStartDeleted && nBpLine <= nEndDeleted) {
          // Il breakpoint cadeva nelle righe cancellate -> Eliminiamo l'elemento!
        m_bookmarks.RemoveAt(i);
          // NON incrementiamo 'i': l'elemento successivo ha preso il posto di quello appena rimosso
				}
      else {
        if (nBpLine > nEndDeleted)          {
          // Se era al di sotto delle righe cancellate, lo tiriamo su
          m_bookmarks.SetAt(i, nBpLine + nDelta); // nDelta è negativo (es. nBpLine + (-3))
          }
        i++; // Passiamo al prossimo
        }
			}
    else
      i++;
		}

	i=0;
  while(i < m_breakpoints.GetSize()) {
		int nBpLine = m_breakpoints[i];

    if(nDelta > 0) {
      // Aggiunte righe (es. ENTER o Incolla multiriga):
      // Spostiamo verso il basso tutti i breakpoint che si trovano SOTTO la riga dove stiamo scrivendo
      // --- RIGHE AGGIUNTE (es. Enter / Incolla multiriga) ---
      // Spostiamo verso il basso tutti i breakpoint SOTTO la riga corrente
      if(nBpLine > nCaretLine)
        m_breakpoints[i] += nDelta;
      i++; // Passiamo al prossimo
	    }
		else if (nDelta < 0) {
      // --- RIGHE RIMOSSE (es. Backspace / Delete / Cut) ---
      // nDelta è negativo (es. -3). La zona cancellata va da (nCaretLine + nDelta) a nCaretLine
      int nStartDeleted = nCaretLine + nDelta;
      int nEndDeleted   = nCaretLine;

      if(nBpLine > nStartDeleted && nBpLine <= nEndDeleted) {
          // Il breakpoint cadeva nelle righe cancellate -> Eliminiamo l'elemento!
        m_breakpoints.RemoveAt(i);
          // NON incrementiamo 'i': l'elemento successivo ha preso il posto di quello appena rimosso
				}
      else {
        if (nBpLine > nEndDeleted)          {
          // Se era al di sotto delle righe cancellate, lo tiriamo su
          m_breakpoints.SetAt(i, nBpLine + nDelta); // nDelta è negativo (es. nBpLine + (-3))
          }
        i++; // Passiamo al prossimo
        }
			}
    else
      i++;
		}

	}


/////////////////////////////////////////////////////////////////////////////
// CPiuMenoDoc serialization

void CPiuMenoDoc::Serialize(CArchive& ar) {
	EDITSTREAM es;

	if(ar.IsStoring())	{
		es.dwCookie = (DWORD)ar.GetFile();
//		es.pfnCallback = CPiuMenoView::MyStreamOutCallback;
		((CPiuMenoView*)m_viewList.GetHead())->StreamOut(es);

//		((CRichEditView*)m_viewList.GetHead())->Serialize(ar);  //da MultiPad...

		//m_bookmarks.Serialize(ar);

		}
	else {
//		CFile cFile(ar.stream,CFile::read);

		es.dwCookie = (DWORD)ar.GetFile();
//		es.pfnCallback = CPiuMenoView::MyStreamInCallback;
		((CPiuMenoView*)m_viewList.GetHead())->StreamIn(es);
//		((CPiuMenoView*)m_viewList.GetTail())->StreamIn(es);

//		((CRichEditView*)m_viewList.GetHead())->Serialize(ar);  //da MultiPad...
		m_nDocLines=((CPiuMenoView*)m_viewList.GetHead())->GetLineCount();		// FINIRE
		//m_bookmarks.Serialize(ar);
		}
	}


void CPiuMenoDoc::SetModifiedFlag(BOOL bModified) {
  // Chiama l'implementazione base di MFC
  CExRichDocument::SetModifiedFlag(bModified);

  // Aggiorna la barra del titolo della finestra MDI
  UpdateFrameTitle();
	}

void CPiuMenoDoc::SetPathName(LPCTSTR lpszPathName, BOOL bAddToMRU) {
  CExRichDocument::SetPathName(lpszPathName, bAddToMRU);
  
  // Assicura che il titolo sia corretto anche all'apertura/salvataggio
  UpdateFrameTitle();
	}

void CPiuMenoDoc::UpdateFrameTitle() {

  // Se il documento non ha ancora una finestra associata, usciamo
  if (m_strTitle.IsEmpty())
    return;

  CString strTitle = GetTitle();

  // Rimuoviamo l'asterisco se già presente per evitare "nome.c **"
  if(strTitle.Right(2) == _T(" *")) {
    strTitle = strTitle.Left(strTitle.GetLength() - 2);
		}

  // Se il file è modificato, aggiungiamo l'asterisco
  if(IsModified()) {
    strTitle += _T(" *");
		}

  // Impostiamo il nuovo titolo del documento
  SetTitle(strTitle);

  // Notifica le viste e la MDI Child Frame di aggiornare la barra del titolo
  UpdateAllViews(NULL);
	}


/////////////////////////////////////////////////////////////////////////////
// CPiuMenoDoc diagnostics

#ifdef _DEBUG
void CPiuMenoDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CPiuMenoDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CPiuMenoDoc commands

void CPiuMenoDoc::OnCompilaFile() {

	if(((CPiuMenoView*)m_viewList.GetHead())->IsModified() /*IsModified()*/)
		OnSaveDocument(GetPathName());	

	theApp.m_pMainWnd->PostMessage(WM_CLSWINDOW,0,(LPARAM)NULL);

	if(!theApp.CompilaFile(GetPathName()))
//		AfxMessageBox("Impossibile caricare il compilatore",MB_ICONEXCLAMATION);
;

fine:
		;
	
	}

void CPiuMenoDoc::OnUpdateCompilaFile(CCmdUI* pCmdUI) {
	CString strFileName=GetPathName();
	
	if(!strFileName.IsEmpty() && !theApp.ccName.IsEmpty()) {
	// Se è un header (.h, .hpp, .i), disabilita la voce di menu
    if(strFileName.Right(2).CompareNoCase(_T(".h")) == 0 ||
      strFileName.Right(4).CompareNoCase(_T(".hpp")) == 0) {
      pCmdUI->Enable(FALSE); // O usi pCmdUI->Enable(FALSE) per ingrigirlo
			}
    else {
      pCmdUI->Enable(TRUE);
			}
		}

	}

/////////////////////////////////////////////////////////////////////////////
// CWordPadCntrItem implementation

IMPLEMENT_SERIAL(CPiuMenoCntrItem, CRichEditCntrItem, 0)

CPiuMenoCntrItem::CPiuMenoCntrItem(REOBJECT *preo, CPiuMenoDoc* pContainer)
	: CRichEditCntrItem(preo, pContainer) {

	}

/////////////////////////////////////////////////////////////////////////////
// CWordPadCntrItem diagnostics

#ifdef _DEBUG
void CPiuMenoCntrItem::AssertValid() const
{
	CRichEditCntrItem::AssertValid();
}

void CPiuMenoCntrItem::Dump(CDumpContext& dc) const
{
	CRichEditCntrItem::Dump(dc);
}
#endif






void CPiuMenoDoc::OnModificaInseriscisegnalibro() {
	CPiuMenoView *w=((CPiuMenoView*)getView());

	int nLineIndex1 = w->GetRichEditCtrl().LineFromChar(-1)  +1;		// zero based
	ToggleBookmark(nLineIndex1);
	w->Invalidate();
	}


void CPiuMenoDoc::OnDebugTogglebreakpoint() {
	CPiuMenoView *w=((CPiuMenoView*)getView());

	int nLineIndex1 = w->GetRichEditCtrl().LineFromChar(-1)  +1;
	ToggleBreakpoint(nLineIndex1);
	w->Invalidate();
	}

void CPiuMenoDoc::OnUpdateDebugTogglebreakpoint(CCmdUI* pCmdUI) {
	
	}

void CPiuMenoDoc::OnModificaVaialprossimosegnalibro() {
	CPiuMenoView *w=((CPiuMenoView*)getView());
	int nLineIndex1 = w->GetRichEditCtrl().LineFromChar(-1) +1;		// zero based
	bool found=FALSE;

  for(INT_PTR i=0; i < m_bookmarks.GetSize(); i++) {
    if(m_bookmarks[i] > nLineIndex1) {
			found=TRUE;
			break;
			}
		}
	if(found) {
		((CMainFrame*)theApp.m_pMainWnd)->GoToRichEditLine(m_bookmarks[i],NULL,FALSE);
		}
	else {
		MessageBeep(-1);
		if(m_bookmarks.GetSize() > 0) {
			((CMainFrame*)theApp.m_pMainWnd)->GoToRichEditLine(m_bookmarks[0],NULL,FALSE);
			}
		}
	
	}

void CPiuMenoDoc::OnModificaVaialsegnalibroprecedente() {
	CPiuMenoView *w=((CPiuMenoView*)getView());
	int nLineIndex1 = w->GetRichEditCtrl().LineFromChar(-1) +1;
	bool found=FALSE;

	if(m_bookmarks.GetSize() == 0)
		return;

  for(INT_PTR i=m_bookmarks.GetSize()-1; i >= 0; i--) {
    if(m_bookmarks[i] < nLineIndex1) {
			found=TRUE;
			break;
			}
		}
	if(found) {
		((CMainFrame*)theApp.m_pMainWnd)->GoToRichEditLine(m_bookmarks[i],NULL,FALSE);
		}
	else {
		MessageBeep(-1);
		if(m_bookmarks.GetSize() > 0) {
			((CMainFrame*)theApp.m_pMainWnd)->GoToRichEditLine(m_bookmarks[m_bookmarks.GetSize()-1],NULL,FALSE);
			}
		}

	}

void CPiuMenoDoc::OnUpdateModificaVaialsegnalibroprecedente(CCmdUI* pCmdUI) {
	
	pCmdUI->Enable(m_bookmarks.GetSize() > 0);	
	}

void CPiuMenoDoc::OnUpdateModificaVaialprossimosegnalibro(CCmdUI* pCmdUI) {
	
	pCmdUI->Enable(m_bookmarks.GetSize() > 0);	
	}



void CPiuMenoDoc::OnModificaVaiallariga() {
	CPiuMenoView *w=((CPiuMenoView*)getView());
	CRichEditCtrl& edit = w->GetRichEditCtrl();
    
  // Calcoliamo la riga attuale dove si trova il cursore per proporla come default
  long nStart, nEnd;
  edit.GetSel(nStart, nEnd);
  int nCurrentLine = edit.LineFromChar(nStart) + 1; // 1-based

  CGotoLineDlg dlg(w);
  dlg.m_nLineNumber = nCurrentLine;

  if(dlg.DoModal() == IDOK) {
      // Usa la tua funzione GotoLine già pronta!
		nCurrentLine=dlg.m_nLineNumber;
		((CMainFrame*)theApp.m_pMainWnd)->GoToRichEditLine(nCurrentLine ,NULL,FALSE);
    }
	}


void CPiuMenoDoc::OnModificaEliminatuttiisegnalibri() {
	CPiuMenoView *w=((CPiuMenoView*)getView());

	m_bookmarks.RemoveAll();
	w->Invalidate();
	}

void CPiuMenoDoc::OnUpdateModificaEliminatuttiisegnalibri(CCmdUI* pCmdUI) {

	pCmdUI->Enable(m_bookmarks.GetSize() > 0);	
	}

