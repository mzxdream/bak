#ifndef _M_NET_CONNECTOR_H_
#define _M_NET_CONNECTOR_H_

#include <net/m_net_event.h>
#include <util/m_circle_buffer.h>

class MSocket;
class MNetEventLoop;

class MNetConnector
{
public:
    explicit MNetConnector(MSocket *p_sock, MNetEventLoop *p_event_loop
        , const std::function<void ()> &connect_cb, const std::function<void ()> &read_cb, const std::function<void ()> &write_complete_cb, const std::function<void (MError)> &error_cb
        , bool need_free_sock, size_t read_len, size_t write_len);
    ~MNetConnector();
    MNetConnector(const MNetConnector &) = delete;
    MNetConnector& operator=(const MNetConnector &) = delete;
public:
    void SetSocket(MSocket *p_sock);
    MSocket* GetSocket();
    MNetEvent& GetEvent();
    void SetEventLoop(MNetEventLoop *p_event_loop);
    MNetEventLoop* GetEventLoop();
    void SetConnectCallback(const std::function<void ()> &connect_cb);
    std::function<void ()>& GetConnectCallback();
    void SetReadCallback(const std::function<void ()> &read_cb);
    std::function<void ()>& GetReadCallback();
    void SetWriteCompleteCallback(const std::function<void ()> &write_complete_cb);
    std::function<void ()>& GetWriteCompleteCallback();
    void SetErrorCallback(const std::function<void (MError)> &error_cb);
    std::function<void (MError)>& GetErrorCallback();
    void SetNeedFreeSock(bool need);
    bool GetNeedFreeSock() const;

    MError EnableReadWrite(bool enable);

    MError Connect(const std::string &ip, unsigned port);
    MError ReadBuf(void *p_buf, size_t len);
    size_t GetReadBufLen() const;
    MError WriteBuf(const char *p_buf, size_t len);
    size_t GetWriteBufLen() const;
public:
    void OnReadCallback();
    void OnWriteCallback();
    void OnErrorCallback(MError err);
private:
    MSocket *p_sock_;
    MNetEvent event_;
    std::function<void ()> connect_cb_;
    std::function<void ()> read_cb_;
    std::function<void ()> write_complete_cb_;
    std::function<void (MError)> error_cb_;
    bool need_free_sock_;
    MCircleBuffer read_buffer_;
    MCircleBuffer write_buffer_;
    bool write_ready_;
};

#endif


