#include "pch.h"
#include "Command.h"

CCommand::CCommand():threadId(0)
{
	struct {
		int nCmd;
		CMDFUNC func;
	}data[] = {			//~¡Ù“‚–¥∑®
		{1,&CCommand::MakeDriverInfo},
		{2,&CCommand::MakeDirectoryInfo},
		{3,&CCommand::RunFile},
		{4,&CCommand::DownloadFile},
		{5,&CCommand::MouseEvent},
		{6,&CCommand::SendScreen},
		{7,&CCommand::LockMachine},
		{8,&CCommand::UnLockMachine},
		{9,&CCommand::DeleteLocalFile},
		{95,&CCommand::TestConnect},
		{-1,NULL},
	};
	for (int i = 0; data[i].nCmd != -1;i++) {
		m_mapFunc.insert(std::pair<int, CMDFUNC>(data[i].nCmd, data[i].func));
	}
}

int CCommand::ExcuteCommand(int nCmd,std::list<CPacket>& pack_list, CPacket& inPacket)
{
	std::map<int, CMDFUNC>::iterator it = m_mapFunc.find(nCmd);
	if (it == m_mapFunc.end()) {
		return -1;
	}
	auto ret = (this->*(it->second))(pack_list, inPacket);
	return ret;
}
