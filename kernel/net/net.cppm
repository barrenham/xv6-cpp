export module xv.net;
export import :socket;
export import :ethernet_interface;
export import :loopback;
export import :tunnel;
export namespace xv::net
{
void initialize(NetworkSetup *setup = nullptr);
const InterfaceRegistry &interfaces();
const RoutingTable &routing_table();
void timer_tick();
int bind(unsigned short port);
int unbind(unsigned short port);
int send(unsigned short source, unsigned destination, unsigned short port,
         unsigned long data, unsigned length);
int receive(unsigned short port, unsigned long source, unsigned long source_port,
            unsigned long data, unsigned capacity);
bool inherit(int parent, int child);
void release_owner(int owner);
}
