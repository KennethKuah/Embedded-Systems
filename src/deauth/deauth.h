#ifndef DEAUTH_HEADER
#define DEAUTH_HEADER
#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "lwip/pbuf.h"
#include "lwip/prot/ethernet.h"

#define DEAUTH_FRAME_CONTROL_FIELD 0xc000
#define DEAUTH_REASON_UNSPECIFIED 0x0100
#define DEAUTH_REASON_NON_ASSOCIATED_STA 0x0700

PACK_STRUCT_BEGIN
struct ieee_80211_hdr {
  PACK_STRUCT_FLD_8(u16_t  frame_control_field);
  PACK_STRUCT_FLD_8(u16_t  duration);
  PACK_STRUCT_FLD_S(struct eth_addr ethaddr_dest);
  PACK_STRUCT_FLD_S(struct eth_addr ethaddr_src);
  PACK_STRUCT_FLD_S(struct eth_addr ethaddr_bssid);
  PACK_STRUCT_FLD_8(u16_t  sequence_control); // incremented by adding 0x10
  PACK_STRUCT_FLD_8(u16_t  reason_code);
} PACK_STRUCT_STRUCT;
PACK_STRUCT_END

bool joinAP(char *ssid, char *password);
void deauth();
void deauth_blocking(struct pbuf* p);

#endif