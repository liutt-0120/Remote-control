// LockDialog.cpp: 实现文件
//

#include "pch.h"
#include "RemoteCtrl.h"
#include "LockDialog.h"
#include "afxdialogex.h"


// CLockDialog 对话框

IMPLEMENT_DYNAMIC(CLockDialog, CDialog)

CLockDialog::CLockDialog(CWnd* pParent /*=nullptr*/)
	: CDialog(IDD_DLG_INFO, pParent)
{

}

CLockDialog::~CLockDialog()
{
	TRACE(_T("到这了？"));
}

void CLockDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CLockDialog, CDialog)
	ON_WM_CLOSE()
	ON_WM_KEYDOWN()
END_MESSAGE_MAP()


// CLockDialog 消息处理程序


void CLockDialog::OnClose()
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	MessageBox(_T("我被点了"));
	//CDialog::OnClose();
}


void CLockDialog::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	MessageBox(_T(""));
	CDialog::OnKeyDown(nChar, nRepCnt, nFlags);
}
