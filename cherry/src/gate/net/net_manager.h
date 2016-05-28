#ifndef _NET_MANAGER_H_
#define _NET_MANAGER_H_

#include <util/m_singleton.h>
#include <map>
#include <net_session.h>
#include <net/m_net_event_loop_thread.h>
#include <net/m_net_listener.h>
#include <string>
#include <mutex>

class NetManager
    :public MSingleton<NetManager>
{
public:
    NetManager();
    ~NetManager();
    NetManager(const NetManager &) = delete;
    NetManager& operator=(const NetManager &) = delete;
public:
    bool Init(size_t thread_count, size_t session_count);
    void Close();
    MNetEventLoopThread* GetMinEventLoopThread();
    bool AddListener(const std::string &ip, unsigned port);
    bool SendSessionMsg(int64_t id, char *p_buf, size_t len);
    bool CloseSession(int64_t id);
public://async
    void OnListenerAcceptCallback(MNetListener *p_listener, MSocket *p_sock);
    void OnListenerErrorCallback(MNetListener *p_listener, MError err);
    void OnListenerEnableCallback(MNetListener *p_listener);
    void OnSessionReadCallback(NetSession *p_session, char *p_buf, size_t len);
    void OnSessionInitErrorCallback(NetSession *p_session, MError err);
    void OnSessionErrorCallback(NetSession *p_session, MError err);
    void OnSessionEnableCallback(NetSession *p_session);
private:
    std::vector<MNetListener*> listeners_;
    std::vector<MNetEventLoopThread*> loop_threads_;
    std::map<int64_t, NetSession*> sessions_;
    std::mutex session_mtx_;
    int64_t session_index_;
};

#endif
