
// RemoteClientDlg.cpp: 实现文件
//

#include "pch.h"
#include "framework.h"
#include "RemoteClient.h"
#include "RemoteClientDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CRemoteClientDlg 对话框



CRemoteClientDlg::CRemoteClientDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_REMOTECLIENT_DIALOG, pParent)
	, m_server_address(0)
	, m_port(_T(""))
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CRemoteClientDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_IPAddress(pDX, IDC_IPADDRESS_SERVER, m_server_address);
	DDX_Text(pDX, IDC_SERVER_PORT, m_port);
	DDX_Control(pDX, IDC_TREE_DIR, m_tree);
	DDX_Control(pDX, IDC_LIST_FILE, m_list);
}

BEGIN_MESSAGE_MAP(CRemoteClientDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BTN_TEST, &CRemoteClientDlg::OnBnClickedBtnTest)
	ON_BN_CLICKED(IDC_BTN_FILEINFO, &CRemoteClientDlg::OnBnClickedBtnFileinfo)
	ON_NOTIFY(NM_DBLCLK, IDC_TREE_DIR, &CRemoteClientDlg::OnNMDblclkTreeDir)
	ON_NOTIFY(NM_CLICK, IDC_TREE_DIR, &CRemoteClientDlg::OnNMClickTreeDir)
	ON_NOTIFY(NM_RCLICK, IDC_LIST_FILE, &CRemoteClientDlg::OnNMRClickListFile)
	ON_COMMAND(ID_OPENFILE, &CRemoteClientDlg::OnOpenfile)
	ON_COMMAND(ID_DOWNLOADFILE, &CRemoteClientDlg::OnDownloadfile)
	ON_COMMAND(ID_DELETEFILE, &CRemoteClientDlg::OnDeletefile)
	ON_MESSAGE(WM_SEND_PACKET,&CRemoteClientDlg::OnSendPacket)
	ON_MESSAGE(WM_SEND_PROGRESS, &CRemoteClientDlg::OnSendProgress)

END_MESSAGE_MAP()


// CRemoteClientDlg 消息处理程序

BOOL CRemoteClientDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码
	UpdateData();
	m_server_address = 0x7F000001;
	m_port = _T("9527");
	UpdateData(false);

	m_statusDlg.Create(IDD_DOWNLOAD_DIALOG, this);
	m_statusDlg.ShowWindow(SW_HIDE);
	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CRemoteClientDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CRemoteClientDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CRemoteClientDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

int CRemoteClientDlg::SendCommandPacket(int nCmd,bool bAutoClose, BYTE* pData, size_t nLength)
{
	UpdateData();
	CClientSocket* pClient = CClientSocket::getInstance();
	int ret = pClient->InitSocket(m_server_address, atoi((LPCTSTR)m_port));	//TODO:返回值的处理
																			//LPCTSTR --- const char*
	if (!ret) {
		AfxMessageBox("网络初始化失败");
		return -1;
	}
	CPacket pack(nCmd, pData, nLength);
	pClient->Send(pack);
	int cmd = pClient->DealCommand();
	TRACE("ack:%d\r\n", pClient->GetPacket().sCmd);
	if(bAutoClose)
		pClient->CloseSocket();
	return cmd;
}

void CRemoteClientDlg::OnBnClickedBtnTest()
{
	// TODO: 在此添加控件通知处理程序代码
	SendCommandPacket(95);
}


void CRemoteClientDlg::OnBnClickedBtnFileinfo()
{
	// TODO: 在此添加控件通知处理程序代码
	int ret = SendCommandPacket(1);
	if (ret == -1) {
		AfxMessageBox(_T("命令处理失败！"));
	}
	CClientSocket* pClient = CClientSocket::getInstance();
	std::string drivers = pClient->GetPacket().strData;
	std::string dr;
	m_tree.DeleteAllItems();
	for (size_t i = 0; i < drivers.size(); ++i) {
		if (drivers[i] == ',') {
			dr += ":";
			HTREEITEM hTmp = m_tree.InsertItem(dr.c_str(), TVI_ROOT, TVI_LAST);
			m_tree.InsertItem(0, hTmp, TVI_LAST);
			dr.clear();
			continue;
		}
		dr += drivers[i];
	}
}

CString CRemoteClientDlg::GetPath(HTREEITEM hTree) {
	CString strRet, strTmp;
	do {
		strTmp = m_tree.GetItemText(hTree);
		strRet = strTmp + '\\' + strRet;
		hTree = m_tree.GetParentItem(hTree);
	} while (hTree != NULL);
	return strRet;
}

void CRemoteClientDlg::DeleteTreeChildrenItem(HTREEITEM hTree)
{
	HTREEITEM hSub = NULL;
	do {
		hSub = m_tree.GetChildItem(hTree);
		if (hSub != NULL) m_tree.DeleteItem(hSub);
	} while (hSub != NULL);
}

/// <summary>
/// 封装的文件树控件点击绘制方法
/// </summary>
void CRemoteClientDlg::LoadFileInfo()
{
	CPoint ptMouse;
	GetCursorPos(&ptMouse);		//设备端坐标，屏幕左上角为(0,0)。该坐标无法检测树控件的点击，树控件是相对于客户区存在的
	m_tree.ScreenToClient(&ptMouse);	//因此要转换为客户端
	HTREEITEM hTreeSelected = m_tree.HitTest(ptMouse, 0);	//检测点击点是否有东西
	if (hTreeSelected == NULL) {	//啥都没点到
		return;
	}
	//bool bRet = m_tree.ItemHasChildren(hTreeSelected);
	//if (m_tree.GetChildItem(hTreeSelected) == NULL) return;	//文件没有子目录，直接卡死文件。date4.26，这会导致一个bug，当文件夹下只有文件时，这个文件夹被点开后，再点就在这被拦截了，认为该文件夹没有子目录，不会进行处理
	DeleteTreeChildrenItem(hTreeSelected);	//重复点击时，删除子树重绘
	m_list.DeleteAllItems();
	CString strPath = GetPath(hTreeSelected);
	int nCmd = SendCommandPacket(2, false, (BYTE*)(LPCSTR)strPath, strPath.GetLength());
	CClientSocket* pClient = CClientSocket::getInstance();
	PFileInfo pInfo = (PFileInfo)pClient->GetPacket().strData.c_str();	//如果下面有货，接第一个文件/文件夹
	while (pInfo->hasNext) {
		TRACE("[%s],idDir:%d\r\n", pInfo->szFileName, pInfo->isDirectory);
		if (pInfo->isDirectory) {
			if (CString(pInfo->szFileName) == "." || CString(pInfo->szFileName) == "..") {
				int cmd = pClient->DealCommand();
				TRACE("ack:%d\r\n", cmd);
				if (cmd < 0)break;
				PFileInfo pInfo = (PFileInfo)pClient->GetPacket().Data();
				continue;
			}
			HTREEITEM hTmp = m_tree.InsertItem(pInfo->szFileName, hTreeSelected, TVI_LAST);
			m_tree.InsertItem("", hTmp, TVI_LAST);	//让目录看起来是有子节点的架势

		}
		else {
			m_list.InsertItem(0, pInfo->szFileName);
		}

		int cmd = pClient->DealCommand();
		TRACE("ack:%d\r\n", cmd);
		if (cmd < 0)break;
		PFileInfo pInfo = (PFileInfo)pClient->GetPacket().strData.c_str();//如果hasNext为true，接后续文件/文件夹
		//if (!(pInfo->hasNext)) {	//date4.26，如果非要用GetChildItem判断是否有子目录，可以在这强行加一个空子目，但不好看，还是觉得直接别GetChildItem检查的好，毕竟文件都不放在树里。
		//	m_tree.InsertItem(NULL, hTreeSelected, TVI_LAST);	//让目录看起来是有子节点的架势
		//}
	}
	pClient->CloseSocket();
}

void CRemoteClientDlg::LoadFileCurrent()
{
	HTREEITEM hTree = m_tree.GetSelectedItem();
	CString strPath = GetPath(hTree);
	m_list.DeleteAllItems();
	int nCmd = SendCommandPacket(2, false, (BYTE*)(LPCSTR)strPath, strPath.GetLength());
	CClientSocket* pClient = CClientSocket::getInstance();
	PFileInfo pInfo = (PFileInfo)pClient->GetPacket().strData.c_str();	//如果下面有货，接第一个文件/文件夹
	while (pInfo->hasNext) {
		TRACE("[%s],idDir:%d\r\n", pInfo->szFileName, pInfo->isDirectory);
		if (!pInfo->isDirectory) {
			m_list.InsertItem(0, pInfo->szFileName);
		}
		int cmd = pClient->DealCommand();
		TRACE("ack:%d\r\n", cmd);
		if (cmd < 0)break;
		PFileInfo pInfo = (PFileInfo)pClient->GetPacket().strData.c_str();//如果hasNext为true，接后续文件/文件夹
	}
}

void CRemoteClientDlg::ThreadForDownloadFile(void* args)
{
	CRemoteClientDlg* pCliDlg = (CRemoteClientDlg*)args;
	pCliDlg->DownloadFile();
	_endthread();
}

void CRemoteClientDlg::DownloadFile() {
	int nListSelected = m_list.GetSelectionMark();
	CString strFile = m_list.GetItemText(nListSelected, 0);
	CFileDialog dlg(FALSE, "*", strFile, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, NULL, this);	//CFileDialog类,封装用于文件打开操作或文件保存操作的常见对话框。
	if (dlg.DoModal() == IDOK) {
		CClientSocket* pClient = CClientSocket::getInstance();

		FILE* pFile;
		fopen_s(&pFile, dlg.GetPathName(), "wb+");
		if (pFile == NULL) {
			AfxMessageBox("本地没有权限保存该文件，或文件无法创建！");
			return;
		}
		HTREEITEM hTreeSelected = m_tree.GetSelectedItem();
		strFile = GetPath(hTreeSelected) + strFile;
		TRACE("%s\r\n", LPCSTR(strFile));
		do {
			//int ret = SendCommandPacket(4, false, (BYTE*)(LPCSTR)strFile, strFile.GetLength());
			int ret = SendMessage(WM_SEND_PACKET, 4 << 1 | 0, (LPARAM)(LPCSTR)strFile);
			if (ret < 0) {
				AfxMessageBox("执行下载命令失败！");
				TRACE("error ret:%d\r\n", ret);
				return;
			}
			long long nLength = *(long long*)CClientSocket::getInstance()->GetPacket().strData.c_str();	//先接收文件的总长
			if (nLength == 0) {
				AfxMessageBox("文件长度为0，无法读取");
				return;
			}
			CStatusDlg m_statusDlg22;
			m_statusDlg22.Create(IDD_DOWNLOAD_DIALOG);
			m_statusDlg22.ShowWindow(SW_SHOW);
			m_statusDlg22.CenterWindow(this);		//弹窗居中
			m_statusDlg22.SetActiveWindow();
			long long nCount = 0;
			/*CWnd* downloadBar = GetDlgItem(IDC_PROGRESS_DOWNLOAD);*/
			while (nCount < nLength) {	//在1k 1k地接收文件
				ret = pClient->DealCommand();
				if (ret < 0) {
					AfxMessageBox("传输失败！");
					TRACE("传输失败 ret:%d\r\n", ret);
					m_statusDlg22.ShowWindow(SW_HIDE);

					break;
				}
				fwrite(pClient->GetPacket().strData.c_str(), 1, pClient->GetPacket().strData.size(), pFile);
				nCount += pClient->GetPacket().strData.size();
				double progressData = (double)nCount * 100 / nLength;
				CString str;
				str.Format(_T("已下载：%.2lf"), progressData);
				//SendMessage(WM_SEND_PROGRESS, NULL, (LPARAM)(LPCSTR)str);
				m_statusDlg22.m_info = str;
				m_statusDlg22.UpdateData(FALSE);
			}
			m_statusDlg22.ShowWindow(SW_HIDE);
			MessageBox(_T("下载成功！"));

		} while (false);	//⭐tip
		fclose(pFile);
		pClient->CloseSocket();
	}
}

void CRemoteClientDlg::OnNMDblclkTreeDir(NMHDR* pNMHDR, LRESULT* pResult)
{
	// TODO: 在此添加控件通知处理程序代码
	*pResult = 0;
	LoadFileInfo();
}


void CRemoteClientDlg::OnNMClickTreeDir(NMHDR* pNMHDR, LRESULT* pResult)
{
	// TODO: 在此添加控件通知处理程序代码
	*pResult = 0;
	LoadFileInfo();

}


void CRemoteClientDlg::OnNMRClickListFile(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码
	*pResult = 0;
	CPoint ptMouse,ptList;		//保留相对屏幕的坐标，后面定位菜单浮动位置用
	GetCursorPos(&ptMouse);		//设备端坐标，屏幕左上角为(0,0)。
	ptList = ptMouse;
	m_list.ScreenToClient(&ptList);	//因此要转换为相对于客户端的坐标
	int nListSelected = m_list.HitTest(ptList);	//检测点击处是否有东西
	if (nListSelected < 0) return;
	CMenu menu;
	menu.LoadMenu(IDR_MENU_RCLICK);	//载入菜单
	CMenu* pRMenu = menu.GetSubMenu(0);	//绑定第一个子菜单
	if(pRMenu != NULL) {
		pRMenu->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, ptMouse.x, ptMouse.y, this);	//TrackPopupMenu--在指定位置显示浮动弹出菜单，并跟踪弹出菜单上项的选择情况
	}

}

/// <summary>
/// 在远端打开文件
/// </summary>
void CRemoteClientDlg::OnOpenfile()
{
	// TODO: 在此添加命令处理程序代码
	int nListSelected = m_list.GetSelectionMark();
	CString strFile = m_list.GetItemText(nListSelected, 0);
	HTREEITEM hTreeSelected = m_tree.GetSelectedItem();
	strFile = GetPath(hTreeSelected) + strFile;
	int ret = SendCommandPacket(3, true, (BYTE*)(LPCSTR)strFile, strFile.GetLength());
	if (ret < 0) {
		AfxMessageBox("打开文件命令执行失败");
	}
}


void CRemoteClientDlg::OnDownloadfile()
{
	// TODO: 在此添加命令处理程序代码

	_beginthread(ThreadForDownloadFile, 0, this);
	Sleep(50);	//稍微延迟50ms确保线程顺利开启，避免手速过快获取不到正确的鼠标位置。

}


void CRemoteClientDlg::OnDeletefile()
{
	// TODO: 在此添加命令处理程序代码
	int nListSelected = m_list.GetSelectionMark();
	CString strFile = m_list.GetItemText(nListSelected, 0);
	HTREEITEM hTreeSelected = m_tree.GetSelectedItem();
	strFile = GetPath(hTreeSelected) + strFile;
	int ret = SendCommandPacket(9, true, (BYTE*)(LPCSTR)strFile, strFile.GetLength());
	if (ret < 0) {
		AfxMessageBox("删除文件命令执行失败");
	}
	LoadFileCurrent();
}

LRESULT CRemoteClientDlg::OnSendPacket(WPARAM wParam, LPARAM lParam)
{
	CString strFile = (LPCSTR)lParam;
	int ret = SendCommandPacket(wParam>>1, wParam&1, (BYTE*)(LPCSTR)strFile, strFile.GetLength());
	return ret;
}

LRESULT CRemoteClientDlg::OnSendProgress(WPARAM wParam, LPARAM lParam)
{
	m_statusDlg.m_info = (LPCSTR)lParam;
	m_statusDlg.UpdateData(FALSE);
	return 0;
}
