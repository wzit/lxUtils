
#include <cassert>
#include <memory>
#include <tuple>
#include <utility>
#include <functional>
#include <sstream>
#include <thread>
#include <deque>

#include "wx/wx.h"

#include "lx/ulog.h"

#include "lx/xcolor.h"

using namespace std;
using namespace	LX;

// hash log id
#define CLIENT_LOG_MACRO(arg)	arg = #arg##_log

enum ImpLogLevel : LogLevel
{
	CLIENT_LOG_MACRO(APP_INIT),
	CLIENT_LOG_MACRO(USER_CMD),
	CLIENT_LOG_MACRO(USER1),
	CLIENT_LOG_MACRO(USER2),
	CLIENT_LOG_MACRO(USER3),
};	

#undef CLIENT_LOG_MACRO

struct log_def
{
	log_def(const string &label, const RGB_COLOR &clr)
		: m_Label(label), m_Color(clr)
	{}
	
	log_def(const log_def&) = default;
	log_def& operator=(const log_def&) = default;
	
	const string	m_Label;
	const RGB_COLOR	m_Color;
};

#define LOG_DEF_MACRO(t, clr)	{t, log_def(#t, RGB_COLOR::clr)}

static const
unordered_map<LogLevel, log_def>	s_LogLevelDefMap
{
	// common / built-in
	LOG_DEF_MACRO(FATAL,		NIGHT_RED),
	LOG_DEF_MACRO(ERROR,		RED),
	LOG_DEF_MACRO(EXCEPTION,	BLUE),
	LOG_DEF_MACRO(WARNING,		ORANGE),
	LOG_DEF_MACRO(MSG,		BLACK),
	LOG_DEF_MACRO(DTOR,		BROWN),
	
	// client
	LOG_DEF_MACRO(APP_INIT,		NIGHT_BLUE),
	LOG_DEF_MACRO(USER_CMD,		GREEN),
	LOG_DEF_MACRO(USER1,		PURPLE),
	LOG_DEF_MACRO(USER2,		BLUE),
	LOG_DEF_MACRO(USER3,		CYAN),
};

#undef LOG_DEF_MACRO

enum
{
	Menu_Quit = wxID_EXIT,
	
	Butt_1,
	Butt_2,
	Butt_3,
	Butt_quit,
	
	Checklistbox_1,
};

struct log_struct
{
	log_struct(const timestamp_t stamp_ms, const LogLevel level, const string &msg, const int thread_index)
		: m_Stamp(stamp_ms), m_Lvl(level), m_Msg(msg), m_ThreadIndex(thread_index)
	{
	}
	
	const timestamp_t	m_Stamp;
	const LogLevel		m_Lvl;
	const string		m_Msg;
	const int		m_ThreadIndex;
};

//---- wx Frame ---------------------------------------------------------------

class MyFrame : public wxFrame, public LogSlot
{
public:
	MyFrame(const wxString &title)
		: wxFrame(NULL, wxID_ANY, title, wxDefaultPosition, wxSize(800, 600)),
		m_ThreadID(this_thread::get_id()),
		m_TextCtrl(this, -1, "", wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE | wxTE_READONLY | wxTE_RICH | wxTE_NOHIDESEL | wxTE_DONTWRAP),
		m_CheckListBox(this, Checklistbox_1, wxDefaultPosition, wxDefaultSize),
		m_Button1(this, Butt_1, "button1", wxDefaultPosition, wxDefaultSize),
		m_Button2(this, Butt_2, "button2", wxDefaultPosition, wxDefaultSize),
		m_Button3(this, Butt_3, "button3", wxDefaultPosition, wxDefaultSize),
		m_QuitButton(this, Butt_quit, "Quit", wxDefaultPosition, wxDefaultSize)
	{
		
		CreateStatusBar(1);
		
		InitMenus();
		
		wxFont	ft(wxFontInfo(9).Family(wxFONTFAMILY_MODERN).Encoding(wxFONTENCODING_DEFAULT));
		m_TextCtrl.SetDefaultStyle(wxTextAttr(*wxBLACK, *wxWHITE, ft));
		
		vector<string>	labels;
		
		for (const auto &it : s_LogLevelDefMap)
		{
			const string	s = it.second.m_Label;
			
			labels.push_back(s);
		}
		
		std::sort(labels.begin(), labels.end());

		for (const string &s : labels)
		{
			m_CheckListBox.Append(wxString{s});
		}
	
		wxBoxSizer	*text_n_check_h_sizer = new wxBoxSizer(wxHORIZONTAL);
		
		text_n_check_h_sizer->Add(&m_TextCtrl, wxSizerFlags(4).Border(wxALL, 4).Expand());
		text_n_check_h_sizer->Add(&m_CheckListBox, wxSizerFlags(1).Border(wxALL, 4).Expand());
		
		wxBoxSizer	*but_h_sizer = new wxBoxSizer(wxHORIZONTAL);
		
		but_h_sizer->Add(&m_Button1, wxSizerFlags(0).Border(wxALL, 4).Expand());
		but_h_sizer->Add(&m_Button2, wxSizerFlags(0).Border(wxALL, 4).Expand());
		but_h_sizer->Add(&m_Button3, wxSizerFlags(0).Border(wxALL, 4).Expand());
		but_h_sizer->AddStretchSpacer(1);
		but_h_sizer->Add(&m_QuitButton, wxSizerFlags(0).Border(wxALL, 4).Expand());
		
		wxBoxSizer	*top_v_sizer =  new wxBoxSizer(wxVERTICAL);
		
		top_v_sizer->Add(text_n_check_h_sizer, 1, wxALL | wxEXPAND, 1);
		top_v_sizer->Add(but_h_sizer, 0, wxALL | wxEXPAND, 1);
		
		SetSizer(top_v_sizer);
		
		SetStatusText("Welcome to uLog example!");
		
		Show();
		Centre();
	}
	
	virtual ~MyFrame()
	{
		uLog(DTOR, "MyFrame::DTOR");
	}
	
	void	OnClose(wxCloseEvent &e)
	{
		uLog(USER_CMD, "MyFrame::OnClose()");
		
		e.Skip();
	}
	
// user action
	void	OnQuit(wxCommandEvent &e)
	{
		uLog(USER_CMD, "MyFrame::OnQuit()");
		
		Close(true);
	}
	
	void	OnButton1(wxCommandEvent &e)
	{
		uLog(USER_CMD, "MyFrame::OnButton1()");
		
		uLog(USER1, "user1 id = %d", e.GetId());
	}
	
	void	OnButton2(wxCommandEvent &e)
	{
		uLog(USER_CMD, "MyFrame::OnButton2()");

		uLog(USER2, "user2 id = %d", e.GetId());
	}
	
	void	OnButton3(wxCommandEvent &e)
	{
		uLog(USER_CMD, "MyFrame::OnButton3()");

		uLog(USER3, "user3 id = %d", e.GetId());
	}
	
	void	OnCheckbox(wxCommandEvent &e)
	{
		const int	ind = e.GetInt();
		const string	s = e.GetString().ToStdString();
		
		uMsg("OnCheckbox(ind = %d, %S)", ind, s);
		
		e.Skip();
	}
	
private:
	
	void	InitMenus(void)
	{
		wxMenu *fileMenu = new wxMenu;
		fileMenu->Append(Menu_Quit, "Exit", "Quit this program");
		
		wxMenuBar *menuBar = new wxMenuBar();
		menuBar->Append(fileMenu, "File");
		SetMenuBar(menuBar);
	}
	
	void	LogAtLevel(const timestamp_t stamp, const LogLevel level, const string &msg) override
	{
		unique_lock<mutex>	lock(m_ThreadMutex);
		
		const auto	tid = this_thread::get_id();
		if (tid != m_ThreadID)
		{
			// don't log from other thread or wx BOMBS!
			if (!m_ThreadIndexMap.count(tid))	m_ThreadIndexMap.emplace(tid, m_ThreadIndexMap.size() + 1);
			
			const int	thread_index = m_ThreadIndexMap.at(tid);
			
			m_ThreadLogs.emplace_back(stamp, level, msg, thread_index);
			
			CallAfter(&MyFrame::DequeueLogs);
			
			return;
		}
		
		DequeueLogs();
		
		const wxString	s(xsprintf("%s %s\n", xtimestamp_str(stamp), msg));
		
		SetLevelColor(level);
		
		m_TextCtrl.AppendText(wxString{s});
		
		// (don't log from inside logger!)
	}
	
	void	DequeueLogs(void)
	{
		for (const auto &e : m_ThreadLogs)
		{
			SetLevelColor(e.m_Lvl);
			
			const string	s = xsprintf("%s THR[%1d] %s\n", xtimestamp_str(e.m_Stamp), e.m_ThreadIndex, e.m_Msg);
			
			m_TextCtrl.AppendText(s);
		}
		
		m_ThreadLogs.clear();
	}
	
	void	SetLevelColor(const LogLevel lvl)
	{
		const RGB_COLOR	clr = s_LogLevelDefMap.count(lvl) ? s_LogLevelDefMap.at(lvl).m_Color : RGB_COLOR::BLACK;
	
		const wxColor	wx_clr = Color(clr).ToWxColor();
	
		m_TextCtrl.SetDefaultStyle(wxTextAttr(wx_clr));
	}
	
	const thread::id		m_ThreadID;
	wxTextCtrl			m_TextCtrl;
	wxCheckListBox			m_CheckListBox;
	wxButton			m_Button1, m_Button2, m_Button3, m_QuitButton;
	
	// wxPanel			m_Panel;
	
	mutable mutex			m_ThreadMutex;
	vector<log_struct>		m_ThreadLogs;
	unordered_map<thread::id, int>	m_ThreadIndexMap;
	
	DECLARE_EVENT_TABLE()
};

BEGIN_EVENT_TABLE(MyFrame, wxFrame)
	
	EVT_CLOSE(					MyFrame::OnClose)
	
	EVT_MENU(		Menu_Quit,		MyFrame::OnQuit)
	
	EVT_BUTTON(		Butt_1,			MyFrame::OnButton1)
	EVT_BUTTON(		Butt_2,			MyFrame::OnButton2)
	EVT_BUTTON(		Butt_3,			MyFrame::OnButton3)
	EVT_BUTTON(		Butt_quit,		MyFrame::OnQuit)
	
	EVT_CHECKLISTBOX(	Checklistbox_1,		MyFrame::OnCheckbox)
	
END_EVENT_TABLE()

//-----------------------------------------------------------------------------

class MyApp : public wxApp
{
public:
	
	MyApp(rootLog &rlog)
		: m_RootLog(rlog), m_TopFrame(nil)
	{	
		uLog(MSG, "MyApp::CTOR()");
	}
	
	virtual ~MyApp()
	{	
		uLog(DTOR, "MyApp::DTOR");
	}
	
	bool	OnInit() override
	{
		uLog(MSG, "MyApp::OnInit()");
		
		if (!wxApp::OnInit())        return false;
		
		m_TopFrame = new MyFrame("logger wx");
		
		m_RootLog.Connect(m_TopFrame);
		
		return true;
	}
	
	int	OnExit() override
	{
		uLog(DTOR, "MyApp::OnExit()");
		
		return wxApp::OnExit();
	}
	
	int	OnRun() override
	{
		uLog("APP_INIT", "MyApp::OnRun()");
		
		return wxApp::OnRun();
	}
		
private:
	
	rootLog		&m_RootLog;
	MyFrame		*m_TopFrame;		// (don't mem-manage wx resources)
};

//---- main -------------------------------------------------------------------

int	main(int argc, char* argv[])
{
	static rootLog		s_LogImp;			// don't use STATIC / out-of-order initializaion
	
	s_LogImp.EnableLevels({FATAL, EXCEPTION, ERROR, WARNING, MSG});
	
	// s_LogImp.EnableLevels({DTOR});
	s_LogImp.EnableLevels({APP_INIT});
	s_LogImp.EnableLevels({USER_CMD});
	s_LogImp.EnableLevels({USER1, USER2, USER3});
		
	/*unique_ptr<ddtLog>*/auto	file_log(rootLog::MakeLogType(LOG_TYPE_T::STD_FILE, "wx_logger.log"));
	s_LogImp.Connect(file_log);
		
	uLog(APP_INIT, "main() file log created, creating wx app");
	
	MyApp		*wx_app = new MyApp(s_LogImp);
	(void)wx_app;
	
	uLog(APP_INIT, "main() wx app created, starting wx event loop");
	
	// start wx event loop
	wxEntry(argc, argv);
	
	uLog(APP_INIT, "main(), about to exit main()");
}

// nada mas