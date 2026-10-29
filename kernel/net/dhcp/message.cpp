module xv.net;
import :dhcp_option_flow;
import :dhcp_codec_flow;
import :dhcp_message;
import :wire;

namespace xv::net
{
unsigned DhcpCodec::encode(unsigned char *b, unsigned capacity, const DhcpRequest &r)
{
  EncodeFlow::Context ctx{b, capacity, r};
  (void)EncodeFlow::Graph::run<0>(ctx);
  return ctx.result;
}

bool DhcpCodec::options(const unsigned char *data, unsigned length, DhcpReply &r,
                        unsigned &overload, bool main)
{
  OptionsFlow::Context ctx{data, length, r, overload, main};
  (void)OptionsFlow::Graph::run<0>(ctx);
  return ctx.result;
}

bool DhcpCodec::decode(const unsigned char *b, unsigned length, DhcpReply &r)
{
  DecodeFlow::Context ctx{b, length, r};
  (void)DecodeFlow::Graph::run<0>(ctx);
  return ctx.result;
}

bool DhcpCodec::lease(const DhcpReply &r, DhcpLease &result)
{
  LeaseFlow::Context ctx{r, result};
  (void)LeaseFlow::Graph::run<0>(ctx);
  return ctx.result;
}
} // namespace xv::net
