#undef TRACE_SYSTEM
#define TRACE_SYSTEM twbnet

#if !defined(_TWBNET_TRACE_H_) || defined(TRACE_HEADER_MULTI_READ)
#define _TWBNET_TRACE_H_

#include <linux/tracepoint.h>
#include <linux/ip.h>
#include <linux/netdevice.h>

TRACE_EVENT(
	twbnet_start_xmit,

	TP_PROTO(struct sk_buff *skb, struct net_device *dev),

	TP_ARGS(skb, dev),

	TP_STRUCT__entry(
		__array( 	unsigned char, src, 		4 	)
		__array( 	unsigned char, dst, 		4 	)
	),

	TP_fast_assign(
		memcpy(__entry->src, &((struct iphdr *) (skb->data + sizeof(struct ethhdr)))->saddr, 4);
		memcpy(__entry->dst, &((struct iphdr *) (skb->data + sizeof(struct ethhdr)))->daddr, 4);
	),

	TP_printk("[twbnet] skb packet: %d.%d.%d.%d --> %d.%d.%d.%d",
									__entry->src[0], __entry->src[1], __entry->src[2], __entry->src[3],
									__entry->dst[0], __entry->dst[1], __entry->dst[2], __entry->dst[3]
	)
);

#endif /* _TWBNET_TRACE_H_ */

#undef TRACE_INCLUDE_PATH
#define TRACE_INCLUDE_PATH .
#define TRACE_INCLUDE_FILE twb_net_trace
#include <trace/define_trace.h>
