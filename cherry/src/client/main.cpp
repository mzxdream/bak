#include <net/m_net_event_loop_thread.h>
#include <net/m_net_connector.h>
#include <net/m_net_listener.h>
#include <util/m_logger.h>
#include <arpa/inet.h>
#include <net/m_socket.h>

uint16_t len = 0;
bool len_readed = false;

void OnConnectCallback(MNetConnector *p_connector)
{
    MLOG(MGetLibLogger(), MERR, "connect success");
    p_connector->EnableReadWrite(true);
}

void OnErrorCallback(MError err)
{
    MLOG(MGetLibLogger(), MERR, "err:", static_cast<int>(err));
}

void OnReadCallback(MNetConnector *p_connector)
{
    while (true)
    {
        if (!len_readed)
        {
            if (p_connector->ReadBuf(&len, sizeof(len)) == MError::No)
            {
                len = ntohs(len);
                len_readed = true;
            }
            else
            {
                return;
            }
        }
        std::string str;
        str.resize(len);
        if (p_connector->ReadBuf(&str[0], str.size()) == MError::No)
        {
            MLOG(MGetLibLogger(), MWARN, str);
            len_readed = false;
        }
        else
        {
            return;
        }
    }
}

void OnWriteCompleteCallback(MNetConnector *p_connector)
{
    MLOG(MGetLibLogger(), MERR, "write complete");
}

void WriteTo(MNetConnector *p_connector, const std::string &str)
{
    uint16_t size = htons(static_cast<uint16_t>(str.size()));
    p_connector->WriteBuf(static_cast<const char*>(static_cast<void *>(&size)), sizeof(size));
    p_connector->WriteBuf(str.c_str(), str.size());
}

int main (int argc, char *argv[])
{
    MNetEventLoopThread ev_thread;
    MError err = ev_thread.Init();
    if (err != MError::No)
    {
        MLOG(MGetLibLogger(), MERR, "event loop thread init failed");
        return 0;
    }
    err = ev_thread.Start();
    if (err != MError::No)
    {
        MLOG(MGetLibLogger(), MERR, "event loop thread start failed");
        return 0;
    }
    MSocket sock;
    err = sock.Create(MSocketFamily::IPV4, MSocketType::TCP, MSocketProtocol::Default);
    if (err != MError::No)
    {
        MLOG(MGetLibLogger(), MERR, "create socket failed");
        return 0;
    }
    MNetConnector conn(&sock, nullptr, &ev_thread.GetEventLoop(), nullptr, nullptr, nullptr, nullptr, false, 10, 10);
    conn.SetConnectCallback(std::bind(OnConnectCallback, &conn));
    conn.SetReadCallback(std::bind(OnReadCallback, &conn));
    conn.SetWriteCompleteCallback(std::bind(OnWriteCompleteCallback, &conn));
    conn.SetErrorCallback(std::bind(OnErrorCallback, std::placeholders::_1));

    err = conn.Connect("127.0.0.1", 3232);
    if (err != MError::No)
    {
        MLOG(MGetLibLogger(), MERR, "connect failed");
        return 0;
    }

    char tmp[100];
    while (std::cin.getline(tmp, 100))
    {
        std::string str(tmp);
        if (str == "exit")
        {
            ev_thread.Stop();
            break;
        }
        ev_thread.AddCallback(std::bind(WriteTo, &conn, str));
        ev_thread.Interrupt();
    }
    ev_thread.Join();
    MLOG(MGetLibLogger(), MERR, "exit...");
    return 0;
}

