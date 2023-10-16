#ifndef UDP_TRAFFIC_GENERATOR_H
#define UDP_TRAFFIC_GENERATOR_H

#include "ns3/application.h"
#include "ns3/event-id.h"
#include "ns3/ipv4-address.h"
#include "ns3/ptr.h"
#include "ns3/traced-callback.h"
#include "ns3/random-variable-stream.h"
#include <unordered_map>

namespace ns3
{

class Socket;
class Packet;

class UdpTrafficGenerator : public Application
{
  public:
    /**
     * \brief Get the type ID.
     * \return the object TypeId
     */
    static TypeId GetTypeId();

    UdpTrafficGenerator();

    ~UdpTrafficGenerator() override;

    /**
     * \brief set the remote address and port
     * \param ip remote IP address
     * \param port remote port
     */
    void SetRemote(Address ip, uint16_t port);
    /**
     * \brief set the remote address
     * \param addr remote address
     */
    void SetRemote(Address addr);

    /**
     * Set the data fill of the packet (what is sent as data to the server) to
     * the zero-terminated contents of the fill string string.
     *
     * \warning The size of resulting echo packets will be automatically adjusted
     * to reflect the size of the fill string -- this means that the PacketSize
     * attribute may be changed as a result of this call.
     *
     * \param fill The string to use as the actual echo data bytes.
     */
    void SetFill(std::string fill);

  protected:
    void DoDispose() override;

  private:
    void StartApplication() override;
    void StopApplication() override;

    /**
     * \brief Schedule the next packet transmission
     * \param dt time interval between packets.
     */
    void ScheduleTransmit(Time dt);
    /**
     * \brief Send a packet
     */
    void Send();

    /**
     * \brief Handle a packet reception.
     *
     * This function is called by lower layers.
     *
     * \param socket the socket the packet was received to.
     */
    void HandleRead(Ptr<Socket> socket);

    uint32_t getIdPacket(Ptr<Packet> packet);

    uint32_t getRandomNumber();

    void printArrayToCSV(std::string filename);

    Ptr<Packet> createRandomPacketRequest(uint32_t m_sent, uint32_t m_size, uint32_t randomNumber);

    uint32_t m_count; //!< Maximum number of packets the application will send
    Time m_interval;  //!< Packet inter-send time
    uint32_t m_size;  //!< Size of the sent packet

    uint32_t m_dataSize; //!< packet payload size (must be equal to m_size)
    uint8_t* m_data;     //!< packet payload data

    uint32_t m_sent;       //!< Counter for sent packets
    Ptr<Socket> m_socket;  //!< Socket
    Address m_peerAddress; //!< Remote peer address
    uint16_t m_peerPort;   //!< Remote peer port
    EventId m_sendEvent;   //!< Event to send the next packet

    uint32_t normal_mean;
    uint32_t normal_variance;

    struct PacketInfo {
      uint32_t id;
      uint64_t requestedAt;
      uint64_t receivedAt;
    };

    Ptr<NormalRandomVariable> random;

    std::unordered_map<uint32_t, PacketInfo> packetList;

    /// Callbacks for tracing the packet Tx events
    TracedCallback<Ptr<const Packet>> m_txTrace;

    /// Callbacks for tracing the packet Rx events
    TracedCallback<Ptr<const Packet>> m_rxTrace;

    /// Callbacks for tracing the packet Tx events, includes source and destination addresses
    TracedCallback<Ptr<const Packet>, const Address&, const Address&> m_txTraceWithAddresses;

    /// Callbacks for tracing the packet Rx events, includes source and destination addresses
    TracedCallback<Ptr<const Packet>, const Address&, const Address&> m_rxTraceWithAddresses;
};

} // namespace ns3

#endif /* UDP_TRAFFIC_GENERATOR_H */
