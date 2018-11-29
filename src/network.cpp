/*
 *  Copyright (c) 2018 str2num. All Rights Reserved.
 *  Copyright (c) 2011, The WebRTC project authors. All rights reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree.
 */
  
/**
 * @file network.cpp
 * @author str2num
 * @brief 
 *  
 **/

#include <stdio.h>
#include <algorithm>
#include <memory>

#include <net/if.h>

#include "ifaddrs_converter.h"
#include "logging.h"
#include "network.h"

namespace rtcbase {

namespace {

// Turning on IPv6 could make many IPv6 interfaces available for connectivity
// check and delay the call setup time. kMaxIPv6Networks is the default upper
// limit of IPv6 networks but could be changed by set_max_ipv6_networks().
const int k_max_IPv6_networks = 5;

const int k_highest_network_preference = 127;

typedef struct {
    Network* net;
    std::vector<InterfaceAddress> ips;
} AddressList;

bool compare_networks(const Network* a, const Network* b) {
    if (a->prefix_length() == b->prefix_length()) {
        if (a->name() == b->name()) {
            return a->prefix() < b->prefix();
        }
    }
    return a->name() < b->name();
}

bool sort_networks(const Network* a, const Network* b) {
    // Network types will be preferred above everything else while sorting
    // Networks.

    // Networks are sorted first by type.
    if (a->type() != b->type()) {
        return a->type() < b->type();
    }

    IPAddress ip_a = a->get_best_IP();
    IPAddress ip_b = b->get_best_IP();

    // After type, networks are sorted by IP address precedence values
    // from RFC 3484-bis
    if (IP_address_precedence(ip_a) != IP_address_precedence(ip_b)) {
        return IP_address_precedence(ip_a) > IP_address_precedence(ip_b);
    }

    // TODO(mallinath) - Add VPN and Link speed conditions while sorting.

    // Networks are sorted last by key.
    return a->key() > b->key();
}

std::string adapter_type_to_string(AdapterType type) {
    switch (type) {
        case ADAPTER_TYPE_UNKNOWN:
            return "Unknown";
        case ADAPTER_TYPE_ETHERNET:
            return "Ethernet";
        case ADAPTER_TYPE_WIFI:
            return "Wifi";
        case ADAPTER_TYPE_CELLULAR:
            return "Cellular";
        case ADAPTER_TYPE_VPN:
            return "VPN";
        case ADAPTER_TYPE_LOOPBACK:
            return "Loopback";
        default:
            return std::string();
    }
}

bool is_ignored_IPv6(const InterfaceAddress& ip) {
    if (ip.family() != AF_INET6) {
        return false;
    }

    // Link-local addresses require scope id to be bound successfully.
    // However, our IPAddress structure doesn't carry that so the
    // information is lost and causes binding failure.
    if (IP_is_link_local(ip)) {
        return true;
    }

    // Any MAC based IPv6 should be avoided to prevent the MAC tracking.
    if (IP_is_mac_based(ip)) {
        return true;
    }

    // Ignore deprecated IPv6.
    if (ip.ipv6_flags() & IPV6_ADDRESS_FLAG_DEPRECATED) {
        return true;
    }

    return false;
}

} // namespace

std::string make_network_key(const std::string& name, const IPAddress& prefix,
        int prefix_length) 
{
    std::ostringstream ost;
    ost << name << "%" << prefix.to_string() << "/" << prefix_length;
    return ost.str();
}

// Test if the network name matches the type<number> pattern, e.g. eth0. The
// matching is case-sensitive.
bool match_type_name_with_index_pattern(const std::string& network_name,
		const std::string& type_name) {
	if (network_name.find(type_name) != 0) {
		return false;
	}
	return std::find_if(network_name.begin() + type_name.size(),
			network_name.end(),
			[](char c) { return !isdigit(c); }) == network_name.end();
}

// A cautious note that this method may not provide an accurate adapter type
// based on the string matching. Incorrect type of adapters can affect the
// result of the downstream network filtering, see e.g.
// BasicPortAllocatorSession::GetNetworks when
// PORTALLOCATOR_DISABLE_COSTLY_NETWORKS is turned on.
AdapterType get_adapter_type_from_name(const char* network_name) {
	if (match_type_name_with_index_pattern(network_name, "lo")) {
		// Note that we have a more robust way to determine if a network interface
		// is a loopback interface by checking the flag IFF_LOOPBACK in ifa_flags of
		// an ifaddr struct. See ConvertIfAddrs in this file.
		return ADAPTER_TYPE_LOOPBACK;
	}
	if (match_type_name_with_index_pattern(network_name, "eth")) {
		return ADAPTER_TYPE_ETHERNET;
	}

	if (match_type_name_with_index_pattern(network_name, "ipsec") ||
			match_type_name_with_index_pattern(network_name, "tun") ||
			match_type_name_with_index_pattern(network_name, "utun") ||
			match_type_name_with_index_pattern(network_name, "tap")) {
		return ADAPTER_TYPE_VPN;
	}

	return ADAPTER_TYPE_UNKNOWN;
}

NetworkManager::NetworkManager() {
}

NetworkManager::~NetworkManager() {
}

NetworkManager::EnumerationPermission NetworkManager::enumeration_permission() const {
    return ENUMERATION_ALLOWED;
}

bool NetworkManager::get_default_local_address(int family, IPAddress* addr) const {
    (void)family;
    (void)addr;
    return false;
}

NetworkManagerBase::NetworkManagerBase()
    : //enumeration_permission_(NetworkManager::ENUMERATION_ALLOWED),
    _max_ipv6_networks(k_max_IPv6_networks),
    _ipv6_enabled(true)
{
}

NetworkManagerBase::~NetworkManagerBase() {
    for (const auto& kv : _networks_map) {
        delete kv.second;
    }
}

void NetworkManagerBase::get_networks(NetworkList* result) const {
    int ipv6_networks = 0;
    result->clear();
    for (Network* network : _networks) {
        // Keep the number of IPv6 networks under |_max_ipv6_networks|.
        if (network->prefix().family() == AF_INET6) {
            if (ipv6_networks >= _max_ipv6_networks) {
                continue;
            }
            ++ipv6_networks;
        }
        result->push_back(network);
    }
}

void NetworkManagerBase::merge_network_list(const NetworkList& new_networks,
        bool* changed) 
{
    NetworkManager::Stats stats;
    merge_network_list(new_networks, changed, &stats);
}

void NetworkManagerBase::merge_network_list(const NetworkList& new_networks,
        bool* changed,
        NetworkManager::Stats* stats) 
{
    *changed = false;
    // AddressList in this map will track IP addresses for all Networks
    // with the same key.
    std::map<std::string, AddressList> consolidated_address_list;
    NetworkList list(new_networks);
    std::sort(list.begin(), list.end(), compare_networks);
    // First, build a set of network-keys to the ipaddresses.
    for (Network* network : list) {
        bool might_add_to_merged_list = false;
        std::string key = make_network_key(network->name(),
                network->prefix(),
                network->prefix_length());
        if (consolidated_address_list.find(key) ==
                consolidated_address_list.end()) 
        {
            AddressList addrlist;
            addrlist.net = network;
            consolidated_address_list[key] = addrlist;
            might_add_to_merged_list = true;
        }
        const std::vector<InterfaceAddress>& addresses = network->get_IPs();
        AddressList& current_list = consolidated_address_list[key];
        for (const InterfaceAddress& address : addresses) {
            current_list.ips.push_back(address);
        }
        if (!might_add_to_merged_list) {
            delete network;
        } else {
            if (current_list.ips[0].family() == AF_INET) {
                stats->ipv4_network_count++;
            } else if (current_list.ips[0].family() == AF_INET6) {
                stats->ipv6_network_count++;
            }
        }
    }
    
    // Next, look for existing network objects to re-use.
    // Result of Network merge. Element in this list should have unique key.
    NetworkList merged_list;
    for (const auto& kv : consolidated_address_list) {
        const std::string& key = kv.first;
        Network* net = kv.second.net;
        auto existing = _networks_map.find(key);
        if (existing == _networks_map.end()) {
            // This network is new. Place it in the network map.
            merged_list.push_back(net);
            _networks_map[key] = net;
            net->set_id(_next_available_network_id++);
            // Also, we might have accumulated IPAddresses from the first
            // step, set it here.
            net->set_IPs(kv.second.ips, true);
            *changed = true;
        } else {
            // This network exists in the map already. Reset its IP addresses.
            Network* existing_net = existing->second;
            *changed = existing_net->set_IPs(kv.second.ips, *changed);
            merged_list.push_back(existing_net);
            if (net->type() != ADAPTER_TYPE_UNKNOWN &&
                    net->type() != existing_net->type()) 
            {
                existing_net->set_type(net->type());
                *changed = true;
            }
            // If the existing network was not active, networks have changed.
            if (!existing_net->active()) {
                *changed = true;
            }
            
            if (existing_net != net) {
                delete net;
            }
        }
    }
    // It may still happen that the merged list is a subset of |networks_|.
    // To detect this change, we compare their sizes.
    if (merged_list.size() != _networks.size()) {
        *changed = true;
    }

    // If the network list changes, we re-assign |networks_| to the merged list
    // and re-sort it.
    if (*changed) {
        _networks = merged_list;
        // Reset the active states of all networks.
        for (const auto& kv : _networks_map) {
            Network* network = kv.second;
            // If |network| is in the newly generated |networks_|, it is active.
            bool found = std::find(_networks.begin(), _networks.end(), network) !=
                _networks.end();
            network->set_active(found);
        }
        std::sort(_networks.begin(), _networks.end(), sort_networks);
        // Now network interfaces are sorted, we should set the preference value
        // for each of the interfaces we are planning to use.
        // Preference order of network interfaces might have changed from previous
        // sorting due to addition of higher preference network interface.
        // Since we have already sorted the network interfaces based on our
        // requirements, we will just assign a preference value starting with 127,
        // in decreasing order.
        int pref = k_highest_network_preference;
        for (Network* network : _networks) {
            network->set_preference(pref);
            if (pref > 0) {
                --pref;
            } else {
                LOG(LS_WARNING) << "Too many network interfaces to handle!";
                break;
            }
        }
    }
}

BasicNetworkManager::BasicNetworkManager()
    : _sent_first_update(false),
      _ignore_non_default_routes(false)
{
}

BasicNetworkManager::~BasicNetworkManager() {
}

void BasicNetworkManager::convert_if_addrs(struct ifaddrs* interfaces,
        IfAddrsConverter* ifaddrs_converter,
        bool include_ignored,
        NetworkList* networks) const 
{
    NetworkMap current_networks;

    for (struct ifaddrs* cursor = interfaces;
            cursor != NULL; cursor = cursor->ifa_next) 
    {
        IPAddress prefix;
        IPAddress mask;
        InterfaceAddress ip;
        int scope_id = 0;

        // Some interfaces may not have address assigned.
        if (!cursor->ifa_addr || !cursor->ifa_netmask) {
            continue;
        }
        
        // Skip ones which are down.
        if (!(cursor->ifa_flags & IFF_RUNNING)) {
            continue;
        }
        
        // Skip unknown family.
        if (cursor->ifa_addr->sa_family != AF_INET &&
                cursor->ifa_addr->sa_family != AF_INET6) {
            continue;
        }
        
        // Skip IPv6 if not enabled.
        if (cursor->ifa_addr->sa_family == AF_INET6 && !ipv6_enabled()) {
            continue;
        }
        
        // Convert to InterfaceAddress.
        if (!ifaddrs_converter->convert_if_addrs_to_IP_address(cursor, &ip, &mask)) {
            continue;
        }

        // Special case for IPv6 address.
        if (cursor->ifa_addr->sa_family == AF_INET6) {
            if (is_ignored_IPv6(ip)) {
                continue;
            }
            scope_id =
                reinterpret_cast<sockaddr_in6*>(cursor->ifa_addr)->sin6_scope_id;
        }
        
        AdapterType adapter_type = ADAPTER_TYPE_UNKNOWN;
        if (cursor->ifa_flags & IFF_LOOPBACK) {
            adapter_type = ADAPTER_TYPE_LOOPBACK;
        } else {
            if (adapter_type == ADAPTER_TYPE_UNKNOWN) {
                adapter_type = get_adapter_type_from_name(cursor->ifa_name);
            }
        }
        int prefix_length = count_IP_mask_bits(mask);
        prefix = truncate_IP(ip, prefix_length);
        std::string key = make_network_key(std::string(cursor->ifa_name),
                prefix, prefix_length);
        auto iter = current_networks.find(key);
        if (iter == current_networks.end()) 
        {
            // TODO(phoglund): Need to recognize other types as well.
            std::unique_ptr<Network> network(
                    new Network(cursor->ifa_name, cursor->ifa_name, prefix, prefix_length,
                        adapter_type));
            network->set_default_local_address_provider(this);
            network->set_scope_id(scope_id);
            network->add_IP(ip);
            network->set_ignored(is_ignored_network(*network));
            if (include_ignored || !network->ignored()) {
                current_networks[key] = network.get();
                networks->push_back(network.release());
            }
        } else {
            Network* existing_network = iter->second;
            existing_network->add_IP(ip);
            if (adapter_type != ADAPTER_TYPE_UNKNOWN) {
                existing_network->set_type(adapter_type);
            }
        }
    }
}

void BasicNetworkManager::start_updating() {
    if (!_sent_first_update) {
        update_networks_once();
    } else {
        signal_networks_changed();
    }
}

void BasicNetworkManager::stop_updating() {
    _sent_first_update = false;
}

void BasicNetworkManager::dump_networks() {
    NetworkList list;
    get_networks(&list);
    LOG(LS_NOTICE) << "NetworkManager detected " << list.size() << " networks:";
    for (const Network* network : list) {
        LOG(LS_NOTICE) << network->to_string() << ": " << network->description()
            << ", active ? " << network->active()
            << ((network->ignored()) ? ", Ignored" : "");
    }
}

bool BasicNetworkManager::is_ignored_network(const Network& network) const {
    // Ignore networks on the explicit ignore list.
    for (const std::string& ignored_name : _network_ignore_list) {
        if (network.name() == ignored_name) {
            return true;
        }
    }

    // Filter out VMware/VirtualBox interfaces, typically named vmnet1,
    // vmnet8, or vboxnet0.
    if (strncmp(network.name().c_str(), "vmnet", 5) == 0 ||
            strncmp(network.name().c_str(), "vnic", 4) == 0 ||
            strncmp(network.name().c_str(), "vboxnet", 7) == 0) {
        return true;
    }
    
    /*
    // Make sure this is a default route, if we're ignoring non-defaults.
    if (_ignore_non_default_routes && !is_default_route(network.name())) {
        return true;
    }
    */

    // Ignore any networks with a 0.x.y.z IP
    if (network.prefix().family() == AF_INET) {
        return (network.prefix().v4_address_as_host_order_integer() < 0x01000000);
    }

    return false;
}

bool BasicNetworkManager::create_networks(bool include_ignored,
        NetworkList* networks) const 
{
    struct ifaddrs* interfaces;
    int error = getifaddrs(&interfaces);
    if (error != 0) {
        LOG_ERR(LFATAL) << "getifaddrs failed to gather interface data: " << error;
        return false;
    }
    
    std::unique_ptr<IfAddrsConverter> ifaddrs_converter(create_if_addrs_converter());
    convert_if_addrs(interfaces, ifaddrs_converter.get(), include_ignored,
            networks);

    freeifaddrs(interfaces);
    return true;
}

void BasicNetworkManager::update_networks_once() {
    NetworkList list;
    if (!create_networks(false, &list)) {
        signal_error();
    } else {
        bool changed;
        NetworkManager::Stats stats;
        merge_network_list(list, &changed, &stats);
        if (changed || !_sent_first_update) {
            signal_networks_changed();
            _sent_first_update = true;
        }
    }
}

/////////////////// Network /////////////////////////

Network::Network(const std::string& name,
        const std::string& desc,
        const IPAddress& prefix,
        int prefix_length)
    : MemCheck("Network"),
    _name(name),
    _description(desc),
    _prefix(prefix),
    _prefix_length(prefix_length),
    _key(make_network_key(name, prefix, prefix_length)),
    _scope_id(0),
    _ignored(false),
    _type(ADAPTER_TYPE_UNKNOWN),
    _preference(0) 
{
}

Network::Network(const std::string& name,
        const std::string& desc,
        const IPAddress& prefix,
        int prefix_length,
        AdapterType type)
    : MemCheck("Network"),
    _name(name),
    _description(desc),
    _prefix(prefix),
    _prefix_length(prefix_length),
    _key(make_network_key(name, prefix, prefix_length)),
    _scope_id(0),
    _ignored(false),
    _type(type),
    _preference(0) 
{
}

Network::~Network() = default;

// Sets the addresses of this network. Returns true if the address set changed.
// Change detection is short circuited if the changed argument is true.
bool Network::set_IPs(const std::vector<InterfaceAddress>& ips, bool changed) {
    // Detect changes with a nested loop; n-squared but we expect on the order
    // of 2-3 addresses per network.
    changed = changed || ips.size() != _ips.size();
    if (!changed) {
        for (const InterfaceAddress& ip : ips) {
            if (std::find(_ips.begin(), _ips.end(), ip) == _ips.end()) {
                changed = true;
                break;
            }
        }
    }

    _ips = ips;
    return changed;
}

// Select the best IP address to use from this Network.
IPAddress Network::get_best_IP() const {
    if (_ips.size() == 0) {
        return IPAddress();
    }

    if (_prefix.family() == AF_INET) {
        return static_cast<IPAddress>(_ips.at(0));
    }

    InterfaceAddress selected_ip, ula_ip;

    for (const InterfaceAddress& ip : _ips) {
        // Ignore any address which has been deprecated already.
        if (ip.ipv6_flags() & IPV6_ADDRESS_FLAG_DEPRECATED) {
            continue;
        }

        // ULA address should only be returned when we have no other
        // global IP.
        if (IP_is_ULA(static_cast<const IPAddress&>(ip))) {
            ula_ip = ip;
            continue;
        }
        selected_ip = ip;

        // Search could stop once a temporary non-deprecated one is found.
        if (ip.ipv6_flags() & IPV6_ADDRESS_FLAG_TEMPORARY)
            break;
    }

    // No proper global IPv6 address found, use ULA instead.
    if (IP_is_unspec(selected_ip) && !IP_is_unspec(ula_ip)) {
        selected_ip = ula_ip;
    }

    return static_cast<IPAddress>(selected_ip);
}

std::string Network::to_string() const {
    std::stringstream ss;
    // Print out the first space-terminated token of the network desc, plus
    // the IP address.
    ss << "Net[" << _description.substr(0, _description.find(' '))
        << ":" << _prefix.to_sensitive_string() << "/" << _prefix_length
        << ":" << adapter_type_to_string(_type) << "]";
    return ss.str();
}

} // namespace rtcbase


