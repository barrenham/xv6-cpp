function(xv_add_net_modules target)
  set(module_root "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/../kernel/net")
  target_sources(${target} PRIVATE FILE_SET net_modules TYPE CXX_MODULES
    BASE_DIRS "${module_root}" FILES
    "${module_root}/dhcp/message.cppm" "${module_root}/dhcp/lease.cppm" "${module_root}/dhcp/client.cppm"
    "${module_root}/link/port.cppm" "${module_root}/bridge/bridge.cppm"
    "${module_root}/net.cppm" "${module_root}/platform/platform.cppm"
    "${module_root}/interface/configuration.cppm" "${module_root}/core/control.cppm"
    "${module_root}/interface/interface.cppm" "${module_root}/interface/loopback.cppm"
    "${module_root}/interface/ethernet_interface.cppm" "${module_root}/interface/tunnel.cppm"
    "${module_root}/ip/routing.cppm"
    "${module_root}/core/address.cppm" "${module_root}/core/wire.cppm"
    "${module_root}/core/packet.cppm"
    "${module_root}/link/ethernet.cppm" "${module_root}/link/arp.cppm"
    "${module_root}/ip/ipv4.cppm" "${module_root}/ip/icmp.cppm"
    "${module_root}/transport/retransmission_timer.cppm"
    "${module_root}/transport/stream_buffer.cppm" "${module_root}/transport/congestion.cppm"
    "${module_root}/transport/udp.cppm" "${module_root}/transport/tcp.cppm"
    "${module_root}/socket/socket.cppm" "${module_root}/socket/registry.cppm"
    "${module_root}/socket/udp_socket.cppm" "${module_root}/socket/tcp_socket.cppm")
  target_sources(${target} PRIVATE
    "${module_root}/dhcp/message.cpp" "${module_root}/dhcp/lease.cpp" "${module_root}/dhcp/client.cpp"
    "${module_root}/interface/configuration.cpp" "${module_root}/core/control.cpp"
    "${module_root}/interface/interface.cpp" "${module_root}/interface/loopback.cpp"
    "${module_root}/interface/ethernet_interface.cpp" "${module_root}/interface/tunnel.cpp"
    "${module_root}/ip/routing.cpp"
    "${module_root}/bridge/bridge.cpp" "${module_root}/bridge/manager.cpp"
    "${module_root}/core/packet.cpp"
    "${module_root}/core/service.cpp"
    "${module_root}/link/ethernet.cpp" "${module_root}/link/arp.cpp"
    "${module_root}/ip/ipv4.cpp" "${module_root}/ip/icmp.cpp"
    "${module_root}/transport/udp.cpp" "${module_root}/transport/tcp_protocol.cpp"
    "${module_root}/socket/tcp_socket.cpp" "${module_root}/socket/tcp_sender.cpp" "${module_root}/socket/tcp_receiver.cpp" "${module_root}/socket/registry.cpp"
    "${module_root}/socket/inet_socket.cpp" "${module_root}/socket/udp_socket.cpp")
endfunction()
