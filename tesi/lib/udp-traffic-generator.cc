#include "udp-traffic-generator.h"

#include "ns3/inet-socket-address.h"
#include "ns3/inet6-socket-address.h"
#include "ns3/ipv4-address.h"
#include "ns3/ipv6-address.h"
#include "ns3/log.h"
#include "ns3/nstime.h"
#include "ns3/packet.h"
#include "ns3/simulator.h"
#include "ns3/socket-factory.h"
#include "ns3/socket.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/random-variable-stream.h"
#include "ns3/uinteger.h"
#include "ns3/seq-ts-header.h"

#include <unordered_map>
#include <fstream>
#include <cstdlib>
#include <cstdio>
#include <regex>

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("UdpTrafficGenerator");

TypeId
UdpTrafficGenerator::GetTypeId()
{
    static TypeId tid =
        TypeId("ns3::UdpTrafficGenerator")
            .SetParent<Application>()
            .SetGroupName("Applications")
            .AddConstructor<UdpTrafficGenerator>()
            .AddAttribute(
                "MaxPackets",
                "The maximum number of packets the application will send (zero means infinite)",
                UintegerValue(100),
                MakeUintegerAccessor(&UdpTrafficGenerator::m_count),
                MakeUintegerChecker<uint32_t>())
            .AddAttribute("Interval",
                          "The time to wait between packets",
                          TimeValue(Seconds(1.0)),
                          MakeTimeAccessor(&UdpTrafficGenerator::m_interval),
                          MakeTimeChecker())
            .AddAttribute("RemoteAddress",
                          "The destination Address of the outbound packets",
                          AddressValue(),
                          MakeAddressAccessor(&UdpTrafficGenerator::m_peerAddress),
                          MakeAddressChecker())
            .AddAttribute("RemotePort",
                          "The destination port of the outbound packets",
                          UintegerValue(0),
                          MakeUintegerAccessor(&UdpTrafficGenerator::m_peerPort),
                          MakeUintegerChecker<uint16_t>())
            .AddAttribute("NormalVariance",
                          "The variance of the normal distribution",
                          UintegerValue(30),
                          MakeUintegerAccessor(&UdpTrafficGenerator::normal_variance),
                          MakeUintegerChecker<uint32_t>())
            .AddAttribute("NormalMean",
                          "The mean of the normal distribution",
                          UintegerValue(50),
                          MakeUintegerAccessor(&UdpTrafficGenerator::normal_mean),
                          MakeUintegerChecker<uint32_t>())
            .AddTraceSource("Tx",
                            "A new packet is created and is sent",
                            MakeTraceSourceAccessor(&UdpTrafficGenerator::m_txTrace),
                            "ns3::Packet::TracedCallback")
            .AddTraceSource("Rx",
                            "A packet has been received",
                            MakeTraceSourceAccessor(&UdpTrafficGenerator::m_rxTrace),
                            "ns3::Packet::TracedCallback")
            .AddTraceSource("TxWithAddresses",
                            "A new packet is created and is sent",
                            MakeTraceSourceAccessor(&UdpTrafficGenerator::m_txTraceWithAddresses),
                            "ns3::Packet::TwoAddressTracedCallback")
            .AddTraceSource("RxWithAddresses",
                            "A packet has been received",
                            MakeTraceSourceAccessor(&UdpTrafficGenerator::m_rxTraceWithAddresses),
                            "ns3::Packet::TwoAddressTracedCallback");
    return tid;
}

UdpTrafficGenerator::UdpTrafficGenerator()
{
    NS_LOG_FUNCTION(this);
    m_sent = 0;
    m_socket = nullptr;
    m_sendEvent = EventId();
    m_data = nullptr;
    m_dataSize = 0;
}

UdpTrafficGenerator::~UdpTrafficGenerator()
{
    NS_LOG_FUNCTION(this);
    m_socket = nullptr;

    delete[] m_data;
    m_data = nullptr;
    m_dataSize = 0;
}

void
UdpTrafficGenerator::SetRemote(Address ip, uint16_t port)
{
    NS_LOG_FUNCTION(this << ip << port);
    m_peerAddress = ip;
    m_peerPort = port;
}

void
UdpTrafficGenerator::SetRemote(Address addr)
{
    NS_LOG_FUNCTION(this << addr);
    m_peerAddress = addr;
}

void
UdpTrafficGenerator::DoDispose()
{
    NS_LOG_FUNCTION(this);
    std::string name = "output/stats.csv";
    UdpTrafficGenerator::printArrayToCSV(name);
    Application::DoDispose();
}

void
UdpTrafficGenerator::StartApplication()
{
    NS_LOG_FUNCTION(this);

    random = CreateObject<NormalRandomVariable>();

    if (!m_socket)
    {
        TypeId tid = TypeId::LookupByName("ns3::UdpSocketFactory");
        m_socket = Socket::CreateSocket(GetNode(), tid);
        if (Ipv4Address::IsMatchingType(m_peerAddress) == true)
        {
            if (m_socket->Bind() == -1)
            {
                NS_FATAL_ERROR("Failed to bind socket");
            }
            m_socket->Connect(
                InetSocketAddress(Ipv4Address::ConvertFrom(m_peerAddress), m_peerPort));
        }
        else if (InetSocketAddress::IsMatchingType(m_peerAddress) == true)
        {
            if (m_socket->Bind() == -1)
            {
                NS_FATAL_ERROR("Failed to bind socket");
            }
            m_socket->Connect(m_peerAddress);
        }
        else
        {
            NS_ASSERT_MSG(false, "Incompatible address type: " << m_peerAddress);
        }
    }

    m_socket->SetRecvCallback(MakeCallback(&UdpTrafficGenerator::HandleRead, this));
    m_socket->SetAllowBroadcast(true);
    ScheduleTransmit(Seconds(0.));
}

void
UdpTrafficGenerator::StopApplication()
{
    NS_LOG_FUNCTION(this);
    std::string name = "output/stats.csv";
    UdpTrafficGenerator::printArrayToCSV(name);

    if (m_socket)
    {
        m_socket->Close();
        m_socket->SetRecvCallback(MakeNullCallback<void, Ptr<Socket>>());
        m_socket = nullptr;
    }

    Simulator::Cancel(m_sendEvent);
}

void
UdpTrafficGenerator::SetFill(std::string fill)
{
    NS_LOG_FUNCTION(this << fill);

    uint32_t dataSize = fill.size() + 1;

    if (dataSize != m_dataSize)
    {
        delete[] m_data;
        m_data = new uint8_t[dataSize];
        m_dataSize = dataSize;
    }

    memcpy(m_data, fill.c_str(), dataSize);

    //
    // Overwrite packet size attribute.
    //
    m_size = dataSize;
}

void
UdpTrafficGenerator::ScheduleTransmit(Time dt)
{
    NS_LOG_FUNCTION(this << dt);
    m_sendEvent = Simulator::Schedule(dt, &UdpTrafficGenerator::Send, this);
}

void
UdpTrafficGenerator::Send()
{
    NS_LOG_FUNCTION(this);

    NS_ASSERT(m_sendEvent.IsExpired());

    uint32_t randomNumber = UdpTrafficGenerator::getRandomNumber();
    std::string message = "{ \"sender\": \"client\", \"type\": \"request\", \"id\": " + std::to_string(randomNumber) + " }";
    UdpTrafficGenerator::SetFill(message);
    Ptr<Packet> p = Create<Packet>(m_data, m_dataSize);
    
    Address localAddress;
    m_socket->GetSockName(localAddress);
    // call to the trace sinks before the packet is actually sent,
    // so that tags added to the packet can be sent as well
    m_txTrace(p);
    if (Ipv4Address::IsMatchingType(m_peerAddress))
    {
        m_txTraceWithAddresses(
            p,
            localAddress,
            InetSocketAddress(Ipv4Address::ConvertFrom(m_peerAddress), m_peerPort));
    }

    if(m_socket->Send(p)==-1){
        NS_LOG_INFO("ERRORE INVIO PACCHETTO");
    }
    ++m_sent;

    PacketInfo newP = {
        randomNumber,
        (uint64_t)Simulator::Now().ToInteger(Time::MS),
        0
    };
    packetList[m_sent] = newP;

    if (Ipv4Address::IsMatchingType(m_peerAddress))
    {
        NS_LOG_INFO("At time " << Simulator::Now().As(Time::S) << " client sent " << m_size
                               << " bytes to " << Ipv4Address::ConvertFrom(m_peerAddress)
                               << " port " << m_peerPort);
    }
    else if (InetSocketAddress::IsMatchingType(m_peerAddress))
    {
        NS_LOG_INFO(
            "At time " << Simulator::Now().As(Time::S) << " client sent " << m_size << " bytes to "
                       << InetSocketAddress::ConvertFrom(m_peerAddress).GetIpv4() << " port "
                       << InetSocketAddress::ConvertFrom(m_peerAddress).GetPort());
    }

    if (m_sent < m_count || m_count == 0)
    {
        ScheduleTransmit(m_interval);
    }
}

void
UdpTrafficGenerator::HandleRead(Ptr<Socket> socket)
{
    NS_LOG_FUNCTION(this << socket);
    Ptr<Packet> packet;
    Address from;
    Address localAddress;
    while ((packet = socket->RecvFrom(from)))
    {
        if (InetSocketAddress::IsMatchingType(from))
        {
            NS_LOG_INFO("At time " << Simulator::Now().As(Time::S) << " client received "
                                   << packet->GetSize() << " bytes from "
                                   << InetSocketAddress::ConvertFrom(from).GetIpv4() << " port "
                                   << InetSocketAddress::ConvertFrom(from).GetPort());
        }
        socket->GetSockName(localAddress);
        m_rxTrace(packet);
        m_rxTraceWithAddresses(packet, from, localAddress);

        uint32_t value_from_pkt = getIdPacket(packet);

        for (auto it = packetList.begin(); it != packetList.end(); it++)
        {
            if (it->second.id == value_from_pkt && it->second.receivedAt == 0)
            {
                it->second.receivedAt = (uint64_t)Simulator::Now().ToInteger(Time::MS);
                break;
            }     
        }
    }
}

uint32_t
UdpTrafficGenerator::getIdPacket(Ptr<Packet> packet)
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
UdpTrafficGenerator::getRandomNumber(){
    return random->GetInteger(normal_mean,normal_variance,100);
}

void UdpTrafficGenerator::printArrayToCSV(std::string filename) {
    
    std::ofstream outputFile(filename);
    
    if (!outputFile.is_open()) {
        std::cerr << "Error opening file " << filename << std::endl;
        return;
    }
    
    for (const auto& pair : packetList) {
        const uint32_t& key = pair.first;
        const PacketInfo& value = pair.second;
        outputFile << key << ";" << value.id << ";" << value.requestedAt << ";" << value.receivedAt << std::endl;
    }
    outputFile.close();
}

} // Namespace ns3
