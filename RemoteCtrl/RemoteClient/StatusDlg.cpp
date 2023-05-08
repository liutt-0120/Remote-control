// StatusDlg.cpp: 实现文件
//

#include "pch.h"
#include "RemoteClient.h"
#include "StatusDlg.h"
#include "afxdialogex.h"


// CStatusDlg 对话框

IMPLEMENT_DYNAMIC(CStatusDlg, CDialog)

CStatusDlg::CStatusDlg(CWnd* pParent /*=nullptr*/)
	: CDialog(IDD_DOWNLOAD_DIALOG, pParent)
	, m_info(_T(""))
{

}

CStatusDlg::~CStatusDlg()
{
}

void CStatusDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_STATIC_DOWNLOAD, m_info);
	DDX_Control(pDX, IDC_PROGRESS_DOWNLOAD, m_progressBar);
	DDX_Control(pDX, IDC_STATIC_DOWNLOAD, m_info2);
}


BEGIN_MESSAGE_MAP(CStatusDlg, CDialog)
END_MESSAGE_MAP()


// CStatusDlg 消息处理程序
