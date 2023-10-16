#include "udp-traffic-cache-cp-helper.h"
#include "udp-cache-server.h"
#include "udp-traffic-generator.h"
#include "udp-content-provider.h"

#include "ns3/names.h"
#include "ns3/uinteger.h"

namespace ns3
{

UdpCacheServerHelper::UdpCacheServerHelper(Address contentSever, uint16_t portContentServer, uint16_t portClients)
{
    m_factory.SetTypeId(UdpCacheServer::GetTypeId());
    SetAttribute("PortClients", UintegerValue(portClients));
    SetAttribute("PortContentServer", UintegerValue(portContentServer));
    SetAttribute("IpContentServer", AddressValue(contentSever));
}

void
UdpCacheServerHelper::SetAttribute(std::string name, const AttributeValue& value)
{
    m_factory.Set(name, value);
}

ApplicationContainer
UdpCacheServerHelper::Install(Ptr<Node> node) const
{
    return ApplicationContainer(InstallPriv(node));
}

ApplicationContainer
UdpCacheServerHelper::Install(std::string nodeName) const
{
    Ptr<Node> node = Names::Find<Node>(nodeName);
    return ApplicationContainer(InstallPriv(node));
}

ApplicationContainer
UdpCacheServerHelper::Install(NodeContainer c) const
{
    ApplicationContainer apps;
    for (NodeContainer::Iterator i = c.Begin(); i != c.End(); ++i)
    {
        apps.Add(InstallPriv(*i));
    }

    return apps;
}

Ptr<Application>
UdpCacheServerHelper::InstallPriv(Ptr<Node> node) const
{
    Ptr<Application> app = m_factory.Create<UdpCacheServer>();
    node->AddApplication(app);

    return app;
}

UdpTrafficGeneratorHelper::UdpTrafficGeneratorHelper(Address address, uint16_t port)
{
    m_factory.SetTypeId(UdpTrafficGenerator::GetTypeId());
    SetAttribute("RemoteAddress", AddressValue(address));
    SetAttribute("RemotePort", UintegerValue(port));
}

UdpTrafficGeneratorHelper::UdpTrafficGeneratorHelper(Address address)
{
    m_factory.SetTypeId(UdpTrafficGenerator::GetTypeId());
    SetAttribute("RemoteAddress", AddressValue(address));
}

void
UdpTrafficGeneratorHelper::SetAttribute(std::string name, const AttributeValue& value)
{
    m_factory.Set(name, value);
}

void
UdpTrafficGeneratorHelper::SetFill(Ptr<Application> app, std::string fill)
{
    app->GetObject<UdpTrafficGenerator>()->SetFill(fill);
}

ApplicationContainer
UdpTrafficGeneratorHelper::Install(Ptr<Node> node) const
{
    return ApplicationContainer(InstallPriv(node));
}

ApplicationContainer
UdpTrafficGeneratorHelper::Install(std::string nodeName) const
{
    Ptr<Node> node = Names::Find<Node>(nodeName);
    return ApplicationContainer(InstallPriv(node));
}

ApplicationContainer
UdpTrafficGeneratorHelper::Install(NodeContainer c) const
{
    ApplicationContainer apps;
    for (NodeContainer::Iterator i = c.Begin(); i != c.End(); ++i)
    {
        apps.Add(InstallPriv(*i));
    }

    return apps;
}

Ptr<Application>
UdpTrafficGeneratorHelper::InstallPriv(Ptr<Node> node) const
{
    Ptr<Application> app = m_factory.Create<UdpTrafficGenerator>();
    node->AddApplication(app);

    return app;
}

UdpContentProviderHelper::UdpContentProviderHelper(uint16_t port)
{
    m_factory.SetTypeId(UdpContentProvider::GetTypeId());
    SetAttribute("Port", UintegerValue(port));
}

void
UdpContentProviderHelper::SetAttribute(std::string name, const AttributeValue& value)
{
    m_factory.Set(name, value);
}

ApplicationContainer
UdpContentProviderHelper::Install(Ptr<Node> node) const
{
    return ApplicationContainer(InstallPriv(node));
}

ApplicationContainer
UdpContentProviderHelper::Install(std::string nodeName) const
{
    Ptr<Node> node = Names::Find<Node>(nodeName);
    return ApplicationContainer(InstallPriv(node));
}

ApplicationContainer
UdpContentProviderHelper::Install(NodeContainer c) const
{
    ApplicationContainer apps;
    for (NodeContainer::Iterator i = c.Begin(); i != c.End(); ++i)
    {
        apps.Add(InstallPriv(*i));
    }

    return apps;
}

Ptr<Application>
UdpContentProviderHelper::InstallPriv(Ptr<Node> node) const
{
    Ptr<Application> app = m_factory.Create<UdpContentProvider>();
    node->AddApplication(app);

    return app;
}


} // namespace ns3
