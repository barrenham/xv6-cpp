module xv.net;
import :transport_algorithm_flows;
import :transport_buffer_flows;

namespace xv::net
{
bool TcpCongestion::duplicate(unsigned acknowledgement, unsigned flight, unsigned next)
{
  DuplicateFlow::Context ctx{*this, acknowledgement, flight, next};
  (void)DuplicateFlow::Graph::run<0>(ctx);
  return ctx.result;
}

bool TcpCongestion::acknowledged(unsigned bytes, unsigned acknowledgement)
{
  AcknowledgeFlow::Context ctx{*this, bytes, acknowledgement};
  (void)AcknowledgeFlow::Graph::run<0>(ctx);
  return ctx.result;
}

void RetransmissionTimer::acknowledge(unsigned acknowledgement, unsigned long now,
                                      bool outstanding)
{
  AcknowledgeFlow::Context ctx{*this, acknowledgement, now, outstanding};
  (void)AcknowledgeFlow::Graph::run<0>(ctx);
}
} // namespace xv::net

namespace xv::net
{
bool TcpPageBuffer::initialize()
{
  InitializeFlow::Context ctx{*this};
  (void)InitializeFlow::Graph::run<0>(ctx);
  return ctx.result;
}

unsigned TcpSendBuffer::append(const unsigned char *data, unsigned length)
{
  AppendFlow::Context ctx{*this, data, length};
  (void)AppendFlow::Graph::run<0>(ctx);
  return ctx.accepted;
}

unsigned TcpReceiveBuffer::insert(unsigned sequence, const unsigned char *data,
                                  unsigned length, bool fin)
{
  InsertFlow::Context ctx{*this, sequence, data, length, fin};
  (void)InsertFlow::Graph::run<0>(ctx);
  return ctx.result;
}
} // namespace xv::net
