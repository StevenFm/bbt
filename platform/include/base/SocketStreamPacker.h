#pragma once

template <size_t MAX_SIZE>
class XSocketStreamPacker
{
public:
    XSocketStreamPacker()
    {
        m_piSocketMgr = NULL;
        m_pSocket     = NULL;
        m_uDataLen    = 0;
    }

    void Reset(ISocketMgr* piSocketMgr, XSocket* pSocket)
    {
        m_piSocketMgr = piSocketMgr;
        m_pSocket     = pSocket;
        m_uDataLen    = 0;
    }

    void PackUp(void* pvData, size_t uDataLen)
    {
        if (m_uDataLen + uDataLen > MAX_SIZE)
            FlushSend();

        if (uDataLen > MAX_SIZE)
        {
            assert(m_uDataLen == 0);
            m_piSocketMgr->Send(m_pSocket, pvData, uDataLen);
            return;
        }

        memcpy(m_byBuffer + m_uDataLen, pvData, uDataLen);
        m_uDataLen += uDataLen;
    }

    void FlushSend()
    {
        if (m_uDataLen > 0)
        {
            m_piSocketMgr->Send(m_pSocket, m_byBuffer, m_uDataLen);
            m_uDataLen = 0;
        }
    }

private:
    ISocketMgr*     m_piSocketMgr;
    XSocket*        m_pSocket;
    size_t          m_uDataLen;
    BYTE            m_byBuffer[MAX_SIZE];
};
