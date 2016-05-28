#ifndef _NET_MANAGER_H_
#define _NET_MANAGER_H_

#include <net/m_socket.h>
#include <net/m_net_event_loop_thread.h>
#include <net/m_net_connector.h>
#include <net/m_net_listener.h>
#include <thread/m_thread.h>
#include <mutex>
#include <functional>
#include <list>
#include <util/m_singleton.h>
#include <set>

struct NetSession
{
    MNetConnector *p_connector;
    MNetEventLoopThread *p_loop_thread;
    bool len_readed;
    uint16_t len;
};

class NetManager
{
public:
    NetManager();
    ~NetManager();
    NetManager(const NetManager &) = delete;
    NetManager& operator=(const NetManager &) = delete;
public:
    bool Init(size_t work_count);
    void Close();

    bool AddListener(const std::string &ip, unsigned short port);
    void CloseSession(NetSession *p_session);
    void WriteSession(NetSession *p_session, char *p_buf, size_t len);
    void WriteAll(const char *p_buf, size_t len);
public:
    void OnConnectCallback(MNetListener *p_listener, MSocket *p_sock);
    void OnListenerErrorCallback(size_t pos, MError err);
    void OnReadCallback(NetSession *p_session);
    void OnCloseCallback(NetSession *p_session, MError err);
private:
    MNetEventLoopThread* GetMinEventsThread();
private:
    std::vector<MNetEventLoopThread*> work_list_;
    std::vector<MNetListener*> listener_list_;
    std::mutex session_mutex_;
    std::set<NetSession*> session_list_;
};

#endif
