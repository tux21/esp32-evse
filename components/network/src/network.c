#include "network.h"

#include "discovery.h"
#include "eth.h"
#include "wifi.h"

void network_init(void)
{
    wifi_init();
    eth_init();
    discovery_init();
}
