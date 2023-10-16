#ifndef UDP_CONTENT_PROVIDER_H
#define UDP_CONTENT_PROVIDER_H

#include "ns3/address.h"
#include "ns3/application.h"
#include "ns3/event-id.h"
#include "ns3/ptr.h"
#include "ns3/traced-callback.h"
#include "ns3/uinteger.h"

namespace ns3
{

class Socket;
class Packet;

class UdpContentProvider : public Application
{
  public:
    static TypeId GetTypeId();
    UdpContentProvider();
    ~UdpContentProvider() override;

  protected:
    void DoDispose() override;

  private:
    void StartApplication() override;
    void StopApplication() override;

    void HandleRead(Ptr<Socket> socket);

    uint32_t getIdPacket(Ptr<Packet> packet);

    uint32_t getRandomNumber();

    void sendPacketBackToCache(uint32_t value_to_send, Address to);

    uint16_t m_port;       //!< Port on which we listen for incoming packets.
    Ptr<Socket> m_socket;  //!< IPv4 Socket
    Address m_local;       //!< local multicast address

    /// Callbacks for tracing the packet Rx events
    TracedCallback<Ptr<const Packet>> m_rxTrace;

    /// Callbacks for tracing the packet Rx events, includes source and destination addresses
    TracedCallback<Ptr<const Packet>, const Address&, const Address&> m_rxTraceWithAddresses;
};

} // namespace ns3

#endif /* UDP_CONTENT_PROVIDER_H */
