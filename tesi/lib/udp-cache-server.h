#ifndef UDP_CACHE_SERVER_H
#define UDP_CACHE_SERVER_H

#include "ns3/address.h"
#include "ns3/application.h"
#include "ns3/event-id.h"
#include "ns3/ptr.h"
#include "ns3/traced-callback.h"
#include "ns3/uinteger.h"
#include "ns3/inet-socket-address.h"
#include <deque>
#include <map>

namespace ns3
{

class Socket;
class Packet;

class UdpCacheServer : public Application
{
  public:
    static TypeId GetTypeId();
    UdpCacheServer();
    ~UdpCacheServer() override;

  protected:
    void DoDispose() override;

  private:
    void StartApplication() override;
    void StopApplication() override;

    void HandleReadClients(Ptr<Socket> socket);

    void HandleReadServer(Ptr<Socket> socket);

    uint32_t getIdPacket(Ptr<Packet> packet);

    uint32_t getRandomNumber();

    void sendPacketBackToClient(uint32_t value_to_send, Address to);
    
    void requestPacketToContentServer(uint32_t value_to_request);

    void pushInCache(const uint32_t& item);

    bool cacheContains(const uint32_t& item);

    bool pushIfNotContained(const uint32_t& item);

    void prefetchData(uint32_t value);

    void printOut();

    uint16_t m_port_clients;  //!< Port on which we listen for incoming request from clients.
    uint16_t m_port_server;   //!< Port on which we listen for incoming packets from content server.
    Ptr<Socket> m_socket_clients;  //!< IPv4 Socket
    Ptr<Socket> m_socket_server;   //!< IPv4 Socket
    Address m_local;          //!< local multicast address
    uint32_t hitcount;
    uint32_t accesscount;

    /// Callbacks for tracing the packet Rx events
    TracedCallback<Ptr<const Packet>> m_rxTrace;

    /// Callbacks for tracing the packet Rx events, includes source and destination addresses
    TracedCallback<Ptr<const Packet>, const Address&, const Address&> m_rxTraceWithAddresses;

    uint32_t m_cacheSize;
    Time m_RTTCacheMiss;
    std::deque<uint32_t> m_cache;
    std::multimap<uint32_t, Address> requestQueue;
    Address contentServerAddress;
};

} // namespace ns3

#endif /* UDP_CACHE_SERVER_H */
