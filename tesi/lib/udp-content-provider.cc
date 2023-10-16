#include "udp-content-provider.h"

#include "ns3/address-utils.h"
#include "ns3/inet-socket-address.h"
#include "ns3/inet6-socket-address.h"
#include "ns3/random-variable-stream.h"
#include "ns3/ipv4-address.h"
#include "ns3/ipv6-address.h"
#include "ns3/log.h"
#include "ns3/nstime.h"
#include "ns3/packet.h"
#include "ns3/simulator.h"
#include "ns3/socket-factory.h"
#include "ns3/socket.h"
#include "ns3/udp-socket.h"
#include "ns3/uinteger.h"

#include <regex>

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("UdpContentProviderApplication");

NS_OBJECT_ENSURE_REGISTERED(UdpContentProvider);

TypeId
UdpContentProvider::GetTypeId()
{
    static TypeId tid =
        TypeId("ns3::UdpContentProvider")
            .SetParent<Application>()
            .SetGroupName("Applications")
            .AddConstructor<UdpContentProvider>()
            .AddAttribute("Port",
                          "Port on which we listen for incoming packets.",
                          UintegerValue(15),
                          MakeUintegerAccessor(&UdpContentProvider::m_port),
                          MakeUintegerChecker<uint16_t>())
            .AddTraceSource("Rx",
                            "A packet has been received",
                            MakeTraceSourceAccessor(&UdpContentProvider::m_rxTrace),
                            "ns3::Packet::TracedCallback")
            .AddTraceSource("RxWithAddresses",
                            "A packet has been received",
                            MakeTraceSourceAccessor(&UdpContentProvider::m_rxTraceWithAddresses),
                            "ns3::Packet::TwoAddressTracedCallback");
    return tid;
}

UdpContentProvider::UdpContentProvider()
{
    NS_LOG_FUNCTION(this);
}

UdpContentProvider::~UdpContentProvider()
{
    NS_LOG_FUNCTION(this);
    m_socket = nullptr;
}

void
UdpContentProvider::DoDispose()
{
    NS_LOG_FUNCTION(this);
    Application::DoDispose();
}

void
UdpContentProvider::StartApplication()
{
    NS_LOG_FUNCTION(this);

    if (!m_socket)
    {
        TypeId tid = TypeId::LookupByName("ns3::UdpSocketFactory");
        m_socket = Socket::CreateSocket(GetNode(), tid);
        InetSocketAddress local = InetSocketAddress(Ipv4Address::GetAny(), m_port);
        if (m_socket->Bind(local) == -1)
        {
            NS_FATAL_ERROR("Failed to bind socket");
        }
    }

    m_socket->SetRecvCallback(MakeCallback(&UdpContentProvider::HandleRead, this));
}

void
UdpContentProvider::StopApplication()
{
    NS_LOG_FUNCTION(this);

    if (m_socket)
    {
        m_socket->Close();
        m_socket->SetRecvCallback(MakeNullCallback<void, Ptr<Socket>>());
    }
}

void
UdpContentProvider::HandleRead(Ptr<Socket> socket)
{
    NS_LOG_FUNCTION(this << socket);

    Ptr<Packet> packet;
    Address from;
    Address localAddress;
    while ((packet = socket->RecvFrom(from)))
    {
        socket->GetSockName(localAddress);
        m_rxTrace(packet);
        m_rxTraceWithAddresses(packet, from, localAddress);
        if (InetSocketAddress::IsMatchingType(from))
        {
            NS_LOG_INFO("At time " << Simulator::Now().As(Time::S) << " content server received "
                                   << packet->GetSize() << " bytes from "
                                   << InetSocketAddress::ConvertFrom(from).GetIpv4() << " port "
                                   << InetSocketAddress::ConvertFrom(from).GetPort());
        }

        packet->RemoveAllPacketTags();
        packet->RemoveAllByteTags();

        uint32_t value_from_pkt = getIdPacket(packet);

        NS_LOG_LOGIC("Serve the request of packet with id: " << value_from_pkt);
        
        sendPacketBackToCache(value_from_pkt, from);

        if (InetSocketAddress::IsMatchingType(from))
        {
            NS_LOG_INFO("At time " << Simulator::Now().As(Time::S) << " content server sent "
                                   << packet->GetSize() << " bytes to "
                                   << InetSocketAddress::ConvertFrom(from).GetIpv4() << " port "
                                   << InetSocketAddress::ConvertFrom(from).GetPort());
        }
    }
}

uint32_t
UdpContentProvider::getIdPacket(Ptr<Packet> packet)
{
    uint8_t* buffer = new uint8_t[packet->GetSize()];
    packet->CopyData(buffer, packet->GetSize());
    std::string payload = std::string(reinterpret_cast<char*>(buffer), packet->GetSize());
    std::regex six_digit_number_regex(R"(\b\d{1,6}\b)");

    std::smatch matches;
    if (std::regex_search(payload, matches, six_digit_number_regex))
    {
        std::string id = matches[0];
        return static_cast<uint32_t>(std::stoul(id));
    }
    else
    {
        // handle whenever the id is not found
        return 0;
    }
}

uint32_t
UdpContentProvider::getRandomNumber(){
    Ptr<UniformRandomVariable> random = CreateObject<UniformRandomVariable>();
    return random->GetInteger(1, 100);
}

void UdpContentProvider::sendPacketBackToCache(uint32_t value_to_send, Address to){

    std::string message = "{ \"sender\": \"server\", \"type\": \"response\", \"id\": " + std::to_string(value_to_send) + " }";
    uint32_t dataSize = message.size() + 1;
    uint8_t* m_data = new uint8_t[dataSize];

    memcpy(m_data, message.c_str(), dataSize);

    Ptr<Packet> packet = Create<Packet>(m_data, dataSize);
    m_socket->SendTo(packet, 0, to);
}

} // Namespace ns3

