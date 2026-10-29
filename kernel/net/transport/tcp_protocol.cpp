module xv.net;
import :tcp_receive_flow;
import :tcp_transmit_flow;

namespace xv::net
{

int TcpProtocol::transmit(Endpoint local, Endpoint peer, unsigned sequence,
                          unsigned acknowledgement, unsigned flags, unsigned window,
                          const void *data, unsigned length)
{
  TransmitFlow::Context ctx{ip_,   local,  peer, sequence, acknowledgement,
                            flags, window, data, length};
  (void)TransmitFlow::Graph::run<0>(ctx);
  return ctx.result;
}

void TcpProtocol::receive(Packet &packet, const IpDatagram &datagram)
{
  ReceiveFlow::Context ctx{*this, sink_, packet, datagram};
  (void)ReceiveFlow::Graph::run<0>(ctx);
}
} // namespace xv::net
