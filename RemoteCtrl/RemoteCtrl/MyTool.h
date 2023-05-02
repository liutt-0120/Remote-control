#pragma once

/// <summary>
/// ������
/// </summary>
class CMyTool
{
public:
    /// <summary>
/// debug�����������
/// </summary>
/// <param name="pData"></param>
/// <param name="nSize"></param>
    static void Dump(BYTE* pData, size_t nSize) {
        std::string strOut;
        char buf[8] = "";
        for (size_t i = 0; i < nSize; i++) {
            if (i > 0 && (i % 16 == 0)) strOut += "\n";
            snprintf(buf, sizeof(buf), "%02X ", pData[i] & 0xFF);
            strOut += buf;
        }
        strOut += "\n";
        OutputDebugStringA(strOut.c_str());
    }

};

