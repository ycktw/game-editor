/////////////////////////////////////////////////////////////////////////////
// Name:        odcombo.cpp
// Purpose:     wxPGOwnerDrawnComboBox and related classes implementation
// Author:      Jaakko Salli
// Modified by:
// Created:     Jan-25-2005
// RCS-ID:      $Id:
// Copyright:   (c) 2005 Jaakko Salli
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

// ============================================================================
// declarations
// ============================================================================

// ----------------------------------------------------------------------------
// headers
// ----------------------------------------------------------------------------

#if defined(__GNUG__) && !defined(NO_GCC_PRAGMA)
    #pragma implementation "odcombobox.h"
#endif

#include "wx/wxprec.h"



#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#if wxUSE_COMBOCTRL

#ifndef WX_PRECOMP
    #include "wx/log.h"

    #include "wx/button.h"
    #include "wx/combobox.h"
    #include "wx/textctrl.h"
    #include "wx/dcclient.h"
    #include "wx/settings.h"
    #include "wx/dialog.h"

#endif

#include "wx/vlbox.h"
#include "wx/tooltip.h"
#include "wx/timer.h"
#include "wx/treectrl.h" //maks
#include "wx/tokenzr.h" //maks

#include "wx/propgrid/odcombo.h"

WX_DEFINE_ARRAY_PTR(void *, VoidPtrArray); //maks

class VoidPtrTreeData : public wxTreeItemData //maks
{
public:
    VoidPtrTreeData(void *_data) : data(_data) { }
    void *data;
};

// Milliseconds to wait for two mouse-ups after focus inorder
// to trigger a double-click.
#define wxODCB_DOUBLE_CLICK_CONVERSION_TRESHOLD     500


//
// Some platform specific constants
//

//
// wxODC_SELECTION_STYLE
//
// 0 = Windows Style (both control and popup: blue background plus lighter dotted edge)
// 1 = GTK'ish Style (popup: blue background. control: text-col dotted edge)
// 2 = A mix (both control and popup: blue background but no edge)
//

//
// wxODC_BORDER_STYLE
//
// 0 = Borders in main control (they surround borderless textctrl and button)
// 1 = GTK Style (border only to textctrl)
//


// Popupwin is really not supported on wxMAC, regardless what the
// wxUSE_POPUPWIN says.
#if defined(__WXMAC__)
#undef wxUSE_POPUPWIN
#define wxUSE_POPUPWIN 0
#endif


#if defined(__WXMSW__)
    // tested

#define wxODC_TEXTCTRLXADJUST               3 // position adjustment for wxTextCtrl
#define wxODC_TEXTCTRLYADJUST               4

#define wxODC_TEXTXADJUST                   0 // how much is read-only text's x adjusted

#define wxODC_DROPDOWN_BUTTON_MAX_WIDTH     19

#define wxODC_SUNKEN_BORDER_WIDTH           2

#define wxODC_SELECTION_STYLE               0

#define wxODC_BORDER_STYLE                  0

#define wxODC_BUTTON_DOWN_WHILE_POPUP       0 // button pressed while popup is shown

#define wxODC_ALLOW_FAKE_POPUP              1 // Use only on plats with problems with wxPopupWindow

#define wxODC_USE_TRANSIENT_POPUP           0 // Use wxPopupWindowTransient (preferred, if it works properly on platform)

//#undef wxUSE_POPUPWIN
//#define wxUSE_POPUPWIN 0

#elif defined(__WXGTK__)
    // tested
#define wxODC_TEXTCTRLXADJUST               2 // position adjustment for wxTextCtrl
#define wxODC_TEXTCTRLYADJUST               0

#define wxODC_TEXTXADJUST                   1 // how much is read-only text's x adjusted

#define wxODC_DROPDOWN_BUTTON_MAX_WIDTH     19

#define wxODC_SUNKEN_BORDER_WIDTH           2

#define wxODC_SELECTION_STYLE               2

#define wxODC_BORDER_STYLE                  0 // Try 1 for alternate

#define wxODC_BUTTON_DOWN_WHILE_POPUP       1 // button pressed while popup is shown

// Fake popup windows cause focus problems on GTK2 (but enable on GTK1.2, just in case)
#if defined(__WXGTK20__)
    #define wxODC_ALLOW_FAKE_POPUP          0 // Use only on plats with problems with wxPopupWindow
#else
    #define wxODC_ALLOW_FAKE_POPUP          1 // Use only on plats with problems with wxPopupWindow
#endif

#define wxODC_USE_TRANSIENT_POPUP           0 // Use wxPopupWindowTransient (preferred, if it works properly on platform)

#elif defined(__WXMAC__)
    // tested partially

#define wxODC_TEXTCTRLXADJUST               3 // position adjustment for wxTextCtrl
#define wxODC_TEXTCTRLYADJUST               0

#define wxODC_TEXTXADJUST                   0 // how much is read-only text's x adjusted

#define wxODC_DROPDOWN_BUTTON_MAX_WIDTH     19

#define wxODC_SUNKEN_BORDER_WIDTH           2

#define wxODC_SELECTION_STYLE               2

#define wxODC_BORDER_STYLE                  0

#define wxODC_BUTTON_DOWN_WHILE_POPUP       0 // button pressed while popup is shown

#define wxODC_ALLOW_FAKE_POPUP              1 // Use only on plats with problems with wxPopupWindow

#define wxODC_USE_TRANSIENT_POPUP           0 // Use wxPopupWindowTransient (preferred, if it works properly on platform)

#else
    // defaults
    // tested on: none

#define wxODC_TEXTCTRLXADJUST               0 // position adjustment for wxTextCtrl
#define wxODC_TEXTCTRLYADJUST               0

#define wxODC_TEXTXADJUST                   0 // how much is read-only text's x adjusted

#define wxODC_DROPDOWN_BUTTON_MAX_WIDTH     17

#define wxODC_SUNKEN_BORDER_WIDTH           2

#define wxODC_SELECTION_STYLE               1

#define wxODC_BORDER_STYLE                  0

#define wxODC_BUTTON_DOWN_WHILE_POPUP       1 // button pressed while popup is shown

#define wxODC_ALLOW_FAKE_POPUP              1 // Use only on plats with problems with wxPopupWindow

#define wxODC_USE_TRANSIENT_POPUP           0 // Use wxPopupWindowTransient (preferred, if it works properly on platform)

#endif


#if wxUSE_POPUPWIN
    #include "wx/popupwin.h"
#endif


// For versions < 2.6.2, don't enable transient popup. There may be
// problems I don't have time to test properly.
#if wxMINOR_VERSION < 6 || ( wxMINOR_VERSION == 6 && wxRELEASE_NUMBER < 2 )
    #undef wxODC_USE_TRANSIENT_POPUP
    #define wxODC_USE_TRANSIENT_POPUP 0
#endif


#if wxODC_USE_TRANSIENT_POPUP
    #undef wxODC_ALLOW_FAKE_POPUP
    #define wxODC_ALLOW_FAKE_POPUP 0

    #define wxComboPopupWindowBase wxPopupTransientWindow
    #define wxODC_INSTALL_TOPLEV_HANDLER       0

#elif wxUSE_POPUPWIN

    #define wxComboPopupWindowBase wxPopupWindow
    #define wxODC_INSTALL_TOPLEV_HANDLER       1

#else

    #define wxComboPopupWindowBase wxDialog
    #if !wxODC_ALLOW_FAKE_POPUP
        #define wxODC_INSTALL_TOPLEV_HANDLER      0 // Doesn't need since can monitor active event
    #else
        #define wxODC_INSTALL_TOPLEV_HANDLER      1 // Doesn't need since can monitor active event
    #endif

#endif


//
// ** TODO **
//
// * proper tab traversal.
// * simple border detection (currently only sunken)
// * correct/faster/better colour acquisition
// * maybe: wxOwnerDrawnChoice. Simple derivate with forced read-only style.
//     Need to use m_typeSelectEvent to use different wx event type.
// * maybe: wxODCB_CUSTOM_ACTION style - redirects button clicks to combo's
//     event handler and ignores them itself.
// * maybe: ability switch between read-only and writable.
// * maybe: msw dropdown button use parent's background colour
//     (needs changes in native_drawdropbutton)
// * maybe: patch wxVListBox to scroll to selected in OnSize handler
//     (con: relatively expensive operation)
//

// constants
// ----------------------------------------------------------------------------

// the margin between the text control and the combo button
static const wxCoord g_comboMargin = 2;

// ----------------------------------------------------------------------------
// wxComboDropButton
// ----------------------------------------------------------------------------

#include "wx/renderer.h"

#define wxDropDownButtonBase wxWindow

class wxComboDropButton : public wxDropDownButtonBase
{
public:

    wxComboDropButton();
    wxComboDropButton(wxWindow *parent,
             wxWindowID id = wxID_ANY,
             const wxPoint& pos = wxDefaultPosition,
             const wxSize& size = wxDefaultSize,
             long style = 0,
             const wxString& name = wxButtonNameStr)
    {
        Init();
        Create(parent, id, pos, size, style, name);
    }

    bool Create(wxWindow *parent,
                wxWindowID id = wxID_ANY,
                const wxPoint& pos = wxDefaultPosition,
                const wxSize& size = wxDefaultSize,
                long style = 0,
                const wxString& name = wxButtonNameStr);

    virtual ~wxComboDropButton();

    // Calling this with popup window pointer indicates
    // that the popup is now visible, and with NULL that
    // it is closed. Reasons why it is needed:
    // * Combo boxes on some platforms show depreseed
    //   button when popup is visible.
    // * Some platforms may incorrectly relay keyboard
    //   events to button and they need to be relayed.
    // * Prevent re-triggering popup immediately after
    //   popup has been closed.
    void SetPopup ( wxWindow* popup );

protected:

    void Init();

    // event handlers
    void OnMouseEvent(wxMouseEvent& event);
    void OnPaint(wxPaintEvent& event);
    void OnKeyEvent(wxKeyEvent& event);

    virtual wxSize DoGetBestSize() const;

protected:

    // ignore button clicks before this time
    wxLongLong              m_timeCanClick;

    // should be not NULL when shown
    wxWindow*               m_popup;

    // state of the button currently drawn
    int                     m_state;

private:

    DECLARE_DYNAMIC_CLASS(wxComboDropButton)

    DECLARE_EVENT_TABLE()
};

BEGIN_EVENT_TABLE(wxComboDropButton, wxDropDownButtonBase)
    EVT_MOUSE_EVENTS(wxComboDropButton::OnMouseEvent)
    EVT_PAINT(wxComboDropButton::OnPaint)
    EVT_KEY_DOWN(wxComboDropButton::OnKeyEvent)
    EVT_KEY_UP(wxComboDropButton::OnKeyEvent)
END_EVENT_TABLE()

IMPLEMENT_DYNAMIC_CLASS(wxComboDropButton,wxDropDownButtonBase)

wxComboDropButton::wxComboDropButton()
{
    Init();
}

void wxComboDropButton::Init()
{
    m_popup = (wxWindow*) NULL;
    m_state = 0;
    m_timeCanClick = 0;
}

bool wxComboDropButton::Create(wxWindow *parent,
                              wxWindowID id,
                              const wxPoint& pos,
                              const wxSize& size,
                              long style,
                              const wxString& name)
{
    if ( wxDropDownButtonBase::Create(parent,id,pos,size,style,name) )
    {
        // Cannot set background style to custom since atleast some XP
        // themes need to draw background.
        return true;
    }
    return false;
}

wxComboDropButton::~wxComboDropButton()
{
    if ( HasCapture() )
        ReleaseMouse ();
}

wxSize wxComboDropButton::DoGetBestSize() const
{

    int fhei = 20;
    if ( m_font.Ok() )
        fhei = m_font.GetPointSize()*2;

    return wxSize(wxODC_DROPDOWN_BUTTON_MAX_WIDTH,fhei);
}

void wxComboDropButton::OnMouseEvent(wxMouseEvent& event)
{
    int type = event.GetEventType();
    int x = event.m_x;
    int y = event.m_y;
    wxSize sz = GetClientSize();
    bool is_inside = ( (x > 0 && x < sz.x) && (y > 0 && y < sz.y) );

    if ( type == wxEVT_MOTION )
    {
        if ( is_inside )
        {
            if ( !(m_state & wxCONTROL_CURRENT) )
            {
                // Mouse hover begins
                m_state |= wxCONTROL_CURRENT;
                if ( HasCapture() ) // Retain pressed state.
                    m_state |= wxCONTROL_PRESSED;
                Refresh();
            }
        }
        else if ( (m_state & wxCONTROL_CURRENT) )
        {
            // Mouse hover ends
            m_state &= ~(wxCONTROL_CURRENT|wxCONTROL_PRESSED);
            Refresh();
        }
    }
    else if ( type == wxEVT_LEFT_DOWN )
    {
        // Only accept event if it wasn't right after popup dismiss
        if ( ::wxGetLocalTimeMillis() > m_timeCanClick )
        {
            // Need to test this, because it might be outside.
            if ( is_inside )
            {
                m_state |= wxCONTROL_PRESSED;
                Refresh();
                CaptureMouse();
            }
        }
        else
        {
            m_state = 0;
        }
    }
    else if ( type == wxEVT_LEFT_UP )
    {
        // Only accept event if mouse was left-press was previously accepted
        if ( HasCapture() )
        {
            ReleaseMouse();

            // If mouse was inside, fire the click event.
            if ( is_inside )
            {
                wxCommandEvent event(wxEVT_COMMAND_BUTTON_CLICKED, GetId());
                event.SetEventObject(this);
                GetEventHandler()->AddPendingEvent(event);
            }

            m_state &= ~(wxCONTROL_PRESSED);
            Refresh();
        }
    }
    else if ( type == wxEVT_LEAVE_WINDOW )
    {
        if ( m_state & wxCONTROL_CURRENT )
        {
            // Mouse hover ends
            m_state &= ~(wxCONTROL_CURRENT|wxCONTROL_PRESSED);
            Refresh();
        }
    }
    else if ( m_popup )
    {
        //wxLogDebug(wxT("wxComboDropButton::OnMouseEvent(%i)"),(int)event.GetEventType());
        m_popup->AddPendingEvent(event);
    }
}

void wxComboDropButton::OnPaint(wxPaintEvent& )
{
    wxPaintDC dc(this);

    wxRect rect( wxPoint(0,0), GetClientSize() );

    int draw_state = m_state;

#if wxODC_BUTTON_DOWN_WHILE_POPUP
    if ( m_popup )
        draw_state |= wxCONTROL_PRESSED;
#endif

    wxRendererNative::Get().DrawComboBoxDropButton(this,
                                                   dc,
                                                   rect,
                                                   draw_state);

}

// relay keys handling (this *is* required atleast for wxMSW)
// FIXME: Remove this after popup window can get focus.
void wxComboDropButton::OnKeyEvent(wxKeyEvent& event)
{
    if ( m_popup )
        m_popup->AddPendingEvent(event);
}

// Calling this with ptr and NULL indicates whether
// popup is visible or not. required for correct event
// handling.
void wxComboDropButton::SetPopup ( wxWindow* popup )
{
    m_popup = popup;

    if ( !popup )
    {
        m_timeCanClick = ::wxGetLocalTimeMillis() + 150;
    }

#if wxODC_BUTTON_DOWN_WHILE_POPUP
    Refresh();
#endif
}

// ----------------------------------------------------------------------------
// wxComboFrameEventHandler takes care of hiding the popup when stuff happens
// in its top level parent.
// ----------------------------------------------------------------------------

#if wxODC_INSTALL_TOPLEV_HANDLER

//
// This will no longer be necessary after wxTransientPopupWindow
// works well on all platforms.
//

class wxComboFrameEventHandler : public wxEvtHandler
{
public:
    wxComboFrameEventHandler( wxPGCustomComboBox* pCb );
    ~wxComboFrameEventHandler();

    void OnPopup();

    void OnIdle( wxIdleEvent& event );
    void OnMouseEvent( wxMouseEvent& event );
    void OnActivate( wxActivateEvent& event );
    void OnResize( wxSizeEvent& event );
    void OnMove( wxMoveEvent& event );
    void OnMenuEvent( wxMenuEvent& event );
    void OnClose( wxCloseEvent& event );

protected:
    wxWindow*               m_focusStart;
    wxPGCustomComboBox*       m_combo;

private:
    DECLARE_EVENT_TABLE()
};

BEGIN_EVENT_TABLE(wxComboFrameEventHandler, wxEvtHandler)
    EVT_IDLE(wxComboFrameEventHandler::OnIdle)
    EVT_LEFT_DOWN(wxComboFrameEventHandler::OnMouseEvent)
    EVT_RIGHT_DOWN(wxComboFrameEventHandler::OnMouseEvent)
    EVT_SIZE(wxComboFrameEventHandler::OnResize)
    EVT_MOVE(wxComboFrameEventHandler::OnMove)
    EVT_MENU_HIGHLIGHT(wxID_ANY,wxComboFrameEventHandler::OnMenuEvent)
    EVT_MENU_OPEN(wxComboFrameEventHandler::OnMenuEvent)
    EVT_ACTIVATE(wxComboFrameEventHandler::OnActivate)
    EVT_CLOSE(wxComboFrameEventHandler::OnClose)
END_EVENT_TABLE()

wxComboFrameEventHandler::wxComboFrameEventHandler( wxPGCustomComboBox* combo )
    : wxEvtHandler()
{
    m_combo = combo;
}

wxComboFrameEventHandler::~wxComboFrameEventHandler()
{
}

void wxComboFrameEventHandler::OnPopup()
{
    m_focusStart = ::wxWindow::FindFocus();
}

void wxComboFrameEventHandler::OnIdle( wxIdleEvent& event )
{
    wxWindow* win_focused = ::wxWindow::FindFocus();

    wxWindow* popup = m_combo->GetPopupControl();
    wxWindow* winpopup = m_combo->GetPopupWindow();

    if ( win_focused && //maks
         win_focused != m_focusStart &&
         win_focused != popup && 
         win_focused->GetParent() != popup &&
         win_focused != winpopup &&
         win_focused->GetParent() != winpopup &&
         win_focused != m_combo &&
         win_focused != m_combo->GetButton() // GTK (atleast) requires this
        )
    {
        //wxASSERT ( win_focused );
        //wxLogDebug( wxT("FOCUSED: class: %s"), win_focused->GetClassInfo()->GetClassName() );
        m_combo->HidePopup( false );    
    }

    event.Skip();
}

void wxComboFrameEventHandler::OnMenuEvent( wxMenuEvent& event )
{
    m_combo->HidePopup( false );
    event.Skip();
}

void wxComboFrameEventHandler::OnMouseEvent( wxMouseEvent& event )
{
    m_combo->HidePopup( false );
    event.Skip();
}

void wxComboFrameEventHandler::OnClose( wxCloseEvent& event )
{
    m_combo->HidePopup( false );
    event.Skip();
}

void wxComboFrameEventHandler::OnActivate( wxActivateEvent& event )
{
    m_combo->HidePopup( false );
    event.Skip();
}

void wxComboFrameEventHandler::OnResize( wxSizeEvent& event )
{
    m_combo->HidePopup( false );
    event.Skip();
}

void wxComboFrameEventHandler::OnMove( wxMoveEvent& event )
{
    m_combo->HidePopup( false );
    event.Skip();
}

#endif // wxODC_INSTALL_TOPLEV_HANDLER

// ----------------------------------------------------------------------------
// wxComboPopupWindow is wxPopupWindow customized for
// wxPGOwnerDrawnComboBox.
// ----------------------------------------------------------------------------

class wxComboPopupWindow : public wxComboPopupWindowBase
{
public:

    wxComboPopupWindow ( wxPGOwnerDrawnComboBox *parent, int style = wxBORDER_NONE );

#if wxODC_USE_TRANSIENT_POPUP
    virtual bool ProcessLeftDown(wxMouseEvent& event);
#endif

    void OnMouseEvent( wxMouseEvent& event );
#if !wxUSE_POPUPWIN
    void OnActivate( wxActivateEvent& event );
#endif

protected:

#if wxODC_USE_TRANSIENT_POPUP 
    virtual void OnDismiss();
#endif

private:
    DECLARE_EVENT_TABLE()
};

BEGIN_EVENT_TABLE(wxComboPopupWindow, wxComboPopupWindowBase)
    EVT_MOUSE_EVENTS(wxComboPopupWindow::OnMouseEvent)
#if !wxUSE_POPUPWIN
    EVT_ACTIVATE(wxComboPopupWindow::OnActivate)
#endif
END_EVENT_TABLE()

void wxComboPopupWindow::OnMouseEvent ( wxMouseEvent& event )
{
    //wxLogDebug(wxT("wxComboPopupWindow::OnMouseEvent(%i)"),(int)event.GetEventType());
    event.Skip();
}

#if !wxUSE_POPUPWIN
void wxComboPopupWindow::OnActivate( wxActivateEvent& event )
{
    if ( !event.GetActive() )
    {
        // Tell combo control that we are dismissed.
        wxPGOwnerDrawnComboBox* combo = (wxPGOwnerDrawnComboBox*) GetParent();
        wxASSERT ( combo );
        wxASSERT ( combo->IsKindOf(CLASSINFO(wxPGOwnerDrawnComboBox)) );

        combo->HidePopup( false );

        //if ( ::wxFindWindowAtPoint(::wxGetMousePosition()) == (wxWindow*) combo->m_btn )
        //    combo->m_ignoreNextButtonClick = 1;

        event.Skip();
    }
}
#endif

wxComboPopupWindow::wxComboPopupWindow (wxPGOwnerDrawnComboBox *parent,
                                        int style)
#if wxUSE_POPUPWIN
                                       : wxComboPopupWindowBase(parent,style)
#else
                                       : wxComboPopupWindowBase(parent,
                                                                wxID_ANY,
                                                                wxEmptyString,
                                                                wxPoint(-21,-21),
                                                                wxSize(20,20),
                                                                style)
#endif
{
}

#if wxODC_USE_TRANSIENT_POPUP
bool wxComboPopupWindow::ProcessLeftDown(wxMouseEvent& event )
{
    //wxLogDebug(wxT("wxComboPopupWindow::ProcessLeftDown(%i,%i)"),event.m_x,event.m_y);
    //return false;
    return wxComboPopupWindowBase::ProcessLeftDown(event);
}
#endif

#if wxODC_USE_TRANSIENT_POPUP 
// First thing that happens when a transient popup closes is that this method gets called.
void wxComboPopupWindow::OnDismiss()
{
    wxPGOwnerDrawnComboBox* combo = (wxPGOwnerDrawnComboBox*) GetParent();
    wxASSERT ( combo->IsKindOf(CLASSINFO(wxPGOwnerDrawnComboBox)) );

    //wxLogDebug(wxT("wxComboPopupWindow::OnDismiss()"));

    combo->OnPopupDismiss();

    /*if ( !(::wxFindWindowAtPoint(::wxGetMousePosition()) == (wxWindow*) combo->m_btn) )
    {
        ((wxComboDropButton*)combo->m_btn)->SetPopup( (wxWindow*) NULL );
        combo->m_isPopupShown = 0;
    }
    else
    {
        combo->OnPopupDismiss();
        combo->m_ignoreNextButtonClick = 1;
    }*/

}
#endif

// ----------------------------------------------------------------------------

BEGIN_EVENT_TABLE(wxComboPopupInterface, wxEvtHandler)
#if !wxODC_USE_TRANSIENT_POPUP
    // This is used for debugging only
    EVT_MOUSE_EVENTS(wxComboPopupInterface::OnMouseEvent)
#endif
END_EVENT_TABLE()

wxComboPopupInterface::wxComboPopupInterface()
{
    m_callback = (wxComboPaintCallback) NULL;
}

wxComboPopupInterface::~wxComboPopupInterface()
{
    wxWindow* popup = m_combo->GetPopupControl();
    if ( popup )
        popup->RemoveEventHandler(this);
}

bool wxComboPopupInterface::Init( wxPGOwnerDrawnComboBox* combo )
{
    m_combo = combo;

    return true; // default is to generate popup immediately
}

void wxComboPopupInterface::Populate ( int n, const wxString choices[], void **data ) //maks
{

}

bool wxComboPopupInterface::IsHighlighted ( int ) const
{
    return false;
}

const int* wxComboPopupInterface::GetIntPtr() const
{
    return (const int*) NULL;
}

void wxComboPopupInterface::Clear()
{
}

void wxComboPopupInterface::Delete( int WXUNUSED(item) )
{
}

int wxComboPopupInterface::FindString(const wxString&) const
{
    return wxNOT_FOUND;
}

int wxComboPopupInterface::GetCount() const
{
    return 0;
}

wxString wxComboPopupInterface::GetString( int ) const
{
    return wxEmptyString;
}

void* wxComboPopupInterface::GetClientData( int item ) const //maks
{
	return NULL;
}

void wxComboPopupInterface::Insert( const wxString& WXUNUSED(item),
                                    int WXUNUSED(pos) )
{
}

void wxComboPopupInterface::SetSelection ( int WXUNUSED(item) )
{
}

void wxComboPopupInterface::SetString( int, const wxString& )
{
}

void wxComboPopupInterface::SetValueFromString ( const wxString& WXUNUSED(value) )
{
}

#if !wxODC_USE_TRANSIENT_POPUP
// This is used for debugging only
void wxComboPopupInterface::OnMouseEvent ( wxMouseEvent& event )
{
    //if ( m_handledWnd)
    //wxLogDebug(wxT("wxComboPopupInterface::OnMouseEvent(%i)"),(int)event.GetEventType());
    event.Skip();
}
#endif

// ----------------------------------------------------------------------------
// wxPGVListBoxComboPopup is a wxVListBox customized to act as a popup control
// ----------------------------------------------------------------------------

class wxVListBoxComboInterface; 

class wxPGVListBoxComboPopup : public wxVListBox //maks: wxPGVListBoxComboPopup -> wxVListBoxComboPopup (wxVListBoxComboPopup already defined in the new wx)
{
public:

    // ctor and dtor
    wxPGVListBoxComboPopup(wxWindow* parent, wxVListBoxComboInterface* iface,
        wxComboPaintCallback callback, int style = 0);
    virtual ~wxPGVListBoxComboPopup();

    inline int GetItemAtPosition( const wxPoint& pos ) { return HitTest(pos); }

    inline wxCoord GetTotalHeight() const { return EstimateTotalHeight(); }

    inline wxCoord GetLineHeight(int line) const { return OnGetLineHeight(line); }

protected:

    // wxVListBox implementation
    virtual void OnDrawItem(wxDC& dc, const wxRect& rect, size_t n) const;
    virtual wxCoord OnMeasureItem(size_t n) const;
    void OnDrawBackground(wxDC& dc, const wxRect& rect, size_t n) const;

    void SendSelectedEvent();

    // filter mouse move events happening outside the list box
    // move selection with cursor
    void OnMouseMove(wxMouseEvent& event);
    void OnMouseWheel(wxMouseEvent& event);
    void OnKey(wxKeyEvent& event);
    void OnLeftClick(wxMouseEvent& event);

    wxVListBoxComboInterface*   m_iface;

    wxComboPaintCallback        m_callback;

private:

    // has the mouse been released on this control?
    bool m_clicked;

    DECLARE_EVENT_TABLE()
};


class wxTreeComboInterface; //maks
class wxTreePopup : public wxTreeCtrl //maks
{
public:

    // ctor and dtor
    wxTreePopup(wxWindow* parent, wxTreeComboInterface* iface, wxComboPaintCallback callback, int style = 0);
    virtual ~wxTreePopup();

	void Init(wxWindow* parent, wxTreeComboInterface* iface, wxComboPaintCallback callback, int style = 0);
    inline int GetItemAtPosition( const wxPoint& pos ) { return HitTest(pos); }

    inline wxCoord GetTotalHeight() { return (GetChildrenCount(GetRootItem(), false) + 1)*GetLineHeight(0); } 

    inline wxCoord GetLineHeight(int line) const 
	{ 
		int x, y = 8;
		GetTextExtent("A", &x, &y, 0, 0, &GetFont());
		return y; 
	} 

	wxTreeItemId AppendItem(const wxTreeItemId& parent, const wxString& text, int image = -1, int selImage = -1, wxTreeItemData* data = NULL);
	void SelectItem(wxString value);
	wxTreeItemId FindString(wxTreeItemId &root, wxString &value);

	void UpdateTree();

protected:

    virtual void OnDrawItem(wxDC& dc, const wxRect& rect, size_t n) const;
    virtual wxCoord OnMeasureItem(size_t n) const;
    void OnDrawBackground(wxDC& dc, const wxRect& rect, size_t n) const;

	void OnPaint(wxPaintEvent& event);

    void SendSelectedEvent();
	void Unselect();
	void Populate();

    // filter mouse move events happening outside the list box
    // move selection with cursor
    void OnMouseMove(wxMouseEvent& event);
    void OnMouseWheel(wxMouseEvent& event);
    void OnKey(wxKeyEvent& event);
    void OnLeftClickUp(wxMouseEvent& event);
	void OnLeftClickDown(wxMouseEvent& event);
	

	
	void OnSeletionChanging(wxTreeEvent& event);
	void OnSeletionChanged(wxTreeEvent& event);

	void OnItemExpanding(wxTreeEvent& event);
	void OnItemCollapsing(wxTreeEvent& event);

	

    wxTreeComboInterface*   m_iface;

    wxComboPaintCallback        m_callback;

private:

    // has the mouse been released on this control?
    bool m_clicked;
	bool bCanSelectRootChild;
	bool bCheckSelection, bOpenCloseTree;
	wxPoint lastMouseClick;

    DECLARE_EVENT_TABLE()
};



// ----------------------------------------------------------------------------

class wxTreeComboInterface : public wxComboPopupInterface //maks
{
    friend class wxTreePopup;
public:

    wxTreeComboInterface ( wxComboPaintCallback callback );
    virtual ~wxTreeComboInterface ();

    bool Init( wxPGOwnerDrawnComboBox* combo );

    virtual void Insert( const wxString& item, int pos );
    virtual void Clear();
    virtual void Delete( int item );
    virtual int FindString(const wxString& s) const;
    virtual int GetCount() const;
    virtual wxString GetString( int item ) const;
    virtual wxString GetValueAsString() const;
    virtual void SetSelection ( int item );
    virtual void SetString( int item, const wxString& str );
    virtual void SetValueFromString ( const wxString& value );

    virtual wxWindow* GeneratePopup( wxWindow* parent, int minWidth,
                                     int maxHeight, int prefHeight );

    virtual bool IsHighlighted ( int item ) const;

    virtual const int* GetIntPtr () const;

    void Populate ( int n, const wxString choices[], void **data = NULL ); //maks

	void* GetClientData( int item ) const; //maks
	void SetClientData( int item, void* clientData ); //maks

protected:

    // Event handlers
    void OnMouseMove ( wxMouseEvent& event );
    void OnKey (wxKeyEvent& event);
    void OnSelect (wxCommandEvent& event);

    void CheckWidth( const wxString& item );

    wxTreePopup*   m_popup;
	wxArrayString           m_strings;
	VoidPtrArray m_datas; //maks



    wxFont                  m_font;


    int                     m_widestWidth; // width of widest item thus far

    int                     m_avgCharWidth;

    int                     m_baseImageWidth; // how much per item drawn in addition to text

private:
    DECLARE_EVENT_TABLE()
};

// ----------------------------------------------------------------------------
// wxVListBoxComboInterface is the "default" wxPGOwnerDrawnComboBox combo
// interface. It uses wxPGVListBoxComboPopup as the popup control.
// ----------------------------------------------------------------------------

class wxVListBoxComboInterface : public wxComboPopupInterface
{
    friend class wxPGVListBoxComboPopup;
public:

    wxVListBoxComboInterface ( wxComboPaintCallback callback );
    virtual ~wxVListBoxComboInterface ();

    bool Init( wxPGOwnerDrawnComboBox* combo );

    virtual void Insert( const wxString& item, int pos );
    virtual void Clear();
    virtual void Delete( int item );
    virtual int FindString(const wxString& s) const;
    virtual int GetCount() const;
    virtual wxString GetString( int item ) const;
    virtual wxString GetValueAsString() const;
    virtual void SetSelection ( int item );
    virtual void SetString( int item, const wxString& str );
    virtual void SetValueFromString ( const wxString& value );

    virtual wxWindow* GeneratePopup( wxWindow* parent, int minWidth,
                                     int maxHeight, int prefHeight );

    virtual bool IsHighlighted ( int item ) const;

    virtual const int* GetIntPtr () const;

    void Populate ( int n, const wxString choices[], void **data = NULL ); //maks

	void* GetClientData( int item ) const; //maks
	void SetClientData( int item, void* clientData ); //maks

protected:

    // Event handlers
    void OnMouseMove ( wxMouseEvent& event );
    void OnKey (wxKeyEvent& event);
    void OnSelect (wxCommandEvent& event);

    void CheckWidth( int pos, const wxString& item );

    wxPGVListBoxComboPopup*   m_popup;

    wxArrayString           m_strings;
	VoidPtrArray m_datas; //maks

    wxFont                  m_font;

    int                     m_value;

    int                     m_widestWidth; // width of widest item thus far

    int                     m_avgCharWidth;

    int                     m_baseImageWidth; // how much per item drawn in addition to text

private:
    DECLARE_EVENT_TABLE()
};

// ----------------------------------------------------------------------------

BEGIN_EVENT_TABLE(wxPGVListBoxComboPopup, wxVListBox)
    EVT_MOTION(wxPGVListBoxComboPopup::OnMouseMove)
    EVT_KEY_DOWN(wxPGVListBoxComboPopup::OnKey)
    EVT_LEFT_UP(wxPGVListBoxComboPopup::OnLeftClick)
END_EVENT_TABLE()

wxPGVListBoxComboPopup::wxPGVListBoxComboPopup(wxWindow *parent,
                                           wxVListBoxComboInterface* iface,
                                           wxComboPaintCallback callback,
                                           int style)
              : wxVListBox(parent, wxID_ANY,
                          wxDefaultPosition, wxDefaultSize,
                          wxBORDER_SIMPLE | wxLB_INT_HEIGHT | style )
{
    m_iface = iface;
    m_callback = callback;

    wxASSERT ( GetParent()->GetParent() );
    SetFont( GetParent()->GetParent()->GetFont() );

}

wxPGVListBoxComboPopup::~wxPGVListBoxComboPopup()
{
}

void wxPGVListBoxComboPopup::OnDrawItem(wxDC& dc, const wxRect& rect, size_t n) const
{
    dc.SetFont( m_font );

    wxPGOwnerDrawnComboBox* pCb = m_iface->m_combo;

    bool is_hilited = pCb->IsHighlighted(n);

    // Set correct text colour for selected items
    if ( is_hilited )
        dc.SetTextForeground ( wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHTTEXT) );

    ((pCb->GetParent())->*m_callback)(pCb,
        n,dc,((wxRect&)rect),0);

    // Restore text colour
    if ( is_hilited )
        dc.SetTextForeground ( wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOWTEXT) );
}

wxCoord wxPGVListBoxComboPopup::OnMeasureItem(size_t n) const
{
    wxRect rect(-1,-1,-1,-1);
    wxDC* invalid_dc_ptr;
#ifdef __WXDEBUG__
    // TODO: Change this to something safe.
    invalid_dc_ptr = (wxDC*) NULL;
#else
    invalid_dc_ptr = (wxDC*) NULL;
#endif
    ((m_iface->m_combo->GetParent())->*m_callback)(m_iface->m_combo,
        n,*invalid_dc_ptr,rect,0);
    wxASSERT_MSG ( rect.height >= 0, wxT("wxPGOwnerDrawnComboBox measure item call didn't return valid value in rect.height") );
    return rect.height;
}

extern void wxPGDrawFocusRect( wxDC&dc, const wxRect& rect );

static void DrawComboSelectionBackground( wxDC& dc, const wxRect& rect )
{
    wxColour sel_col = wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHT);
    dc.SetBrush ( sel_col );
    dc.SetPen ( sel_col );
    dc.DrawRectangle ( rect );

#if wxODC_SELECTION_STYLE == 0
    // Draw dotted edge (use sys_hilight_text colour)
    wxPGDrawFocusRect(dc,rect);
#endif

}

void wxPGVListBoxComboPopup::OnDrawBackground(wxDC& dc, const wxRect& rect, size_t n) const
{
    // we need to render selected and current items differently
    if ( IsCurrent(n) )
    {
        DrawComboSelectionBackground( dc, rect );
        /*dc.SetBrush( wxBrush(GetSelectionBackground(), wxSOLID) );

        dc.SetPen( *wxTRANSPARENT_PEN );

        dc.DrawRectangle(rect);*/
    }
    //else: do nothing for the normal items
}

void wxPGVListBoxComboPopup::SendSelectedEvent()
{
    wxCommandEvent event(wxEVT_COMMAND_COMBOBOX_SELECTED, GetId());
    event.SetEventObject(this);
    event.SetInt(GetSelection());

    (void)GetEventHandler()->ProcessEvent(event);
}

void wxPGVListBoxComboPopup::OnMouseMove(wxMouseEvent& event)
{
    int item_here = GetItemAtPosition(event.GetPosition());
    if ( item_here >= 0 )
        SetSelection(item_here);
    event.Skip();
}

void wxPGVListBoxComboPopup::OnLeftClick(wxMouseEvent& event)
{
    wxPoint pt = event.GetPosition();
    wxSize sz = GetClientSize();
    if ( pt.x >= 0 && pt.y >= 0 && pt.x < sz.x && pt.y < sz.y )
        wxPGVListBoxComboPopup::SendSelectedEvent();
    else
        event.Skip();
}

void wxPGVListBoxComboPopup::OnKey(wxKeyEvent& event)
{
    if ( event.GetKeyCode() == WXK_RETURN || event.GetKeyCode() == WXK_NUMPAD_ENTER )
        wxPGVListBoxComboPopup::SendSelectedEvent();
    else
        event.Skip();
}



// ----------------------------------------------------------------------------
//maks wxTreePopup

BEGIN_EVENT_TABLE(wxTreePopup, wxTreeCtrl)
    EVT_MOTION(wxTreePopup::OnMouseMove)
    EVT_KEY_DOWN(wxTreePopup::OnKey)
    EVT_LEFT_UP(wxTreePopup::OnLeftClickUp)
	EVT_LEFT_DOWN(wxTreePopup::OnLeftClickDown)
	EVT_TREE_ITEM_EXPANDING(wxID_ANY, wxTreePopup::OnItemExpanding)
	EVT_TREE_ITEM_COLLAPSING(wxID_ANY, wxTreePopup::OnItemCollapsing)
	EVT_TREE_SEL_CHANGING(wxID_ANY, wxTreePopup::OnSeletionChanging)
	EVT_TREE_SEL_CHANGED(wxID_ANY, wxTreePopup::OnSeletionChanged)
	EVT_PAINT(wxTreePopup::OnPaint)
END_EVENT_TABLE()




wxTreePopup::wxTreePopup(wxWindow *parent,
                                           wxTreeComboInterface* iface,
                                           wxComboPaintCallback callback,
                                           int style)
              : wxTreeCtrl(parent, wxID_ANY,
                          wxDefaultPosition, wxDefaultSize,
                          wxTR_HAS_BUTTONS | wxTR_SINGLE | wxTR_HIDE_ROOT | wxTR_LINES_AT_ROOT | wxBORDER | style ) 
{
    Init(parent, iface, callback, style);
}

void wxTreePopup::Init(wxWindow *parent, wxTreeComboInterface* iface, wxComboPaintCallback callback, int style)
{
    m_iface = iface;
    m_callback = callback;
	bCheckSelection = true;
	bOpenCloseTree = false;
	bCanSelectRootChild = true;

    SetFont( GetParent()->GetParent()->GetFont() );
	GetParent()->SetWindowStyle(parent->GetWindowStyle() | wxRESIZE_BORDER);

	Populate();
	SelectItem(m_iface->GetCombo()->GetValueString());
}

wxTreePopup::~wxTreePopup()
{
}

void wxTreePopup::UpdateTree() //maks
{
	if(!GetCount())
	{
		Populate();
	}
}

void wxTreePopup::Populate()
{
	if(!m_iface) return;

	int i;
	wxTreeItemId rootId = AddRoot(wxT(""));

	for(i = 0; i < m_iface->m_strings.Count(); i++)
	{
		wxString s(m_iface->m_strings[i]);
		
		wxStringTokenizer tkz(s, wxT(","));
		int n = 0;
		wxTreeItemId lastItem = rootId;


		while ( tkz.HasMoreTokens() )
		{
			wxString token(tkz.GetNextToken());

			wxTreeItemId itemId = FindString(lastItem, token);

			if(!itemId.IsOk())
			{
				itemId = AppendItem(lastItem, token);
				SetItemData(itemId, new VoidPtrTreeData(m_iface->m_datas[i])); //maks
			}
			
			if(n == 0) lastItem = itemId;
			n++;
		}		
	}	
}

wxTreeItemId wxTreePopup::AppendItem(const wxTreeItemId& parent, const wxString& text, int image, int selImage, wxTreeItemData* data)
{
	if(m_iface) m_iface->CheckWidth(text);
	return wxTreeCtrl::AppendItem(parent, text, image, selImage, data);
}

//Call back for images
int (*OnGetItemImage)(wxTreeCtrl *pTree, wxTreeItemId item) = NULL;

void wxTreePopup::OnPaint(wxPaintEvent& event)
{
    wxTreeItemId h;

    for (h=GetFirstVisibleItem(); h.IsOk();	h=GetNextVisible(h)) 
    {
        if (!IsVisible(h))
            break;

		int image = GetItemImage(h);
		if(image == -1 && OnGetItemImage) //wxWidgets return 0 even if there is no image in the item
		{
			image = OnGetItemImage(this, h);

			if(image >= 0)
			{
				SetItemImage(h, image, wxTreeItemIcon_Normal);
				SetItemImage(h, image, wxTreeItemIcon_Selected);
			}
		}
    }

	event.Skip();
}

void wxTreePopup::OnDrawItem(wxDC& dc, const wxRect& rect, size_t n) const
{
  
}

wxCoord wxTreePopup::OnMeasureItem(size_t n) const
{
	return 16;
}


void wxTreePopup::OnDrawBackground(wxDC& dc, const wxRect& rect, size_t n) const
{

}

void wxTreePopup::SendSelectedEvent()
{
	wxTreeItemId item = GetSelection();

	if(item.IsOk()) //maks
	{
		wxCommandEvent event(wxEVT_COMMAND_COMBOBOX_SELECTED, GetId());
		event.SetEventObject(this);	
		event.SetString(GetItemText(item));

		VoidPtrTreeData *itemData = (VoidPtrTreeData *)GetItemData(item); //maks
		if(itemData) event.SetClientData(itemData->data); //maks

		(void)GetEventHandler()->ProcessEvent(event);
	}
}

void wxTreePopup::OnMouseMove(wxMouseEvent& event)
{
    int item_here = GetItemAtPosition(event.GetPosition());

    event.Skip();
}

void wxTreePopup::OnLeftClickUp(wxMouseEvent& event)
{
    wxPoint pt = event.GetPosition();
    wxSize sz = GetClientSize();

	lastMouseClick = wxPoint(0, 0);

	if(!bOpenCloseTree && GetSelection().IsOk())
	{
		if ( pt.x >= 0 && pt.y >= 0 && pt.x < sz.x && pt.y < sz.y )
			wxTreePopup::SendSelectedEvent();
		else
			event.Skip();
	}
	else if(!bOpenCloseTree)
	{
		wxTreePopup::SendSelectedEvent();
	}
}

void wxTreePopup::OnLeftClickDown(wxMouseEvent& event)
{
	bOpenCloseTree = false;
	lastMouseClick = event.GetPosition();
	event.Skip();
}

void wxTreePopup::OnItemExpanding(wxTreeEvent& event)
{
	bOpenCloseTree = true;
	
	wxRect rect = GetParent()->GetRect(), rectfix;
	bool bFixMove = false;

	if(GetParent()->IsTopLevel() && GetParent()->GetParent()) //maks
	{
		bFixMove = true;
		rectfix = GetParent()->GetParent()->GetRect();
		rect.SetPosition(GetParent()->ScreenToClient(GetParent()->GetRect().GetPosition()));
	}
	else
	{
		rect.SetPosition(GetParent()->ClientToScreen(GetParent()->GetRect().GetPosition()));
	}

	int newHeight = (GetChildrenCount(event.GetItem(), false) + 1)*GetLineHeight(0) + rect.GetHeight(); //maks
	if(newHeight > rect.GetHeight())
	{
		int screenY = wxSystemSettings::GetMetric(wxSYS_SCREEN_Y); //maks
		/*if(rect.y < GetParent()->GetParent()->GetRect().y)
		{
			//Move up
			GetParent()->Move(rect.x, rect.y - (newHeight - rect.GetHeight()));
		}*/

		/*if(rect.y + newHeight > screenY)
		{
			//Don't grow bellow screen
			newHeight = screenY - rect.y - 1;
		}*/

		if(newHeight > rect.GetHeight())
		{
			//Set sizes
			GetParent()->SetSize( rect.GetWidth(), newHeight);
			if(bFixMove) 
				GetParent()->Move(rect.x, rect.y + rectfix.GetHeight());

			SetSize( rect.GetWidth(), newHeight);
		}

		EnsureVisible(event.GetItem());
	}
}

void wxTreePopup::OnItemCollapsing(wxTreeEvent& event) //maks
{
	bOpenCloseTree = true;

	wxRect rect = GetParent()->GetRect(), rectfix;
	bool bFixMove = false;

	if(GetParent()->IsTopLevel() && GetParent()->GetParent()) //maks
	{
		bFixMove = true;
		rectfix = GetParent()->GetParent()->GetRect();
		rect.SetPosition(GetParent()->ScreenToClient(GetParent()->GetRect().GetPosition()));
	}
	else
	{
		rect.SetPosition(GetParent()->ClientToScreen(GetParent()->GetRect().GetPosition()));
	}

	int newHeight = rect.GetHeight() - (GetChildrenCount(event.GetItem(), false) + 1)*GetLineHeight(0); //maks
	if(newHeight < rect.GetHeight())
	{
		int screenY = wxSystemSettings::GetMetric(wxSYS_SCREEN_Y); //maks
		/*if(rect.y < GetParent()->GetParent()->GetRect().y)
		{
			//Move up
			GetParent()->Move(rect.x, rect.y - (newHeight - rect.GetHeight()));
		}*/

		/*if(rect.y + newHeight > screenY)
		{
			//Don't grow bellow screen
			newHeight = screenY - rect.y - 1;
		}*/

		if(newHeight < rect.GetHeight())
		{
			//Set sizes
			GetParent()->SetSize( rect.GetWidth(), newHeight);
			if(bFixMove) 
				GetParent()->Move(rect.x, rect.y + rectfix.GetHeight());

			SetSize( rect.GetWidth(), newHeight);
		}

		EnsureVisible(event.GetItem());
	}
}

void wxTreePopup::Unselect()
{
	bCheckSelection = false;
	wxTreeCtrl::Unselect();
	bCheckSelection = true;
}

void wxTreePopup::SelectItem(wxString value)
{
	if(value.IsEmpty())
	{
		Unselect();
		return;
	}

	bCheckSelection = false;
	
	wxTreeItemId itemId = FindString(GetRootItem(), value);

	if(itemId.IsOk())
	{
		wxTreeCtrl::SelectItem(itemId);
	}

	bCheckSelection = true;
}

wxTreeItemId wxTreePopup::FindString(wxTreeItemId &root, wxString &value)
{
	wxTreeItemId itemId;
	wxTreeItemIdValue cookie;
	
	if(root != GetRootItem() && GetItemText(root) == value)
	{
		return root;
	}

    wxTreeItemId child = GetFirstChild (root, cookie);
    while(child.IsOk()) 
	{
		itemId = FindString(child, value);
        if(itemId.IsOk())
		{
			return itemId;
		}

        child = GetNextChild (itemId, cookie);
    }

	return itemId;
}

void wxTreePopup::OnSeletionChanging(wxTreeEvent& event)
{
	if(bCheckSelection)
	{
		wxRect rect;
		if(!(GetBoundingRect(event.GetItem(), rect, false) && rect.Inside(lastMouseClick)))
		{	
			event.Veto();
		}
	}
}

void wxTreePopup::OnSeletionChanged(wxTreeEvent& event)
{
	if(event.GetItem().IsOk())
	{
		if(bCanSelectRootChild || GetItemParent(event.GetItem()) != GetRootItem())
		{
			wxTreePopup::SendSelectedEvent();
		}
	}

	event.Skip();
}

void wxTreePopup::OnKey(wxKeyEvent& event)
{
    if ( event.GetKeyCode() == WXK_RETURN || event.GetKeyCode() == WXK_NUMPAD_ENTER )
	{
        wxTreePopup::SendSelectedEvent();
	}
	else if ( event.GetKeyCode() == WXK_ESCAPE )
	{
		Unselect();
		wxTreePopup::SendSelectedEvent();
	}
    else
	{
        event.Skip();
	}
}

// ----------------------------------------------------------------------------
// wxVListBoxComboInterface is the "default" wxOwnerDrawComboBox combo
// interface. It uses wxPGVListBoxComboPopup as the popup control.
// ----------------------------------------------------------------------------

BEGIN_EVENT_TABLE(wxVListBoxComboInterface, wxEvtHandler)
    EVT_COMBOBOX(wxID_ANY,wxVListBoxComboInterface::OnSelect)
    //EVT_KEY_DOWN(wxVListBoxComboInterface::OnKey)
    //EVT_KEY_UP(wxVListBoxComboInterface::OnKey)
    //EVT_LEFT_UP(wxVListBoxComboInterface::OnMouseMove)
END_EVENT_TABLE()

wxVListBoxComboInterface::wxVListBoxComboInterface ( wxComboPaintCallback callback )
    : wxComboPopupInterface()
{
    m_callback = callback;
    m_popup = (wxPGVListBoxComboPopup*) NULL;
    m_widestWidth = 0;
    m_avgCharWidth = 0;
    m_baseImageWidth = 0;
}

wxVListBoxComboInterface::~wxVListBoxComboInterface ()
{
    Clear();
}

bool wxVListBoxComboInterface::Init( wxPGOwnerDrawnComboBox* combo )
{
    wxComboPopupInterface::Init(combo);

    return false; // generate on first popup
}

void wxVListBoxComboInterface::OnSelect (wxCommandEvent& event)
{
    m_value = m_popup->GetSelection();
    m_combo->HidePopup(true, &event); //maks
}

void wxVListBoxComboInterface::OnMouseMove ( wxMouseEvent& event )
{
    int type = event.GetEventType();
    if ( type == wxEVT_LEFT_UP )
    {
        m_value = m_popup->GetSelection();
        m_combo->HidePopup();
    }
    event.Skip();
}

void wxVListBoxComboInterface::OnKey(wxKeyEvent& event)
{
    if ( event.GetKeyCode() == WXK_RETURN )
    {
        m_value = m_popup->GetSelection();
        m_combo->HidePopup();
    }
    else
        event.Skip();
}

void wxVListBoxComboInterface::CheckWidth( int pos, const wxString& item )
{

    // Calculate average character width
    if ( !m_avgCharWidth )
    {
        wxASSERT ( m_combo );
        m_font = m_combo->GetFont();
        wxClientDC dc(m_combo);
        dc.SetFont(m_font);
        m_avgCharWidth = (dc.GetCharWidth()*3)/2 + 1;
    }

    // Get usual custom image width
    if ( !m_baseImageWidth )
    {
        wxASSERT( GetCount() > 0 );
        wxRect rect(-1,-1,0,0);
        wxDC* invalid_dc_ptr;
    #ifdef __WXDEBUG__
        // TODO: Change this to something safe.
        invalid_dc_ptr = (wxDC*) NULL;
    #else
        invalid_dc_ptr = (wxDC*) NULL;
    #endif
        ((m_combo->GetParent())->*m_callback)(m_combo,
                                              pos,
                                              *invalid_dc_ptr,
                                              rect,
                                              0);
        m_baseImageWidth = rect.width;
    }

    int baseWidth = (m_avgCharWidth * item.length()) + m_baseImageWidth;
    if ( m_widestWidth < baseWidth )
    {
        // Need to check the actual width
        int x, y;
        m_combo->GetTextExtent(item, &x, &y, 0, 0, &m_font);
        int actualWidth = x + m_baseImageWidth;
        if ( m_widestWidth < actualWidth )
        {
            //wxLogDebug(wxT("%i"),m_baseImageWidth);
            //wxLogDebug(wxT("%s: %i"),item.c_str(),actualWidth);
            m_widestWidth = actualWidth;
        }
        //wxLogDebug(wxT("%i (%i,%i)"),m_widestWidth,(int)m_avgCharWidth,(int)item.length());
    }
}

void wxVListBoxComboInterface::Insert( const wxString& item, int pos )
{
    m_strings.Insert(item,pos);

    // Calculate width
    CheckWidth(pos,item);

    if ( m_popup )
        m_popup->SetItemCount(m_popup->GetItemCount()+1);
}

void wxVListBoxComboInterface::Clear()
{
    wxASSERT ( m_combo );

    m_strings.Empty();

    if ( m_popup )
        m_popup->SetItemCount(0);
}

void wxVListBoxComboInterface::Delete( int item )
{
    wxASSERT ( m_combo );

    m_strings.RemoveAt(item);

    /*
    if ( m_combo->HasClientObjectData() )
        delete (wxClientData *) m_datas[item];*/

    m_datas.RemoveAt(item); //maks
    

    if ( m_popup )
        m_popup->SetItemCount(m_popup->GetItemCount()-1);
}

int wxVListBoxComboInterface::FindString(const wxString& s) const
{
    return m_strings.Index(s);
}

void* wxVListBoxComboInterface::GetClientData( int item ) const //maks
{
	if(item >= 0 && item < m_strings.GetCount())
	{
		return m_datas[item];
	}

	return NULL;
}

int wxVListBoxComboInterface::GetCount() const
{
    return m_strings.GetCount();
}

wxString wxVListBoxComboInterface::GetString( int item ) const //maks
{
	if(item >= 0 && item < m_strings.GetCount())
	{
		return m_strings[item];
	}

	return "";
}

wxString wxVListBoxComboInterface::GetValueAsString() const
{
    if ( m_value >= 0 )
        return m_strings[m_value];
    return wxEmptyString;
}

void wxVListBoxComboInterface::SetClientData( int item,
                                              void* clientData )
{
    m_datas[item] = clientData;
}

void wxVListBoxComboInterface::SetSelection ( int item )
{
    // This seems to be necessary (2.5.3 w/ MingW atleast)
    if ( item < -1 || item >= (int)m_strings.GetCount() )
        item = -1;

    m_value = item;

    if ( m_popup )
    {
            m_popup->SetSelection(item);
    }
}

void wxVListBoxComboInterface::SetString( int item, const wxString& str )
{
    m_strings[item] = str;
}

void wxVListBoxComboInterface::SetValueFromString ( const wxString& value )
{
    int index = m_strings.Index(value);
    if ( index != wxNOT_FOUND )
        SetSelection(index);
}

wxWindow* wxVListBoxComboInterface::GeneratePopup(wxWindow* parent, int minWidth,
                                                  int maxHeight, int prefHeight )
{
    if ( parent )
    {
        m_popup = new wxPGVListBoxComboPopup(parent,this,m_callback,0);
        m_popup->SetItemCount(m_strings.GetCount());
    }

    int height = 250;

    if ( m_strings.GetCount() )
    {
        if ( prefHeight > 0 )
            height = prefHeight;

        if ( height > maxHeight )
            height = maxHeight;

        int total_height = m_popup->GetTotalHeight() + 3;
        if ( height >= total_height )
        {
            height = total_height;
        }
        else
        {
        // Adjust height to a multiple of the height of the first item
            int fih = m_popup->GetLineHeight ( 0 );
            int shown = height/fih;
            height = shown * fih;
        }
    }
    else
        height = 50;

    // Take scrollbar into account in width calculations
    int widestWidth = m_widestWidth + wxSystemSettings::GetMetric(wxSYS_VSCROLL_X); //27;
    m_popup->Move(0,0);
    m_popup->SetSize( minWidth > widestWidth ? minWidth : widestWidth,
                     height+2);

    // *must* set value after size is set (this is because of a vlbox bug)
    if ( m_value >= 0 )
        SetSelection(m_value);

    return m_popup;
}

void wxVListBoxComboInterface::Populate ( int n, const wxString choices[], void **data )  //maks
{
	m_strings.Clear();
	m_datas.SetCount(n);

    int i;
    for ( i=0; i<n; i++ )
    {
        m_strings.Add(choices[i]);
        CheckWidth(n,choices[i]);
		if(data) m_datas[i] = data[i];
    }

}

const int* wxVListBoxComboInterface::GetIntPtr () const
{
    return &m_value;
}

bool wxVListBoxComboInterface::IsHighlighted ( int item ) const
{
    if ( m_popup->GetParent()->IsShown() )
    {
        return m_popup->IsCurrent( item );
    }
    return ( m_value == item );
}


// ----------------------------------------------------------------------------
// maks: wxTreeComboInterface
// ----------------------------------------------------------------------------

BEGIN_EVENT_TABLE(wxTreeComboInterface, wxEvtHandler)
    EVT_COMBOBOX(wxID_ANY,wxTreeComboInterface::OnSelect)
    //EVT_KEY_DOWN(wxTreeComboInterface::OnKey)
    //EVT_KEY_UP(wxTreeComboInterface::OnKey)
    //EVT_LEFT_UP(wxTreeComboInterface::OnMouseMove)
END_EVENT_TABLE()

wxTreeComboInterface::wxTreeComboInterface ( wxComboPaintCallback callback )
    : wxComboPopupInterface()
{
    m_callback = callback;
    m_popup = (wxTreePopup*) NULL;
    m_widestWidth = 0;
    m_avgCharWidth = 0;
    m_baseImageWidth = 0;
}

wxTreeComboInterface::~wxTreeComboInterface ()
{
    Clear();
}

bool wxTreeComboInterface::Init( wxPGOwnerDrawnComboBox* combo )
{
    wxComboPopupInterface::Init(combo);

    return false; // generate on first popup
}

void wxTreeComboInterface::OnSelect (wxCommandEvent& event)
{
	m_combo->HidePopup(true, &event); //maks
}

void wxTreeComboInterface::OnMouseMove ( wxMouseEvent& event )
{
    int type = event.GetEventType();
    if ( type == wxEVT_LEFT_UP )
    {
        m_combo->HidePopup();
    }
    event.Skip();
}

void wxTreeComboInterface::OnKey(wxKeyEvent& event)
{
    if ( event.GetKeyCode() == WXK_RETURN )
    {
        m_combo->HidePopup();
    }
    else
        event.Skip();
}

void wxTreeComboInterface::CheckWidth( const wxString& item )
{

    // Calculate average character width
    if ( !m_avgCharWidth )
    {
        wxASSERT ( m_combo );
        m_font = m_combo->GetFont();
        wxClientDC dc(m_combo);
        dc.SetFont(m_font);
        m_avgCharWidth = (dc.GetCharWidth()*3)/2 + 1;
    }


    int baseWidth = (m_avgCharWidth * (item.length() + 8)) + m_baseImageWidth;
    if ( m_widestWidth < baseWidth )
    {
        // Need to check the actual width
        int x, y;
        m_combo->GetTextExtent(item, &x, &y, 0, 0, &m_font);
        int actualWidth = x + (m_avgCharWidth*8) + m_baseImageWidth;
        if ( m_widestWidth < actualWidth )
        {
            //wxLogDebug(wxT("%i"),m_baseImageWidth);
            //wxLogDebug(wxT("%s: %i"),item.c_str(),actualWidth);
            m_widestWidth = actualWidth;
        }
        //wxLogDebug(wxT("%i (%i,%i)"),m_widestWidth,(int)m_avgCharWidth,(int)item.length());
    }
}

void wxTreeComboInterface::Insert( const wxString& item, int pos )
{
    // Calculate width
    //CheckWidth(pos,item);   
}

void wxTreeComboInterface::Clear()
{
    wxASSERT ( m_combo );

    if(m_popup) m_popup->DeleteAllItems(); 
}

void wxTreeComboInterface::Delete( int item )
{
    wxASSERT ( m_combo );


    /*
    if ( m_combo->HasClientObjectData() )
        delete (wxClientData *) m_datas[item];*/

    m_datas.RemoveAt(item); //maks
    
}

int wxTreeComboInterface::FindString(const wxString& s) const
{
    return 0;
}

void* wxTreeComboInterface::GetClientData( int item ) const //maks
{
	if(item >= 0 && item < m_strings.GetCount())
	{
		return m_datas[item];
	}

	return NULL;
}

int wxTreeComboInterface::GetCount() const
{
	return m_strings.Count();
}

wxString wxTreeComboInterface::GetString( int item ) const
{
    return GetValueAsString();
}

wxString wxTreeComboInterface::GetValueAsString() const
{
	if(m_popup)
	{
		wxTreeItemId id = m_popup->GetSelection();
		if(id.IsOk())
		{
			return m_popup->GetItemText(id);
		}
	}

    return wxEmptyString;
}

void wxTreeComboInterface::SetClientData( int item,
                                              void* clientData ) //maks
{
    m_datas[item] = clientData;
}

void wxTreeComboInterface::SetSelection ( int item )
{
	if(m_popup)
	{
		m_popup->SelectItem(GetCombo()->GetValueString());
	}	
}

void wxTreeComboInterface::SetString( int item, const wxString& str )
{

}

void wxTreeComboInterface::SetValueFromString ( const wxString& value )
{

}

wxWindow* wxTreeComboInterface::GeneratePopup(wxWindow* parent, int minWidth,
                                                  int maxHeight, int prefHeight )
{
    if ( parent )
    {
        m_popup = new wxTreePopup(parent,this,m_callback,0);
	}
	
	
	int height = 250;
	
	if ( m_popup->GetCount() )
	{
		if ( prefHeight > 0 )
			height = prefHeight;
		
		if ( height > maxHeight )
			height = maxHeight;
		
		int total_height = m_popup->GetTotalHeight() + 3;
		if ( height >= total_height )
		{
			height = total_height;
		}
		else
		{
			// Adjust height to a multiple of the height of the first item
			int fih = m_popup->GetLineHeight ( 0 );
			int shown = height/fih;
			height = shown * fih;
		}
	}
	else
		height = 50;
	
	// Take scrollbar into account in width calculations
	int widestWidth = m_widestWidth + wxSystemSettings::GetMetric(wxSYS_VSCROLL_X); //27;
	m_popup->Move(0,0);
	m_popup->SetSize( minWidth > widestWidth ? minWidth : widestWidth,
		height+2);
	
	
	return m_popup;
}

void wxTreeComboInterface::Populate ( int n, const wxString choices[], void **data ) //maks
{
	m_strings.Clear();
	m_datas.SetCount(n); 

    int i;
    for ( i=0; i<n; i++ )
    {
		m_strings.Add(choices[i]);
        //CheckWidth(choices[i]);
		if(data) m_datas[i] = data[i];
    }	

	if(m_popup) m_popup->UpdateTree(); //maks
}

const int* wxTreeComboInterface::GetIntPtr () const
{
    return NULL;
}

bool wxTreeComboInterface::IsHighlighted ( int item ) const
{
    if ( m_popup->GetParent()->IsShown() )
    {
		return false;
        //return m_popup->IsCurrent( item );
    }

    return false;
}


// ----------------------------------------------------------------------------
// input handling
// ----------------------------------------------------------------------------

//
// This is pushed to the event handler queue of either combo box
// or its textctrl (latter only if it is created).
//
class wxPGComboBoxExtraInputHandler : public wxEvtHandler //maks: wxComboBoxExtraInputHandler -> wxPGComboBoxExtraInputHandler (wxComboBoxExtraInputHandler already defined in the new wx)
{
public:

    wxPGComboBoxExtraInputHandler( wxPGCustomComboBox* combo )
        : wxEvtHandler()
    {
        m_combo = combo;
    }
    ~wxPGComboBoxExtraInputHandler() { }
    void OnKey(wxKeyEvent& event);

protected:
    wxPGCustomComboBox*   m_combo;

private:
    DECLARE_EVENT_TABLE()
};

BEGIN_EVENT_TABLE(wxPGComboBoxExtraInputHandler, wxEvtHandler)
    EVT_KEY_DOWN(wxPGComboBoxExtraInputHandler::OnKey)
END_EVENT_TABLE()

//extern void wxSetDebugEventType(int type);

void wxPGComboBoxExtraInputHandler::OnKey(wxKeyEvent& event)
{
    int keycode = event.GetKeyCode();

    if ( keycode == WXK_TAB )
    {
        wxNavigationKeyEvent evt;
        evt.SetFlags(wxNavigationKeyEvent::FromTab|
                     (!event.ShiftDown()?wxNavigationKeyEvent::IsForward:
                                         wxNavigationKeyEvent::IsBackward));
        evt.SetEventObject(m_combo);
        m_combo->GetParent()->GetEventHandler()->AddPendingEvent(evt);
        return;
    }

    if ( m_combo->IsPopupShown() )
    {
        // pass it to the popped up control
        m_combo->GetPopupControl()->AddPendingEvent(event);
    }
    else // no popup
    {
        if ( keycode == WXK_NUMPAD_ENTER ||
             keycode == WXK_UP || keycode == WXK_DOWN || keycode == WXK_RETURN )
        {
            m_combo->SendShowPopupSignal();
        }
        else
            event.Skip();
    }
}


// ----------------------------------------------------------------------------
// wxPGCustomComboBox
// ----------------------------------------------------------------------------

IMPLEMENT_DYNAMIC_CLASS(wxPGCustomComboBox, wxPGCustomComboBoxBase)

BEGIN_EVENT_TABLE(wxPGCustomComboBox, wxPGCustomComboBoxBase)
	EVT_MOUSE_EVENTS(wxPGCustomComboBox::OnMouseEvent)
    EVT_PAINT(wxPGCustomComboBox::OnPaint)
    EVT_SIZE(wxPGCustomComboBox::OnResize)
    //EVT_KEY_DOWN(wxPGCustomComboBox::OnKey)
    //EVT_KEY_UP(wxPGCustomComboBox::OnKey)
    //EVT_SIZE(wxPGCustomComboBox::OnResize)
    EVT_SET_FOCUS(wxPGCustomComboBox::OnFocusEvent)
    EVT_KILL_FOCUS(wxPGCustomComboBox::OnFocusEvent)
END_EVENT_TABLE()


void wxPGCustomComboBox::Init()
{
    m_winPopup = (wxWindow *)NULL;
    m_popup = (wxWindow *)NULL;
    m_isPopupShown = 0;
    m_btn = (wxWindow*) NULL;
    m_text = (wxTextCtrl*) NULL;

    m_extraEvtHandler = (wxEvtHandler*) NULL;
    m_textEvtHandler = (wxEvtHandler*) NULL;

#if wxODC_INSTALL_TOPLEV_HANDLER
    m_toplevEvtHandler = (wxEvtHandler*) NULL;
#endif

    m_heightPopup = 175;

    m_widthCustomPaint = 0;

#if wxODC_BORDER_STYLE == 1
    m_widthCustomBorder = 0;
#endif

    m_extLeft = 0;
    m_extRight = 0;

    m_fakePopupUsage = 0;
    //m_ignoreNextButtonClick = 0;
    m_downReceived = 0;

    m_timeCanAcceptClick = 0;

}

bool wxPGCustomComboBox::Create(wxWindow *parent,
                              wxWindowID id,
                              const wxString& value,
                              const wxPoint& pos,
                              const wxSize& size,
                              long style,
                              const wxValidator& validator,
                              const wxString& name)
{
    // Set border correctly.
    long border = style & wxBORDER_MASK;

#if wxODC_BORDER_STYLE == 0
    // msw style

    if ( !border )
        border = wxBORDER_SUNKEN;
    style = (style & ~(wxBORDER_MASK)) | border;
#else
    // gtk style

    // maybe save border to textctrl
    style = (style & ~(wxBORDER_MASK));

    if ( style & wxCB_READONLY )
    {
        if ( !border )
        {
            m_widthCustomBorder = 1;
            border = wxNO_BORDER;
        }

        style |= border;
    }
    else
    {
        style |= wxNO_BORDER;

        // border is then used by textctrl
        if ( !border )
        {
            //m_widthCustomBorder = 1;
            border = wxBORDER_SUNKEN;
        }
    }
#endif

    // first create our own window, i.e. the one which will contain all
    // subcontrols
    if ( !wxControl::Create(parent,
                            id,
                            wxDefaultPosition,
                            wxDefaultSize,
                            style | wxWANTS_CHARS,
                            wxDefaultValidator,
                            name) )
        return false;

    id = GetId(); // Make sure we got a valid id.

    if ( !(style & wxCB_READONLY) )
    {
        // Use same id to make event handling straighforward
        m_text = new wxTextCtrl(this,id,value,
                                wxDefaultPosition,wxDefaultSize,
                            #if wxODC_BORDER_STYLE == 0
                                wxNO_BORDER,
                            #else
                                border,
                            #endif
                                validator);

        // This is required for some platforms (GTK+ atleast)
        m_text->SetSizeHints(2,4);

        m_textEvtHandler = new wxPGComboBoxExtraInputHandler(this);
        m_text->PushEventHandler(m_textEvtHandler);
    }

    wxPGComboBoxExtraInputHandler* input_handler = new wxPGComboBoxExtraInputHandler(this);
    PushEventHandler(input_handler);
    m_extraEvtHandler = input_handler;

    m_btn = new wxComboDropButton(this,id+1,wxDefaultPosition,wxDefaultSize,wxNO_BORDER/*|wxBU_COMBO*/);

    // Prevent excess background clearing
    //   (only if readonly)
    if ( style & wxCB_READONLY )
        SetBackgroundStyle( wxBG_STYLE_CUSTOM );

    SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW));

    // for compatibility with the other ports, the height specified is the
    // combined height of the combobox itself and the popup
    // JMS: I do not understand this yet.
    /*if ( size.y == wxDefaultCoord )
    {
        // ok, use default height for popup too
        m_heightPopup = wxDefaultCoord;
    }
    else
    {
        m_heightPopup = size.y - DoGetBestSize().y;
    }

    SetBestSize(size);
    Move(pos);*/

    // Generate valid size
    wxSize useSize = size;
    wxSize bestSize = DoGetBestSize();
    if ( useSize.x < 0 )
        useSize.x = bestSize.x;
    if ( useSize.y < 0 )
        useSize.y = bestSize.y;

    SetSize(pos.x,pos.y,useSize.x,useSize.y);

    return true;
}

wxPGCustomComboBox::~wxPGCustomComboBox()
{
#if wxODC_INSTALL_TOPLEV_HANDLER
    delete ((wxComboFrameEventHandler*)m_toplevEvtHandler);
    m_toplevEvtHandler = (wxEvtHandler*) NULL;
#endif

    delete m_winPopup;

    RemoveEventHandler(m_extraEvtHandler);

    if ( m_text )
        m_text->RemoveEventHandler(m_textEvtHandler);

    delete m_textEvtHandler;
    delete m_extraEvtHandler;

    //wxLogDebug( wxT("wxPGOwnerDrawnComboBox::~wxPGOwnerDrawnComboBox ends...") );
}


// ----------------------------------------------------------------------------
// geometry stuff
// ----------------------------------------------------------------------------

wxSize wxPGCustomComboBox::DoGetBestSize() const
{
    wxASSERT ( m_btn );
    wxSize sizeBtn = m_btn->GetBestSize();
    wxSize sizeText(150,0);

    if ( m_text ) sizeText = m_text->GetBestSize();

    int fhei = 2;
    if ( m_font.Ok() )
        fhei = (m_font.GetPointSize()*2);

    return wxSize(sizeText.x + g_comboMargin + sizeBtn.x,
                  wxMax(fhei, sizeText.y) + 6);
}

void wxPGCustomComboBox::DoMoveWindow(int x, int y, int width, int height)
{
    wxControl::DoMoveWindow(x, y, width, height);
}


// ----------------------------------------------------------------------------
// operations
// ----------------------------------------------------------------------------

bool wxPGCustomComboBox::Enable(bool enable)
{
    if ( !wxPGCustomComboBoxBase::Enable(enable) )
        return false;

    m_btn->Enable(enable);
    if ( m_text )
        m_text->Enable(enable);

    return true;
}

bool wxPGCustomComboBox::Show(bool show)
{
    if ( !wxPGCustomComboBoxBase::Show(show) )
        return false;

    if (m_btn)
        m_btn->Show(show);

    if (m_text)
        m_text->Show(show);

    return true;
}

bool wxPGCustomComboBox::SetFont ( const wxFont& font )
{
    if ( !wxPGCustomComboBoxBase::SetFont(font) )
        return false;

    if (m_text)
        m_text->SetFont(font);

    return true;
}

/*
void wxPGCustomComboBox::SetFocus()
{
    //wxPGCustomComboBoxBase::SetFocus();
    //wxLogDebug(wxT("wxPGCustomComboBox::SetFocus()"));
    if (m_text)
    {
        m_text->SetFocus();
    }
    else
        wxPGCustomComboBoxBase::SetFocus();
}
*/

/*void wxPGCustomComboBox::SetFocusFromKbd()
{
    SetFocus();
}*/

#if wxUSE_TOOLTIPS
void wxPGCustomComboBox::DoSetToolTip(wxToolTip *tooltip)
{
    wxPGCustomComboBoxBase::DoSetToolTip(tooltip);

    // Set tool tip for button and text box
    if (tooltip)
    {
        const wxString &tip = tooltip->GetTip();
        if ( m_text ) m_text->SetToolTip(tip);
        if ( m_btn ) m_btn->SetToolTip(tip);
    }
    else
    {
        if ( m_text ) m_text->SetToolTip( (wxToolTip*) NULL );
        if ( m_btn ) m_btn->SetToolTip( (wxToolTip*) NULL );
    }
}
#endif // wxUSE_TOOLTIPS

void wxPGCustomComboBox::SetId( wxWindowID winid )
{
    wxPGCustomComboBoxBase::SetId(winid);
    // *must* set textctrl's id to same as combo's
    // (otherwise event handling will get messed up)
    if ( m_text ) m_text->SetId(winid);
}


// ----------------------------------------------------------------------------
// event handlers
// ----------------------------------------------------------------------------

void wxPGCustomComboBox::OnItemPaint( wxDC& dc, const wxRect& rect )
{
    if ( !m_text )
    {
        dc.DrawText(m_valueString,
                    3+wxODC_TEXTXADJUST,
                    (rect.height-dc.GetCharHeight())/2 );
    }
}

void wxPGCustomComboBox::OnPaint( wxPaintEvent& )
{
    wxPaintDC dc(this);

    // paint, whatever is required, on the control
    if ( m_btn && ( !m_text || m_widthCustomPaint ) )
    {
        wxASSERT ( m_widthCustomPaint >= 0 );

        wxSize sz = GetClientSize();
        wxRect rect(0,0, sz.x - m_btn->GetSize().x, sz.y );

#if wxODC_BORDER_STYLE == 1
        // artifical simple border around custom-drawn value
        if ( m_widthCustomBorder )
        {
            dc.SetPen( wxSystemSettings::GetColour( wxSYS_COLOUR_BTNSHADOW ) );
            dc.SetBrush( *wxTRANSPARENT_BRUSH );
            dc.DrawRectangle( rect );
            rect.Deflate(m_widthCustomBorder);
        }
#endif

        // this is intentionally here to allow drawed rectangle's
        // right edge to be hidden
        if ( m_text )
            rect.width = m_widthCustomPaint;

        wxColour win_col = wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW);

        dc.SetFont( GetFont() );

        dc.SetBrush ( win_col );
        dc.SetPen ( win_col );
        dc.DrawRectangle ( rect );

    #if wxODC_SELECTION_STYLE == 0 || wxODC_SELECTION_STYLE == 2
        // If popup is hidden and this control is focused,
        // then draw the focus-indicator (blue background etc.).
        wxWindow* cur_focus = FindFocus();
        if ( m_isPopupShown < 1 && (cur_focus == this || cur_focus == m_btn) &&
             !m_text
           )
        {
            wxRect sel_rect(rect);
            sel_rect.y += 1;
            sel_rect.height -= 2;
            sel_rect.x += m_widthCustomPaint + 1;
            sel_rect.width -= m_widthCustomPaint + 2;

            dc.SetTextForeground( wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHTTEXT) );

            DrawComboSelectionBackground( dc, sel_rect );
        }
        else
            dc.SetTextForeground( wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOWTEXT) );
    #else
        wxColour tx_col(wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOWTEXT));

        // If popup is hidden and this control is focused,
        // then draw the focus-indicator (blue background etc.).
        wxWindow* cur_focus = FindFocus();
        if ( m_isPopupShown < 1 &&
             (cur_focus == this || cur_focus == m_btn) &&
             !m_text )
        {
            wxRect sel_rect(rect);
            sel_rect.y += 1;
            sel_rect.height -= 2;
            sel_rect.x += m_widthCustomPaint + 1;
            sel_rect.width -= m_widthCustomPaint + 2;

            wxPGDrawFocusRect(dc,sel_rect);
       }
       dc.SetTextForeground( tx_col );
    #endif

        OnItemPaint(dc,rect);

    }
}

void wxPGCustomComboBox::OnPopupDismiss()
{
    //wxLogDebug(wxT("wxPGOwnerDrawnComboBox::OnPopupDismiss(m_isPopupShown=%i)"),m_isPopupShown);

    // Just in case, avoid double dismiss
    if ( !m_isPopupShown )
        return;

    // *Must* set this before focus etc.
    m_isPopupShown = 0;

    ((wxComboDropButton*)m_btn)->SetPopup( (wxWindow*) NULL );

#if wxODC_INSTALL_TOPLEV_HANDLER
    // Remove top level window event handler
    if ( m_toplevEvtHandler )
    {
        wxWindow* toplev = ::wxGetTopLevelParent( this );
        if ( toplev )
            toplev->RemoveEventHandler( m_toplevEvtHandler );
    }
#endif

#if !wxUSE_POPUPWIN
    if ( m_fakePopupUsage != 2 )
        GetParent()->SetFocus();
#endif

    m_timeCanAcceptClick = ::wxGetLocalTimeMillis() + 150;

    // refresh control (necessary even if m_text)
    Refresh();
}

void wxPGCustomComboBox::HidePopup( bool WXUNUSED(sendEvent) )
{
    wxASSERT( m_winPopup );	

#if wxUSE_POPUPWIN
    if ( m_winPopup->IsKindOf(CLASSINFO(wxPopupTransientWindow)) )
    {
        ((wxPopupTransientWindow*)m_winPopup)->Dismiss();
    }
    else
#endif
    {
        m_winPopup->Hide();
    }

    OnPopupDismiss();
}

void wxPGCustomComboBox::SendShowPopupSignal()
{
    wxCommandEvent evt(wxEVT_COMMAND_BUTTON_CLICKED,GetId());
    GetEventHandler()->AddPendingEvent(evt);
}

void wxPGCustomComboBox::OnMouseEvent( wxMouseEvent& event )
{
    int type = event.GetEventType();

    /*
    static unsigned long prev_time = 0;
    unsigned long time = (unsigned long) ::wxGetLocalTimeMillis().GetLo();
    unsigned long passed = time-prev_time;

    if ( type == wxEVT_LEFT_UP )
        wxLogDebug(wxT("wxEVT_LEFT_UP, t = %ums"),passed);
    else if ( type == wxEVT_LEFT_DOWN )
        wxLogDebug(wxT("wxEVT_LEFT_DOWN, t = %ums"),passed);

    prev_time = time;
    */

    // Do not accept clicks immediately after popup dismiss
    /*if ( t < m_timeCanAcceptClick )
    {
        event.Skip();
        return;
    }*/

    //
    // Generate our own double-clicks
    // (to allow on-focus dc-event on double-clicks instead of triple-clicks)
    if ( (m_windowStyle & wxODCB_DOUBLE_CLICK_CYCLES) && !m_isPopupShown )
    {
        if ( type == wxEVT_LEFT_DOWN )
        {
            // Set value to avoid up-events without corresponding downs
            m_downReceived = 1;
        }
        else if ( type == wxEVT_LEFT_DCLICK )
        {
            // We'll make our own double-clicks
            type = 0;
        }
        else if ( type == wxEVT_LEFT_UP )
        {
            if ( m_downReceived || m_timeLastMouseUp == 1 )
            {
                wxLongLong t = ::wxGetLocalTimeMillis();
                wxLongLong t_from_last_up = (t-m_timeLastMouseUp);
                //wxLogDebug(wxT("wxEVT_LEFT_UP, t_from_last_up = %ums  m_timeLastMouseUp = %ums"),t_from_last_up.GetLo(),m_timeLastMouseUp.GetLo());

                if ( t_from_last_up < wxODCB_DOUBLE_CLICK_CONVERSION_TRESHOLD )
                {
                    type = wxEVT_LEFT_DCLICK;
                    m_timeLastMouseUp = 1;
                }
                else
                {
                    m_timeLastMouseUp = t;
                }

                //m_downReceived = 0;
            }
        }
    }

    //if ( type != wxEVT_MOTION ) wxLogDebug(wxT("wxPGOwnerDrawnComboBox::OnMouseEvent"));

    if ( type == wxEVT_LEFT_DOWN || type == wxEVT_LEFT_DCLICK )
    {
        //wxLogDebug(wxT("wxPGOwnerDrawnComboBox::OnMouseEvent"));
    #if !wxODC_USE_TRANSIENT_POPUP
        // Check if event occurs on the button (technically not necessary on MSW,
        // but important in GTK).
        // TODO: Remove this
        if ( event.GetPosition().x < m_btn->GetPosition().x )
    #endif
        {
            if ( m_isPopupShown > 0 )
            {
        #if !wxUSE_POPUPWIN
            // Normally do nothing - evt handler should closed it for us
          #if wxODC_ALLOW_FAKE_POPUP
            if ( m_fakePopupUsage == 2 )
                HidePopup( false );
          #endif
        #elif !wxODC_USE_TRANSIENT_POPUP
                // Click here always hides the popup.
                HidePopup( false );
        #endif
            }
            else
            {

                if ( IsKindOf(CLASSINFO(wxPGOwnerDrawnComboBox)) )
                {
                    wxPGOwnerDrawnComboBox* odcb = (wxPGOwnerDrawnComboBox*) this;

                    if ( !(m_windowStyle & wxODCB_DOUBLE_CLICK_CYCLES) || !odcb->m_hasIntValue )
                    {
                        // In read-only mode, clicking the text is the
                        // same as clicking the button.
                        SendShowPopupSignal();
                    }
                    else if ( type == wxEVT_LEFT_UP || type == wxEVT_LEFT_DCLICK )
                    {
                        int value = *odcb->m_popupInterface->GetIntPtr();
                        int max_value = odcb->m_popupInterface->GetCount();

                        if ( !::wxGetKeyState(WXK_SHIFT) )
                            value++;
                        else
                            value--;

                        if ( value >= max_value )
                            value = 0;
                        else if ( value < 0 )
                            value = max_value-1;

                        //wxLogDebug(wxT("%i"),value);

                        odcb->Select(value);

                        // fire the event
                        wxCommandEvent event(wxEVT_COMMAND_COMBOBOX_SELECTED, GetId());
                        event.SetEventObject(this);
                        event.SetInt(value);
                        GetEventHandler()->AddPendingEvent(event);

                    }
                }
                else
                {
                    SendShowPopupSignal();
                }
            }
        }
    }
    else if ( m_isPopupShown > 0 )
    {
        // relay (some) mouse events to the popup
        if ( type == wxEVT_MOUSEWHEEL )
            m_popup->AddPendingEvent(event);
    }
    else
        event.Skip();
}

void wxPGCustomComboBox::OnResize( wxSizeEvent& event )
{
    if ( !m_btn )
        return;

    //wxLogDebug(wxT("wxPGOwnerDrawnComboBox::OnResize"));

    wxSize sz = GetClientSize();

    int butWidth = sz.y;
    if ( butWidth > wxODC_DROPDOWN_BUTTON_MAX_WIDTH )
        butWidth = wxODC_DROPDOWN_BUTTON_MAX_WIDTH;

    m_btn->SetSize(sz.x - butWidth,
                   0,
                   butWidth,
                   sz.y
                  );

    if ( m_text )
    {
        //wxLogDebug( wxT("wxPGOwnerDrawnComboBox::DoMoveWindow(%i,%i)"),textRect.x,textRect.y);
        //m_text->SetSize(textRect);

#if wxODC_BORDER_STYLE == 1
        if ( m_text->GetWindowStyleFlag() & wxNO_BORDER )
#endif
        {
            // Centre textctrl
            wxSize tsz = m_text->GetSize();
            int diff = sz.y - tsz.y;
            m_text->SetSize(wxODC_TEXTCTRLXADJUST + m_widthCustomPaint,
                            wxODC_TEXTCTRLYADJUST + (diff/2),
                            sz.x - butWidth - g_comboMargin - (wxODC_TEXTCTRLXADJUST + m_widthCustomPaint),
                            -1);
        }
#if wxODC_BORDER_STYLE == 1
        else
        {
            m_text->SetSize(m_widthCustomPaint + m_widthCustomBorder,
                            0,
                            sz.x - butWidth - m_widthCustomPaint - m_widthCustomBorder,
                            sz.y);
        }
#endif
    }

    // update area outside the button
    if ( !m_text )
    {
        wxRect updateRect(0,0,sz.x - butWidth,sz.y);
        RefreshRect(updateRect);
    }

    event.Skip();
}

void wxPGCustomComboBox::OnFocusEvent( wxFocusEvent& )
{
    //wxLogDebug(wxT("wxPGCustomComboBox::OnSetFocus()"));

    // First click is the first part of double-click
    // Some platforms don't generate down-less mouse up-event
    // (Windows does, GTK+2 doesn't), so that's why we have
    // to do this.
    m_timeLastMouseUp = ::wxGetLocalTimeMillis();

    if (m_text)
        m_text->SetFocus();
    else
        // no need to check for m_widthCustomPaint - that
        // area never gets special handling when selected
        // (in writable mode, that is)
        Refresh();
}

/*void wxPGCustomComboBox::OnKey(wxKeyEvent& event)
{
    if ( m_isPopupShown > 0 )
    {
        // pass it to the popped up control
        m_popupInterface->GetPopupControl()->AddPendingEvent(event);
    }
    else // no popup
    {
        event.Skip();
    }
}*/


// ----------------------------------------------------------------------------
// user methods
// ----------------------------------------------------------------------------

void wxPGCustomComboBox::SetPopupExtents( int extLeft, int extRight, int prefHeight )
{
    m_extLeft = extLeft;
    m_extRight = extRight;
    m_heightPopup = prefHeight;

    Refresh();
}

void wxPGCustomComboBox::SetCustomPaintArea( int width )
{
    if ( m_text )
    {
        // move textctrl accordingly
        wxRect r = m_text->GetRect();
        int inc = width - m_widthCustomPaint;
        r.x += inc; r.width -= inc;

        m_text->SetSize( r );

#if wxODC_BORDER_STYLE == 1
        // do we need to set/reset border?
        if ( !width )
        {
            m_widthCustomBorder = 0;
        }
        else
        {
            int tx_border = m_text->GetWindowStyleFlag() & wxBORDER_MASK;

            // FIXME: Get real border with.
            if ( !(tx_border & wxNO_BORDER) )
                m_widthCustomBorder = 1;
        }
#endif
    }
    
    m_widthCustomPaint = width;

}

// ----------------------------------------------------------------------------
// methods forwarded to wxTextCtrl
// ----------------------------------------------------------------------------

wxString wxPGCustomComboBox::GetValue() const
{
    if ( m_text )
        return m_text->GetValue();
    return m_valueString;
}

void wxPGCustomComboBox::SetValue(const wxString& value)
{
    if ( m_text )
        m_text->SetValue(value);
    m_valueString = value;
    Refresh();
}

void wxPGCustomComboBox::Copy()
{
    wxASSERT_MSG ( m_text, wxT("Do not call this wxPGCustomComboBox method in read-only mode.") );
    GetTextCtrl()->Copy();
}

void wxPGCustomComboBox::Cut()
{
    wxASSERT_MSG ( m_text, wxT("Do not call this wxPGCustomComboBox method in read-only mode.") );
    GetTextCtrl()->Cut();
}

void wxPGCustomComboBox::Paste()
{
    wxASSERT_MSG ( m_text, wxT("Do not call this wxPGCustomComboBox method in read-only mode.") );
    GetTextCtrl()->Paste();
}

void wxPGCustomComboBox::SetInsertionPoint(long pos)
{
    wxASSERT_MSG ( m_text, wxT("Do not call this wxPGCustomComboBox method in read-only mode.") );
    GetTextCtrl()->SetInsertionPoint(pos);
}

void wxPGCustomComboBox::SetInsertionPointEnd()
{
    wxASSERT_MSG ( m_text, wxT("Do not call this wxPGCustomComboBox method in read-only mode.") );
    GetTextCtrl()->SetInsertionPointEnd();
}

long wxPGCustomComboBox::GetInsertionPoint() const
{
    wxASSERT_MSG ( m_text, wxT("Do not call this wxPGCustomComboBox method in read-only mode.") );
    return GetTextCtrl()->GetInsertionPoint();
}

long wxPGCustomComboBox::GetLastPosition() const
{
    wxASSERT_MSG ( m_text, wxT("Do not call this wxPGCustomComboBox method in read-only mode.") );
    return GetTextCtrl()->GetLastPosition();
}

void wxPGCustomComboBox::Replace(long from, long to, const wxString& value)
{
    wxASSERT_MSG ( m_text, wxT("Do not call this wxPGCustomComboBox method in read-only mode.") );
    GetTextCtrl()->Replace(from, to, value);
}

void wxPGCustomComboBox::Remove(long from, long to)
{
    wxASSERT_MSG ( m_text, wxT("Do not call this wxPGCustomComboBox method in read-only mode.") );
    GetTextCtrl()->Remove(from, to);
}

void wxPGCustomComboBox::SetSelection(long from, long to)
{
    wxASSERT_MSG ( m_text, wxT("Do not call this wxPGCustomComboBox method in read-only mode.") );
    GetTextCtrl()->SetSelection(from, to);
}

/*void wxPGCustomComboBox::SetEditable(bool editable)
{
    GetTextCtrl()->SetEditable(editable);
}*/


// ----------------------------------------------------------------------------
// wxPGOwnerDrawnComboBox
// ----------------------------------------------------------------------------


IMPLEMENT_DYNAMIC_CLASS(wxPGOwnerDrawnComboBox, wxPGCustomComboBox)


BEGIN_EVENT_TABLE(wxPGOwnerDrawnComboBox, wxPGCustomComboBox)
    EVT_BUTTON(wxID_ANY, wxPGOwnerDrawnComboBox::OnButtonClick)
END_EVENT_TABLE()


void wxPGOwnerDrawnComboBox::Init()
{
    m_popupInterface = (wxComboPopupInterface*) NULL;
    m_hasIntValue = false;
    /*
    //m_btn = (wxWindow*) NULL;
    //m_text = (wxTextCtrl*) NULL;

    //m_extraEvtHandler = (wxEvtHandler*) NULL;
    //m_textEvtHandler = (wxEvtHandler*) NULL;
#if wxODC_INSTALL_TOPLEV_HANDLER
    m_toplevEvtHandler = (wxEvtHandler*) NULL;
#endif

    m_heightPopup = 175;

    m_widthCustomPaint = 0;

#if wxODC_BORDER_STYLE == 1
    m_widthCustomBorder = 0;
#endif

    m_extLeft = 0;
    m_extRight = 0;
    */
}

bool wxPGOwnerDrawnComboBox::Create(wxWindow *parent,
                            wxWindowID id,
                            const wxString& value,
                            const wxPoint& pos,
                            const wxSize& size,
                            wxComboPopupInterface* iface,
                            long style,
                            const wxValidator& validator,
                            const wxString& name)
{

    // Prepare interface
    if ( iface )
        SetPopupInterface(iface);

    return wxPGCustomComboBox::Create(parent,id,value,pos,size,style,validator,name);
}

wxPGOwnerDrawnComboBox::wxPGOwnerDrawnComboBox(wxWindow *parent,
                       wxWindowID id,
                       const wxString& value,
                       const wxPoint& pos,
                       const wxSize& size,
                       const wxArrayString& choices,
                       wxComboPaintCallback callback,
                       long style,
                       const wxValidator& validator,
                       const wxString& name)
    : wxPGCustomComboBox()
{
    Init();

    Create(parent, id, value, pos, size, choices, callback, style, validator, name);
}

bool wxPGOwnerDrawnComboBox::Create(wxWindow *parent,
                        wxWindowID id,
                        const wxString& value,
                        const wxPoint& pos,
                        const wxSize& size,
                        const wxArrayString& choices,
                        wxComboPaintCallback callback,
                        long style,
                        const wxValidator& validator,
                        const wxString& name)
{
    wxCArrayString chs(choices);

    return Create(parent, id, value, pos, size, chs.GetCount(),
                  chs.GetStrings(), callback, style, validator, name);
}

bool wxPGOwnerDrawnComboBox::Create(wxWindow *parent,
                        wxWindowID id,
                        const wxString& value,
                        const wxPoint& pos,
                        const wxSize& size,
                        int n,
                        const wxString choices[],
                        wxComboPaintCallback callback,
                        long style,
                        const wxValidator& validator,
                        const wxString& name)
{
	wxComboPopupInterface *iface;

	if(style & wxODCB_TREE) //maks
	{
		iface = new wxTreeComboInterface(callback);
	}
	else
	{
		iface = new wxVListBoxComboInterface(callback);
	}

    iface->Init(this);

    if ( !Create(parent, id, value, pos, size, iface, style,
                 validator, name) )
    {
        return false;
    }

    // Add initial choices to the interface.
    iface->Populate(n,choices);

    return true;
}

wxPGOwnerDrawnComboBox::~wxPGOwnerDrawnComboBox()
{
    ClearClientDatas();

    if ( m_isPopupShown > 0 )
        HidePopup( false );

    delete m_popupInterface;

}

void wxPGOwnerDrawnComboBox::ClearClientDatas()
{
    if ( HasClientObjectData() )
    {
        size_t i;
        for ( i=0; i<m_clientDatas.GetCount(); i++ )
            delete (wxClientData*) m_clientDatas[i];
    }

    m_clientDatas.Empty();
}

bool wxPGOwnerDrawnComboBox::IsHighlighted ( int item ) const
{
    wxASSERT ( m_popupInterface );
    return m_popupInterface->IsHighlighted(item);
}

// ----------------------------------------------------------------------------
// popup window handling
// ----------------------------------------------------------------------------

void wxPGOwnerDrawnComboBox::SetPopupInterface( wxComboPopupInterface* iface )
{
    delete m_popupInterface;
    delete m_winPopup;

    m_popupInterface = iface;

#if wxODC_ALLOW_FAKE_POPUP
    m_fakePopupUsage = 0;
#endif

    bool create_now = iface->Init(this);

    if ( create_now )
    {
        m_winPopup = new wxComboPopupWindow ( this, wxNO_BORDER );

        // Dimensions are pretty much dummy here
        // (this is called again in ShowPopup to properly resize the control)
        m_popup = iface->GeneratePopup( m_winPopup,
            100, 400, m_heightPopup );

        // Add interface as event handler
        m_popup->PushEventHandler( iface );

        // FIXME: This bypasses wxGTK popupwindow bug
        //   (i.e. window is not initially hidden when it should be)
        m_winPopup->Hide();

#if wxODC_ALLOW_FAKE_POPUP
        m_fakePopupUsage = 1;
#endif
    }

    m_hasIntValue = iface->GetIntPtr() ? true : false;
}

void wxPGOwnerDrawnComboBox::ShowPopup()
{

    wxCHECK_RET( m_popupInterface, _T("no popup interface specified in wxPGOwnerDrawnComboBox") );
    wxCHECK_RET( !IsPopupShown(), _T("popup window already shown") );

    SetFocus();

    //wxLogDebug( wxT("wxPGOwnerDrawnComboBox::ShowPopup") );

    // Space above and below
    int screen_height;
    wxPoint scr_pos;
    int space_above;
    int space_below;
    int maxHeightPopup;
    wxSize ctrl_sz = GetSize();

#if wxODC_ALLOW_FAKE_POPUP

    int existing_height = 200;
    if ( m_popup )
        existing_height = m_popup->GetSize().y;

    //wxLogDebug(wxT("Existing height: %i"),existing_height);

    int screen_width;
    GetParent()->GetClientSize(&screen_width,&screen_height);
    screen_width -= 2;
    scr_pos = GetPosition();

    space_above = scr_pos.y - 2;
    space_below = screen_height - space_above - ctrl_sz.y - 4;

    maxHeightPopup = space_below;
    if ( space_above > space_below )
        maxHeightPopup = space_above;

    //wxLogDebug(wxT("FakeMaxHei: %i"),maxHeightPopup);

    if ( maxHeightPopup >= existing_height )
    {
        if ( m_winPopup && m_fakePopupUsage!=2 )
        {
            delete m_winPopup;
            m_winPopup = (wxWindow*) NULL;
            m_popup = (wxWindow*) NULL;
        }
        m_fakePopupUsage = 2;
    }
    else
    {
        if ( m_winPopup && m_fakePopupUsage!=1 )
        {
            delete m_winPopup;
            m_winPopup = (wxWindow*) NULL;
            m_popup = (wxWindow*) NULL;
        }
        m_fakePopupUsage = 1;
#else
    {
#endif

        screen_height = wxSystemSettings::GetMetric( wxSYS_SCREEN_Y );
        scr_pos = GetParent()->ClientToScreen(GetPosition());

        space_above = scr_pos.y;
        space_below = screen_height - space_above - ctrl_sz.y;

        maxHeightPopup = space_below;
        if ( space_above > space_below )
            maxHeightPopup = space_above;

        //wxLogDebug(wxT("RealMaxHei: %i"),maxHeightPopup);

    }

    // Width
    int widthPopup = ctrl_sz.x + m_extLeft + m_extRight;

    wxWindow* winPopup = m_winPopup;
    wxWindow* give_parent = (wxWindow*) NULL;

    if ( !winPopup )
    {
#if wxODC_ALLOW_FAKE_POPUP
        if ( m_fakePopupUsage == 2 )
        {
            //wxLogDebug(wxT("Faking popup!"));
            winPopup = new wxWindow();
        #ifdef __WXMSW__
            // Only wxMSW supports this
            winPopup->Hide();
        #endif
            winPopup->Create( GetParent(), -1 );
        }
        else
#endif
        {
            winPopup =
                new wxComboPopupWindow( this, wxNO_BORDER );
        }
        m_winPopup = give_parent = winPopup;
    }

    // Generate/resize window
    wxWindow* popup = m_popupInterface->GeneratePopup( give_parent,
        widthPopup, maxHeightPopup, m_heightPopup );

    wxASSERT ( !m_popup || m_popup == popup ); // Consistency check.

#if wxODC_ALLOW_FAKE_POPUP
    // Make sure fake popup didn't get too big
    if ( m_fakePopupUsage == 2 && popup->GetSize().x > screen_width )
    {
        popup->SetSize(screen_width-2,popup->GetSize().y);
    }
#endif

    // Add interface as event handler
    if ( !m_popup )
        popup->PushEventHandler( m_popupInterface );

    //
    // Reposition and resize popup window
    //

    wxSize szp = popup->GetSize();

    wxPoint ptp(scr_pos.x+ctrl_sz.x+m_extRight-szp.x,
        scr_pos.y + ctrl_sz.y);

#if wxODC_ALLOW_FAKE_POPUP
    if ( m_fakePopupUsage == 2 )
    {
        if ( space_below < szp.y )
        {
            //wxLogDebug(wxT("%i,%i"),space_below,space_above);
            if ( space_above > space_below )
            {
                if ( szp.y > space_above )
                {
                    popup->SetSize(szp.x,space_above);
                    szp.y = space_above;
                }
                ptp.y = scr_pos.y - szp.y;
            }
            else
            {
                if ( szp.y > space_below )
                {
                    popup->SetSize(szp.x,space_below);
                    szp.y = space_below;
                }
            }
        }
    }
    else
#endif
    if ( space_below < szp.y )
    {
        ptp.y = scr_pos.y - szp.y;
    }

    // Move to position
    //wxLogDebug(wxT("popup scheduled position1: %i,%i"),ptp.x,ptp.y);
    //wxLogDebug(wxT("popup position1: %i,%i"),winPopup->GetPosition().x,winPopup->GetPosition().y);

    //winPopup->SetSize( ptp.x, ptp.y, szp.x, szp.y );
    // Some platforms (GTK) may need these two to be separate
    winPopup->SetSize( szp.x, szp.y );
    winPopup->Move( ptp.x, ptp.y );

    //wxLogDebug(wxT("popup position2: %i,%i"),winPopup->GetPosition().x,winPopup->GetPosition().y);

    //wxLogDebug(wxT("popup position3: %i,%i"),winPopup->GetPosition().x,winPopup->GetPosition().y);

    m_popup = popup;

    ((wxComboDropButton*)m_btn)->SetPopup( popup );

    if ( m_text )
    {
        m_text->SelectAll();

        // Set string selection (must be this way instead of SetStringSelection)
        m_popupInterface->SetValueFromString( m_text->GetValue() );

    #ifdef __WXDEBUG__
        // Id confirmation
        wxASSERT_MSG ( GetId() == m_text->GetId(),
            wxT("You must use wxPGOwnerDrawnComboBox::SetId to set its id - calling parent class's won't do"));
    #endif

    }
    else
    {
        Refresh();
    }

    // this must be after SetValueFromString
    m_isPopupShown = 1;

    // Show it
    {
    #if wxODC_USE_TRANSIENT_POPUP
        ((wxPopupTransientWindow*)winPopup)->Popup(popup);
    #else		
        winPopup->Show();
        //popup->SetFocus();
    #endif
    }

#if wxODC_INSTALL_TOPLEV_HANDLER
    // If our real popup is wxDialog, then only install handler
    // incase of fake popup.
  #if !wxUSE_POPUPWIN
    if ( m_fakePopupUsage != 2 )
    {
        if ( m_toplevEvtHandler )
        {
            delete m_toplevEvtHandler;
            m_toplevEvtHandler = (wxEvtHandler*) NULL;
        }
    }
    else
  #endif
    {
        // Put top level window event handler into place
        if ( !m_toplevEvtHandler )
            m_toplevEvtHandler = new wxComboFrameEventHandler(this);

        wxWindow* toplev = ::wxGetTopLevelParent( this );
        wxASSERT( toplev );
        ((wxComboFrameEventHandler*)m_toplevEvtHandler)->OnPopup();
        toplev->PushEventHandler( m_toplevEvtHandler );
    }
#endif

}

void wxPGOwnerDrawnComboBox::HidePopup( bool sendEvent,  wxCommandEvent *pEvent ) //maks
{
    wxCHECK_RET( m_popupInterface, _T("no popup interface specified in wxPGOwnerDrawnComboBox") );
    if ( m_isPopupShown < 1 )
    {
        //wxLogDebug(wxT("wxPGOwnerDrawnComboBox::HidePopup: Warning: trying to hide hidden popup."));
        return;
    }

    //wxLogDebug( wxT("wxPGOwnerDrawnComboBox::HidePopup") );

    // transfer value and refresh control
    m_valueString = m_popupInterface->GetValueAsString();
    if ( m_text )
        m_text->SetValue (m_valueString);

#if wxODC_USE_TRANSIENT_POPUP
    ((wxPopupTransientWindow*)m_winPopup)->Dismiss();
#else
    m_winPopup->Hide();
#endif

    OnPopupDismiss();


    if ( sendEvent )
    {
        // fire the event
        wxCommandEvent event(wxEVT_COMMAND_COMBOBOX_SELECTED, GetId());
        event.SetEventObject(this);
        event.SetInt(GetSelection());

		if(pEvent) //maks
		{
			event.SetString(pEvent->GetString());
			event.SetClientData(pEvent->GetClientData());
		}

        GetEventHandler()->AddPendingEvent(event);
    }

    //wxLogDebug( wxT("wxPGOwnerDrawnComboBox::HidePopup") );
}

// ----------------------------------------------------------------------------
// event handlers
// ----------------------------------------------------------------------------

void wxPGOwnerDrawnComboBox::OnItemPaint( wxDC& dc, const wxRect& rect )
{

    wxComboPaintCallback callback = m_popupInterface->GetCallback();

    if ( !(m_windowStyle & wxODCB_STD_CONTROL_PAINT) && callback )
    {
        ((GetParent())->*callback)(this,GetSelection(),
                                   dc,(wxRect&)rect,
                                   wxODCB_CB_PAINTING_CONTROL);
    }
    else
        wxPGCustomComboBox::OnItemPaint(dc,rect);
}


// SendShowPopupSignal call triggers this
void wxPGOwnerDrawnComboBox::OnButtonClick( wxCommandEvent& WXUNUSED(event) )
{
    //wxLogDebug(wxT("wxPGOwnerDrawnComboBox::OnButtonClick(m_isPopupShown=%i,m_ignoreNextButtonClick=%i)"),m_isPopupShown,m_ignoreNextButtonClick);

    /*if ( m_ignoreNextButtonClick )
    {
        m_ignoreNextButtonClick = 0;
        return;
    }*/

    wxLongLong t = ::wxGetLocalTimeMillis();

    // Do not accept clicks immediately after popup dismiss
    if ( t < m_timeCanAcceptClick )
    {
        return;
    }

    SetFocus();

    // Must check exactly against 0 to allow -1 value to take effect below
    if ( m_isPopupShown == 0 )
    {
        ShowPopup();
    }
    else
    {
    #if !wxUSE_POPUPWIN
        if ( m_isPopupShown > 0 )
            HidePopup( false );
    #elif !wxODC_USE_TRANSIENT_POPUP
        HidePopup( false );
        //m_winPopup->OnDismiss();
    #else
        // Why this? See wxComboPopupWindow::OnDismiss
        OnPopupDismiss();
    #endif
    }
}

// ----------------------------------------------------------------------------
// More wxPGOwnerDrawnComboBox methods
// ----------------------------------------------------------------------------

void wxPGOwnerDrawnComboBox::Clear()
{
    wxASSERT ( m_popupInterface );

    m_popupInterface->Clear();

    ClearClientDatas();

    //maks GetTextCtrl()->SetValue(wxEmptyString);
}

void wxPGOwnerDrawnComboBox::Delete(unsigned int n)
{
    if ( m_hasIntValue )
    {
        wxCHECK_RET( (n >= 0) && (n < GetCount()), _T("invalid index in wxPGOwnerDrawnComboBox::Delete") );

        if ( GetSelection() == n )
            SetValue(wxEmptyString);

        // Remove client data, if set
        if ( m_clientDatas.GetCount() )
        {
            if ( HasClientObjectData() )
                delete (wxClientData *) m_clientDatas[n];

            m_clientDatas.RemoveAt(n);
        }

        m_popupInterface->Delete(n);
    }
}

unsigned int wxPGOwnerDrawnComboBox::GetCount() const
{
    wxASSERT ( m_popupInterface );
    return m_popupInterface->GetCount();
}

wxString wxPGOwnerDrawnComboBox::GetString(unsigned int n) const
{
    wxCHECK_MSG( (n >= 0) && (n < GetCount()), wxEmptyString, _T("invalid index in wxPGOwnerDrawnComboBox::GetString") );
    return m_popupInterface->GetString(n);
}

void wxPGOwnerDrawnComboBox::SetString(unsigned int n, const wxString& s)
{
    wxCHECK_RET( (n >= 0) && (n < GetCount()), _T("invalid index in wxPGOwnerDrawnComboBox::SetString") );
    m_popupInterface->SetString(n,s);
}

int wxPGOwnerDrawnComboBox::FindString(const wxString& s) const
{
    wxASSERT ( m_popupInterface );
    return m_popupInterface->FindString(s);
}

void wxPGOwnerDrawnComboBox::Select(int n)
{
    wxCHECK_RET( (n >= -1) && (n < GetCount()), _T("invalid index in wxPGOwnerDrawnComboBox::Select") );

    m_popupInterface->SetSelection(n);

    wxString str;
    if ( n >= 0 )
        str = m_popupInterface->GetString(n);

    // Refresh text portion in control
    if ( m_text )
        m_text->SetValue( str );
    else
        m_valueString = str;

    Refresh();
}

int wxPGOwnerDrawnComboBox::GetSelection() const
{
    if ( m_hasIntValue )
    {
        wxASSERT ( m_popupInterface );
    #if 1 // FIXME:: What is the correct behavior?
        // if the current value isn't one of the listbox strings, return -1
        return *m_popupInterface->GetIntPtr();
    #else
        // Why oh why is this done this way?
        // It is not because the value displayed in the text can be found
        // in the list that it is the item that is selected!
        return FindString( GetValue() );
    #endif
    }
    return -1;
}

int wxPGOwnerDrawnComboBox::DoAppend(const wxString& item)
{
    wxASSERT ( m_popupInterface );

    int pos = m_popupInterface->GetCount();

    m_popupInterface->Insert(item,pos);

    return pos;
}

int wxPGOwnerDrawnComboBox::DoInsert(const wxString& item, unsigned int pos)
{
    wxCHECK_MSG(!(GetWindowStyle() & wxCB_SORT), -1, wxT("can't insert into sorted list"));
    wxCHECK_MSG((pos>=0) && (pos<=GetCount()), -1, wxT("invalid index"));

    m_popupInterface->Insert(item,pos);

    return pos;
}

void wxPGOwnerDrawnComboBox::DoSetItemClientData(unsigned int n, void* clientData)
{
    m_clientDatas.SetCount(n+1,NULL);
    m_clientDatas[n] = clientData;
}

void *wxPGOwnerDrawnComboBox::DoGetItemClientData(unsigned int n) const
{
    if ( (int)m_clientDatas.GetCount() > n )
        return m_clientDatas[n];

    return NULL;
}

void wxPGOwnerDrawnComboBox::DoSetItemClientObject(unsigned int n, wxClientData* clientData)
{
    DoSetItemClientData(n, (void*) clientData);
}

wxClientData* wxPGOwnerDrawnComboBox::DoGetItemClientObject(unsigned int n) const
{
    return (wxClientData*) DoGetItemClientData(n);
}

#endif // wxUSE_COMBOBOX
