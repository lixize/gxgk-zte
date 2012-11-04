
/**************************************************************************************
			The Luzj's Zte Project
			//////////////////////
			Copyleft ? 2009 Luzj
		Author:Luzj		QQ:86829232
		http://blog.csdn.net/luzjqq
		Email: luzjcn@gmail.com
	///////////////////////////////////
关于Luzj's Zte认证端的声明：

1、本软件所有涉及的中兴认证的功能的实现均是通过黑盒分析得来，并未通过任何不正当方法获得。

2、本软件仅供研究学习之用，不得使用本软件损害中兴公司商业利益。

3、本软件不可用于任何商业和非法用途，否则责任自负。

4、本软件在发布前均通过一般性应用测试，但不保证任何情况下对机器无害，
由于未知的使用环境或不当的使用对计算机造成的损害，责任由使用者全部承担。

5.本软件版权没有，翻印不究，但请协助改进本作品。

6.本软件属开源软件，如果需要修改本软件源码以进行二次发布，也请继续公开源代码。

由于任何不遵守上叙条例引起的纠纷，均与本人无关，如不同意该声明请不要使用该软件，谢谢合作。
**************************************************************************************/



// Luzj_ZTEDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Luzj_ZTE.h"
#include "Luzj_ZTEDlg.h"
#include "AutoUpdate.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/////////////////////////////////////////////////////////////////////////////
// CLuzj_ZTEDlg dialog
CLuzj_ZTEDlg::CLuzj_ZTEDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CLuzj_ZTEDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CLuzj_ZTEDlg)

	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	//认证状态为 未认证
	m_bAuth=FALSE;
}

void CLuzj_ZTEDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CLuzj_ZTEDlg)
	DDX_Control(pDX, IDC_USERNAME, m_ccb_username);
	DDX_Control(pDX, IDC_NETCARD, m_ccbNetCard);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CLuzj_ZTEDlg, CDialog)
	//{{AFX_MSG_MAP(CLuzj_ZTEDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_START, OnStart)
	ON_BN_CLICKED(IDC_LOGOFF, OnLogoff)	
	ON_BN_CLICKED(IDC_EXIT, OnExit)
	ON_BN_CLICKED(MENU_SHOW, OnTrayShow)
	ON_BN_CLICKED(MENU_EXIT, OnExit)
	ON_MESSAGE(WM_USER_TRAY_NOTIFICATION,OnTrayNotification)
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_SETTING, OnSetting)
	ON_BN_CLICKED(IDC_LOGSHOW, OnLogshow)
	ON_CBN_SELCHANGE(IDC_USERNAME, OnSelchangeUsername)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


int CLuzj_ZTEDlg::CheckUpdate()
{
	int ret = AutoUpdate();
	if(ret == 0) {
		if(this->m_bAuth) {
			this->OnLogoff();
		}
		this->PostMessage(WM_QUIT);
	//	Sleep(1000);
		TerminateProcess(GetCurrentProcess(), 0);
	} else if(ret == 4) { return 2; }

	return 1;
}

BOOL CLuzj_ZTEDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	
	SetWindowText(STR_AppName);
	m_bAuth = FALSE;

	editLog = (CEdit*)GetDlgItem(IDC_EDIT_LOG);

	//创建托盘图标
	m_tray.Create(this, WM_USER_TRAY_NOTIFICATION,STR_AppName, m_hIcon, 0);

	//修改列表控件
	GetWindowRect(&m_rc);
	m_rc.top=m_rc.bottom-5;    //设置状态栏的矩形区域
	m_StatusBar.Create(WS_CHILD |WS_VISIBLE|CBRS_BOTTOM,m_rc,this,20000);  
	int nParts[2]= {100,-1};      //分割尺寸
	m_StatusBar.SetParts(2, nParts);
	m_StatusBar.SetText("00:00:00",0,0);
	m_StatusBar.SetText("        状态:  未认证",1,0);

	//控制状态栏的显示
	SetTimer(1,1000,NULL);
	//开始的时候先将日志框隐藏
	OnLogshow();

	//////////////////////////////////////////////////////////////////////////
	char szTemp[MAX_STRING];
	int i=0,k=0;
	CString str;

	//读取配置到文件配置对象中去
	Config.LoadConfig();
	//if(Config.m_bAutoUpdate) CheckUpdate();

	CheckDlgButton(IDC_REMEMBER,Config.m_bRememberPWD?BST_CHECKED:BST_UNCHECKED);
	//////////////////////////////////////////////////////////////////////////
	//加载账号信息
	CString user,pass;	
	POSITION p = Config.m_UserInfo.GetStartPosition();	
	while(p != NULL) {
		Config.m_UserInfo.GetNextAssoc(p, user, pass);		
		m_ccb_username.AddString(user);		
	}
	k = m_ccb_username.FindStringExact(-1, Config.m_csLastUser);
	if(k < 0) k = 0;
	if(!Config.m_UserInfo.IsEmpty()) {
		m_ccb_username.SetCurSel(k);
		m_ccb_username.GetWindowText(user);
		GetDlgItem(IDC_PWD)->SetWindowText(Config.m_UserInfo[user]);		
	}
	

	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	//加载网卡信息
	char m_errorBuffer[ PCAP_ERRBUF_SIZE ];		//错误信息缓冲区
	pcap_if_t		* allAdapters;				//适配器列表
	if(pcap_findalldevs(&allAdapters, m_errorBuffer) == -1 || allAdapters == NULL)
	{
		MessageBox("读取网卡信息失败，请确保你安装了WinPcap!","错误",MB_ICONERROR|MB_OK);
		pcap_freealldevs(allAdapters);
		CDialog::OnCancel();
	}

	pcap_if_t* adapter;//临时存放适配器
    for(adapter = allAdapters; adapter != NULL; adapter = adapter->next) {
		if(adapter->flags & PCAP_IF_LOOPBACK) continue;
		m_ccbNetCard.AddString(adapter->description);
    }
	pcap_freealldevs(allAdapters);


	m_ccbNetCard.SetCurSel(0);
	for (i=0;i<m_ccbNetCard.GetCount();i++)	{
		m_ccbNetCard.GetLBText(i,szTemp);
		if (strcmp(szTemp,Config.m_csNetCard)==0) {
			m_ccbNetCard.SetCurSel(i);
			break;
		}
	}
	
	this->Log("加载网卡完成");
	//////////////////////////////////////////////////////////////////////////

	//使得开始按钮有效，而断开按钮无效
	UpdateStatus(FALSE);
	
	if (Config.m_bAutologon == TRUE)
	{
		ShowWindow(SW_HIDE);
		this->OnStart();
	}
	SetProcessWorkingSetSize(GetCurrentProcess(),-1,-1);
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CLuzj_ZTEDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == SC_MINIMIZE || (nID & 0xFFF0) == SC_CLOSE)
	{
		ShowWindow(SW_HIDE);
		SetProcessWorkingSetSize(GetCurrentProcess(),-1,-1);
		SetBubble("提示",STR_AppName" 正在后台运行着……",1);
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}
void CLuzj_ZTEDlg::SetBubble(char * title,char * content,int timeout)
{
	if (Config.m_bShowBubble==TRUE)
	{
		m_tray.SetBubble(title,content,timeout);
	}
}

void CLuzj_ZTEDlg::OnTrayShow()
{   
	if (IsWindowVisible())
	{
		ShowWindow(SW_HIDE);
		SetProcessWorkingSetSize(GetCurrentProcess(),-1,-1);
	} 
	else
	{
		ShowWindow(SW_SHOW);
	}
}


LONG CLuzj_ZTEDlg::OnTrayNotification(WPARAM wparam, LPARAM lparam)
{   
	CMenu	m_menu;
	CPoint	m_point;
	switch (lparam)
	{
	case WM_RBUTTONDOWN:
		m_menu.CreatePopupMenu();
		if (IsWindowVisible())
		{
			m_menu.AppendMenu(MF_ENABLED, MENU_SHOW,_T("隐藏窗口"));
		} 
		else
		{
			m_menu.AppendMenu(MF_ENABLED, MENU_SHOW,_T("显示窗口"));
		}
		m_menu.AppendMenu(MF_ENABLED, MF_SEPARATOR);
		m_menu.AppendMenu(MF_ENABLED, MENU_EXIT,_T("退 出"));
		GetCursorPos(&m_point);
		m_menu.TrackPopupMenu(TPM_LEFTALIGN,m_point.x,m_point.y,this);
		break;
	case WM_LBUTTONDOWN:
		if (IsWindowVisible())
		{
			ShowWindow(SW_HIDE);
			SetProcessWorkingSetSize(GetCurrentProcess(),-1,-1);
		}
		else
		{
			ShowWindow(SW_SHOW);
			BringWindowToTop();
		}	
		break;
	}
	return 0;
}

void CLuzj_ZTEDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CLuzj_ZTEDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

void CLuzj_ZTEDlg::OnStart() 
{
	UpdateStatus(TRUE);

	//////////////////////////////////////////////////////////////////////////
	//		先存放设置参数
	CString strTemp;

	//取得用户名密码
	GetDlgItem(IDC_USERNAME)->GetWindowText((char*)m_username, sizeof(m_username));
	m_usernameLen=strlen((char*)m_username);
	GetDlgItem(IDC_PWD)->GetWindowText((char*)m_password, sizeof(m_password));
	m_passwordLen=strlen((char*)m_password);

	
	if (m_usernameLen<1 || m_passwordLen<1)	{
		this->Log("用户名或者密码太短!"); UpdateStatus(FALSE); return;	
	}
	
	GetDlgItem(IDC_NETCARD)->GetWindowText(strTemp);
	Config.m_csNetCard=strTemp;
	
	if (IsDlgButtonChecked(IDC_REMEMBER)) {		
		Config.m_bRememberPWD=TRUE;
	} else {		
		Config.m_bRememberPWD=FALSE;
	}

	Config.m_csLastUser=(char*)m_username;
	Config.m_UserInfo[(char*)m_username]=(char*)m_password;

	Config.SaveConfig();
	//////////////////////////////////////////////////////////////////////////
	SetBubble("提示",STR_AppName" 开始联网认证……");
		
	char m_errorBuffer[PCAP_ERRBUF_SIZE];
	int retcode = 0;
		
	/////////////////////////////////////////////////////////////////////////
	//寻找所选的网卡的MAC	
	if (GetMacIP(Config.m_csNetCard, m_ip, m_MacAdd) != 0)	{
		if(m_MacAdd[0] == 0 && m_MacAdd[1] == 0 && m_MacAdd[2] == 0 && 
			m_MacAdd[3] == 0 && m_MacAdd[4] == 0 && m_MacAdd[5] == 0) {
			Log("GetMacIP:no mac address."); UpdateStatus(FALSE);
			return;
		}
	} else {
		Log("MAC:%02X-%02X-%02X-%02X-%02X-%02X", m_MacAdd[0], m_MacAdd[1], m_MacAdd[2], 
			m_MacAdd[3], m_MacAdd[4], m_MacAdd[5]);
	}
	//////////////////////////////////////////////////////////////////////////
	// 打开指定适配器
	m_adapterHandle=pcap_open_live(DescriptionToName(Config.m_csNetCard),65536,1,Config.m_iTimeout,m_errorBuffer);
    if(m_adapterHandle == NULL) {	
		Log("pcap_open_live:%s", m_errorBuffer); UpdateStatus(FALSE);
		return;
    }

	char	FilterStr[100];		//包过滤字符串
	struct bpf_program	mfcode;	

	sprintf(FilterStr, "(ether proto 0x888e) and (ether dst host %02x:%02x:%02x:%02x:%02x:%02x)",
			m_MacAdd[0],m_MacAdd[1],m_MacAdd[2],m_MacAdd[3],m_MacAdd[4],m_MacAdd[5]);

    if((retcode=pcap_compile(m_adapterHandle, &mfcode, FilterStr, 1, 0xff))==-1
		||(retcode=pcap_setfilter(m_adapterHandle, &mfcode))==-1)
    {
		Log("pcap_compile & pcap_setfilter:%s", pcap_strerror(retcode));
		pcap_close(m_adapterHandle); UpdateStatus(FALSE);
		return;
    }

	CPacket packet;	
	//////////////////////////////////////////////////////////////////////////
	///开始认证包
    if(!(retcode=packet.send_packet_start(m_adapterHandle,m_MacAdd))) {   
		Log("send_packet_start:(%d)%s", retcode, pcap_geterr(m_adapterHandle));	 UpdateStatus(FALSE);	
		return;
    }
	Log("Client:EAPOL_START...");

	m_AuthThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)eap_thread, this, 0, 0);
}

DWORD WINAPI CLuzj_ZTEDlg::eap_thread(void *para)
{
	CLuzj_ZTEDlg *Dlg = (CLuzj_ZTEDlg*)para;	

	pcap_loop(Dlg->m_adapterHandle,-1,CLuzj_ZTEDlg::get_packet,(u_char*)Dlg);

	pcap_close(Dlg->m_adapterHandle);
	return 0;
}

    

DWORD WINAPI CLuzj_ZTEDlg::GetMacIP(const char *adaptername, char ip[16], unsigned char mac[6])
{

	PIP_ADAPTER_INFO AdapterInfo = NULL;

	DWORD dwBufLen = sizeof(IP_ADAPTER_INFO);
	DWORD dwStatus;
	AdapterInfo = (PIP_ADAPTER_INFO)malloc(dwBufLen);
	if(AdapterInfo == NULL) return -1;
	dwStatus = GetAdaptersInfo(AdapterInfo,&dwBufLen);	
	if(dwStatus == ERROR_BUFFER_OVERFLOW) {
		free(AdapterInfo); 
		AdapterInfo = (PIP_ADAPTER_INFO)malloc(dwBufLen);
		dwStatus = GetAdaptersInfo(AdapterInfo,&dwBufLen);	
		if(AdapterInfo == NULL) return -1;
	}
	if(dwStatus != NO_ERROR) {
		if(AdapterInfo != NULL) free(AdapterInfo);
		return -2;
	}

	pcap_if_t* adapter;							//临时存放适配器
	pcap_if_t* allAdapters;						//适配器列表
	
	if(pcap_findalldevs(&allAdapters, NULL) == -1 || allAdapters == NULL) {		
		free(AdapterInfo); pcap_freealldevs(allAdapters);	return -1;
	}
	
	memset(ip, 0, 16);	memset(mac, 0, 6);

    for(adapter = allAdapters; adapter != NULL; adapter= adapter->next) {
		if (strcmp(adapter->description,adaptername)==0){
			break;
		}
    }
 	
	if(adapter != NULL) {
		PIP_ADAPTER_INFO pAdapterInfo = AdapterInfo;		
		while(pAdapterInfo) {
			if (strstr(adapter->name,pAdapterInfo->AdapterName) >= 0) {
				memcpy(mac, pAdapterInfo->Address, 6);
				strncpy(ip, pAdapterInfo->IpAddressList.IpAddress.String, 16);
				break;
			}
			pAdapterInfo = pAdapterInfo->Next;
		};
	}
	
	free(AdapterInfo);	pcap_freealldevs(allAdapters);
	
	return 0;
}

DWORD WINAPI CLuzj_ZTEDlg::IpconfigRenew()
{
	
	SHELLEXECUTEINFO ShExecInfo = {0};
	TCHAR tempBuffer[MAX_STRING];
	CString strSystemDirectory;
	GetSystemDirectory(tempBuffer, MAX_STRING);
	strSystemDirectory.Format(_T("%s\\ipconfig.exe"),tempBuffer);
	ShExecInfo.cbSize	= sizeof(SHELLEXECUTEINFO);
	ShExecInfo.fMask		= SEE_MASK_NOCLOSEPROCESS;
	ShExecInfo.lpFile		= strSystemDirectory;		
	ShExecInfo.lpParameters = _T("/renew");	
	ShExecInfo.nShow	= SW_HIDE;
	ShellExecuteEx(&ShExecInfo);
	
	int iRnt =WaitForSingleObject(ShExecInfo.hProcess,Config.m_iTimeout*1000);
	if (iRnt == WAIT_TIMEOUT || iRnt == WAIT_FAILED) return -1;
	
	return 0;
}

void CLuzj_ZTEDlg::Log (const char *fmt, ...)
{
    va_list args;
    char msg[MAX_STRING];
    va_start (args, fmt);
    vsnprintf (msg, MAX_STRING, fmt, args);
    va_end (args);

	time_t t=time(NULL);
	char szTime[MAX_STRING];
	strftime(szTime,MAX_STRING,"%H:%M:%S",localtime(&t));	
	strcat(szTime, " "); strcat(szTime, msg); strcat(szTime, "\r\n");

	int nLength = editLog->SendMessage(WM_GETTEXTLENGTH);
    editLog->SetSel(nLength, nLength);
    editLog->ReplaceSel(szTime);
	editLog->SendMessage(WM_VSCROLL,SB_BOTTOM,0);
}

char* CLuzj_ZTEDlg::DescriptionToName(const char *description)
{
	pcap_if_t* adapter;							//临时存放适配器
	pcap_if_t* allAdapters;						//适配器列表
	char m_errorBuffer[ PCAP_ERRBUF_SIZE ];		//错误信息缓冲区
	static char name[MAX_STRING];
	if(pcap_findalldevs(&allAdapters, m_errorBuffer) == -1 || allAdapters == NULL)	{		
		pcap_freealldevs(allAdapters);
		return NULL;
	}
    for(adapter = allAdapters; adapter != NULL; adapter= adapter->next)
    {
		if (strcmp(adapter->description,Config.m_csNetCard)==0)
		{
			break;
		}
    }
	if(adapter != NULL) strncpy(name, adapter->name, MAX_STRING);
	pcap_freealldevs(allAdapters);
	return name;
}

char * CLuzj_ZTEDlg::HttpAuth(BOOL bForce = FALSE)
{
	static char *msg = NULL;
	if((Config.m_bWebAuth && Config.m_csWebAuthUrl.GetLength() > 0) || bForce) {
		if(Config.m_bEnableWebAccount) {
			msg = WebAuth(Config.m_csWebUsername, Config.m_csWebPassword, m_ip, Config.m_csWebAuthUrl);
		} else {
			msg = WebAuth((const char *)m_username, (const char *)m_password, m_ip, Config.m_csWebAuthUrl);
		}
	}
	return msg;
}

void CLuzj_ZTEDlg::UpdateStatus(BOOL bOnline)
{
	
	const char *m2[] = {"Offline", "Online"};
	m_bAuth = bOnline;
	int i = (bOnline ? 1 : 0);
	
	
	this->Log("status:%s", m2[i]);

	if(m_bAuth) {
		m_startTime = time(NULL);
		ShowWindow(SW_HIDE);
	}

	GetDlgItem(IDC_USERNAME)->EnableWindow(!bOnline);
	GetDlgItem(IDC_PWD)->EnableWindow(!bOnline);
	GetDlgItem(IDC_NETCARD)->EnableWindow(!bOnline);
	GetDlgItem(IDC_START)->EnableWindow(!bOnline);

	GetDlgItem(IDC_LOGOFF)->EnableWindow(bOnline);

}

void CLuzj_ZTEDlg::get_packet(u_char *args, const struct pcap_pkthdr *pcaket_header, const u_char *packet)
{
	int retcode;
	static int good = 0;
	CLuzj_ZTEDlg *Dlg = (CLuzj_ZTEDlg*)args;
	/* declare pointers to packet headers */
	CPacket P;
	if(packet[15] == 0x00) { //eap_packet
		if(packet[18] == 0x01) {//request
			if(packet[22] == 0x01) { //identity
				Dlg->Log("Server:Request Identity...");
				if(!(retcode=P.send_packet_response_Identity(Dlg->m_adapterHandle,
					packet,Dlg->m_MacAdd,Dlg->m_username,Dlg->m_usernameLen)))	{
					Dlg->Log("send_packet_response_Identity:(%d)%s", retcode, pcap_geterr(Dlg->m_adapterHandle));					
					Dlg->UpdateStatus(FALSE); return;					
				}
				Dlg->Log("Client:Response Identity:%s", Dlg->m_username);
			} else if(packet[22] == 0x04) {//md5 challenge
				Dlg->Log("Server:Request MD5 Chanllenge...");
				if(!(retcode=P.send_packet_response_MD5(Dlg->m_adapterHandle,packet,Dlg->m_MacAdd,
					Dlg->m_username,Dlg->m_usernameLen,Dlg->m_password,Dlg->m_passwordLen))) {
					Dlg->Log("send_packet_response_MD5:(%d)%s", retcode, pcap_geterr(Dlg->m_adapterHandle));
					Dlg->UpdateStatus(FALSE); return;
				}
				Dlg->Log("Client:Response MD5 Chanllenge");				
			} else {//unknown
				Dlg->Log("Unknown EAPOL Type:0x%02X", packet[22]);
			}
		} else if(packet[18] == 0x03) {//successful
			SetProcessWorkingSetSize(GetCurrentProcess(),-1,-1);
			Dlg->UpdateStatus(TRUE);
			retcode = Dlg->IpconfigRenew();
			if(retcode != 0) Dlg->Log("Ipconfig/Renew return %d with timeout %d", retcode, Config.m_iTimeout);

			if (Dlg->GetMacIP(Config.m_csNetCard, Dlg->m_ip, Dlg->m_MacAdd) != 0)	{
				Dlg->Log("GetMacIP:no IP address.");		
			} else {
				Dlg->Log("IP:%s", Dlg->m_ip);
			}
			
			if(Config.m_bWebAuth && Config.m_csWebAuthUrl.GetLength() > 0){
				char *msg; int i;
				for(i = 0; i < 3; i++) {
					msg = Dlg->HttpAuth();
					if(msg == NULL) {							
						Dlg->Log("网页认证成功!");	
						if(Config.m_bAutoUpdate) Dlg->CheckUpdate();
						break;
					} else {				
						Dlg->Log("HttpAuth:%s", msg);			
					}
					Sleep(3000);
				}
				if(i >= 3) {
					Dlg->Log("网页认证失败，请手动认证");
				}
			}
		} else if(packet[18] == 0x04) {//fail
			Dlg->UpdateStatus(FALSE);
			AfxMessageBox("服务器认证失败,请查看日志信息!");			
			Dlg->Log("failed...");
			Dlg->Log((char*)packet+24);	
			return;
		} else {//unknown
			Dlg->Log("Unknown EAP Type:0x%02X", packet[18]);
		}
	} else if(packet[15] == 0x03) {//key		
		if(!(retcode=P.send_packet_key2(Dlg->m_adapterHandle,packet,Dlg->m_MacAdd))) {
			Dlg->Log("send_packet_key2:(%d)%s", retcode, pcap_geterr(Dlg->m_adapterHandle));
			Dlg->UpdateStatus(FALSE);  return;
		}	
		if(good <= 3) Dlg->Log(good++ == 3 ? "正常在线。" : "send_packet_key2...");					
		Dlg->HttpAuth();
	} else {//unknown
		Dlg->Log("Unknown Type:0x%02X", packet[15]);
	}
   
    return;
}

//========================OnLogoff======================================
void CLuzj_ZTEDlg::OnLogoff() 
{
	if(Config.m_bWebLogout && strlen(Config.m_csWebLogoutUrl) > 0) {
		char *msg = WebLogout(this->m_ip, Config.m_csWebLogoutUrl);
		if(msg == NULL) {
			this->Log("网页认证已下线!");
		} else {
			this->Log("WebLogout:%s", msg);			
		}
	}

	CPacket packet;
	packet.send_packet_logoff(m_adapterHandle,m_MacAdd);
	pcap_breakloop(m_adapterHandle);
	//pcap_close(m_adapterHandle);

	this->UpdateStatus(FALSE);

	//等待，如果500ms后没有自动退出，则强制结束
	if (::WaitForSingleObject(this->m_AuthThread,500) == WAIT_TIMEOUT)	
	{
		::TerminateThread(this->m_AuthThread ,0);
	}
}
//========================OnLogoff======================================


//////////////////////////////////////////////////////////////////////////
//响应退出按钮，如果已经认证，就先发送退出认证包
void CLuzj_ZTEDlg::OnExit() 
{
	ShowWindow(SW_HIDE);
	if (m_bAuth) {		
		this->OnLogoff();
	}
	CDialog::OnOK();
}
//////////////////////////////////////////////////////////////////////////


void CLuzj_ZTEDlg::OnTimer(UINT nIDEvent)
{
	time_t t =time(NULL);
	char szTime[MAX_STRING];
	strftime(szTime,MAX_STRING,"%H:%M:%S", localtime(&t));
	m_StatusBar.SetText(szTime,0,0);

	if(m_bAuth && Config.m_bReauth && strcmp(szTime, Config.m_csReauthTime) == 0) {
		OnLogoff();
		OnStart();
	}

	if (m_bAuth) {
		t -= m_startTime;
		strftime(szTime,MAX_STRING,"状态: 在线时间%H:%M:%S",gmtime(&t));
		m_StatusBar.SetText(szTime,1,0);
	} else {
		m_StatusBar.SetText("状态:  未联网",1,0);
	}
		
	CDialog::OnTimer(nIDEvent);
}

void CLuzj_ZTEDlg::OnLogshow() 
{
	static bShow=TRUE;
	RECT rc1={0},rc2={0};
	if (bShow)
	{
		GetDlgItem(IDC_SPLIT)->GetWindowRect(&rc1);
		GetWindowRect(&m_rc);
		rc2=m_rc;
		rc2.right=rc1.left-2;
		MoveWindow(&rc2,TRUE);
		bShow=!bShow;
		GetDlgItem(IDC_LOGSHOW)->SetWindowText("日志>>");
	}
	else
	{
		GetWindowRect(&rc1);
		
		m_rc.right+=rc1.left-m_rc.left;
		m_rc.bottom+=rc1.top-m_rc.top;
		m_rc.left=rc1.left;
		m_rc.top=rc1.top;

		MoveWindow(&m_rc,TRUE);
		bShow=!bShow;
		GetDlgItem(IDC_LOGSHOW)->SetWindowText("日志<<");
	}
}

void CLuzj_ZTEDlg::OnSelchangeUsername() 
{
	CString str;
	m_ccb_username.GetLBText(m_ccb_username.GetCurSel(), str);	
	GetDlgItem(IDC_PWD)->SetWindowText(Config.m_UserInfo[str]);	
}

void CLuzj_ZTEDlg::OnSetting() 
{
	CSettingDlg dlg;
	if (dlg.DoModal())
	{
	}
}
