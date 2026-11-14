// Typecheck test (not run): the Network C API that the Network Swift overlay calls, as Swift imports it, and the
// helpers of the overlay's shim module. Typecheck with the overlay shim module map, for example
//   swiftc -frontend -typecheck network_import.swift -Xcc -fmodule-map-file=<darling-swift>/overlays/shims/overlay-shims.modulemap ...
import Dispatch
import Foundation
import Network
import _SwiftNetworkOverlayShims

func parameters() {
    let params = nw_parameters_create()
    let copy: nw_parameters_t = nw_parameters_copy(params)
    let stack: nw_protocol_stack_t = nw_parameters_copy_default_protocol_stack(copy)
    let tcp: nw_protocol_options_t = nw_tcp_create_options()
    nw_tcp_options_set_no_delay(tcp, true)
    nw_tcp_options_set_keepalive_count(tcp, 3)
    nw_protocol_stack_set_transport_protocol(stack, tcp)
    nw_protocol_stack_prepend_application_protocol(stack, nw_tls_create_options())
    nw_protocol_stack_iterate_application_protocols(stack) { (options: nw_protocol_options_t) in
        let definition: nw_protocol_definition_t = nw_protocol_options_copy_definition(options)
        _ = nw_protocol_definition_is_equal(definition, nw_protocol_copy_tls_definition())
    }
    let transport: nw_protocol_options_t? = nw_protocol_stack_copy_transport_protocol(stack)
    let internet: nw_protocol_options_t? = nw_protocol_stack_copy_internet_protocol(stack)
    _ = (transport, internet)
    nw_protocol_stack_clear_application_protocols(stack)

    nw_parameters_set_local_only(params, true)
    nw_parameters_set_prohibit_expensive(params, false)
    nw_parameters_set_prefer_no_proxy(params, false)
    nw_parameters_set_reuse_local_address(params, true)
    nw_parameters_set_fast_open_enabled(params, true)
    nw_parameters_set_include_peer_to_peer(params, true)
    nw_parameters_set_service_class(params, nw_service_class_responsive_data)
    nw_parameters_set_multipath_service(params, nw_multipath_service_handover)
    nw_parameters_set_expired_dns_behavior(params, nw_parameters_expired_dns_behavior_allow)
    nw_parameters_set_required_interface_type(params, nw_interface_type_wifi)
    nw_parameters_prohibit_interface_type(params, nw_interface_type_cellular)
    nw_parameters_iterate_prohibited_interface_types(params) { (type: nw_interface_type_t) -> Bool in
        return type == nw_interface_type_cellular
    }
    nw_parameters_clear_prohibited_interface_types(params)
    nw_parameters_iterate_prohibited_interfaces(params) { (interface: nw_interface_t) -> Bool in
        return nw_interface_get_index(interface) != 0
    }
    nw_parameters_clear_prohibited_interfaces(params)
    let service: nw_service_class_t = nw_parameters_get_service_class(params)
    let multipath: nw_multipath_service_t = nw_parameters_get_multipath_service(params)
    let dns: nw_parameters_expired_dns_behavior_t = nw_parameters_get_expired_dns_behavior(params)
    let flags: [Bool] = [nw_parameters_get_local_only(params), nw_parameters_get_prohibit_expensive(params),
                         nw_parameters_get_prefer_no_proxy(params), nw_parameters_get_reuse_local_address(params),
                         nw_parameters_get_fast_open_enabled(params), nw_parameters_get_include_peer_to_peer(params)]
    _ = (service, multipath, dns, flags)

    let local: nw_endpoint_t? = nw_parameters_copy_local_endpoint(params)
    nw_parameters_set_local_endpoint(params, local)
    let required: nw_interface_t? = nw_parameters_copy_required_interface(params)
    nw_parameters_require_interface(params, required)
    _ = nw_parameters_get_required_interface_type(params)

    let ip = _swift_nw_parameters_create_custom_ip(47)
    _ = ip
}

func endpoints() {
    let host = nw_endpoint_create_host("example.org", "80")
    let bonjour = nw_endpoint_create_bonjour_service("name", "_http._tcp", "local")
    var address = sockaddr_in()
    let ip = withUnsafePointer(to: &address) { $0.withMemoryRebound(to: sockaddr.self, capacity: 1) { nw_endpoint_create_address($0) } }
    for endpoint in [host, bonjour, ip] {
        switch nw_endpoint_get_type(endpoint) {
        case nw_endpoint_type_address: _ = nw_endpoint_get_address(endpoint)
        case nw_endpoint_type_host: _ = nw_endpoint_get_hostname(endpoint)
        case nw_endpoint_type_bonjour_service:
            _ = String(cString: nw_endpoint_get_bonjour_service_name(endpoint))
            _ = String(cString: nw_endpoint_get_bonjour_service_type(endpoint))
            _ = String(cString: nw_endpoint_get_bonjour_service_domain(endpoint))
        default: break
        }
    }
    if let unixEndpoint = nw_endpoint_create_unix("/tmp/socket") {
        nw_endpoint_set_interface(unixEndpoint, nw_endpoint_copy_interface(unixEndpoint))
    }
    let string = nw_endpoint_copy_address_string(host)
    free(string)
    _ = nw_interface_create_with_name("en0")
    _ = nw_interface_create_with_index(1)
    _ = nwlog_get_string_for_dns_service_error(0)
}

func connections(queue: DispatchQueue, endpoint: nw_endpoint_t, params: nw_parameters_t) {
    let connection = nw_connection_create(endpoint, params)
    nw_connection_set_state_changed_handler(connection) { (state: nw_connection_state_t, error: nw_error_t?) in
        if let error = error {
            _ = (nw_error_get_error_domain(error), nw_error_get_error_code(error))
        }
    }
    nw_connection_set_path_changed_handler(connection) { (path: nw_path_t) in
        _ = nw_path_copy_effective_local_endpoint(path)
        _ = nw_path_copy_effective_remote_endpoint(path)
    }
    nw_connection_set_viability_changed_handler(connection) { (viable: Bool) in _ = viable }
    nw_connection_set_better_path_available_handler(connection) { (better: Bool) in _ = better }
    nw_connection_set_viability_changed_handler(connection, nil)
    nw_connection_batch(connection) { _ = 0 }
    nw_connection_receive_message(connection) { (data, context, complete, error) in
        _ = (data, context, complete, error)
    }
    _ = nw_connection_copy_current_path(connection)
    _ = nw_connection_copy_protocol_metadata(connection, nw_protocol_copy_tcp_definition())
    _ = nw_connection_get_maximum_datagram_size(connection)
    nw_connection_restart(connection)
    nw_connection_cancel_current_endpoint(connection)
    nw_connection_force_cancel(connection)

    let context = nw_content_context_create("context")
    nw_content_context_set_is_final(context, true)
    nw_content_context_set_expiration_milliseconds(context, 10)
    nw_content_context_set_relative_priority(context, 0.5)
    nw_content_context_set_antecedent(context, nil)
    nw_content_context_set_metadata_for_protocol(context, nw_ip_create_metadata())
    nw_content_context_foreach_protocol_metadata(context) { (definition: nw_protocol_definition_t, metadata: nw_protocol_metadata_t) in
        _ = (nw_protocol_metadata_is_ip(metadata), nw_protocol_metadata_is_tcp(metadata),
             nw_protocol_metadata_is_udp(metadata), nw_protocol_metadata_is_tls(metadata))
    }
    _ = (String(cString: nw_content_context_get_identifier(context)), nw_content_context_get_is_final(context))
    _ = nw_content_context_copy_protocol_metadata(context, nw_protocol_copy_ip_definition())
    _ = nw_tls_copy_sec_protocol_metadata(nw_ip_create_metadata())

    _swift_nw_connection_send(connection, nil, context, true, { _ in })
    _swift_nw_connection_send_idempotent(connection, nil, nil, true)
    _ = (_swift_nw_content_context_default_message(), _swift_nw_content_context_final_message(),
         _swift_nw_content_context_default_stream())
    _ = _swift_nw_data_transfer_report_all_paths()
    _ = NWCreateNSDataFromDispatchData(nil)
    _ = NWCreateDispatchDataFromNSData(nil)
}

func listeners(queue: DispatchQueue, params: nw_parameters_t, connection: nw_connection_t) {
    guard let listener = nw_listener_create_with_port("8080", params) else { return }
    _ = nw_listener_create(params)
    _ = nw_listener_create_with_connection(connection, params)
    nw_listener_set_queue(listener, queue)
    nw_listener_set_new_connection_handler(listener) { (connection: nw_connection_t) in _ = connection }
    nw_listener_set_state_changed_handler(listener) { (state: nw_listener_state_t, error: nw_error_t?) in
        _ = (state == nw_listener_state_ready, error)
    }
    nw_listener_set_advertised_endpoint_changed_handler(listener) { (endpoint: nw_endpoint_t, added: Bool) in
        _ = (endpoint, added)
    }
    let descriptor = nw_advertise_descriptor_create_bonjour_service("name", "_http._tcp", nil)
    var txt: [UInt8] = [0]
    if let descriptor = descriptor {
        nw_advertise_descriptor_set_txt_record(descriptor, &txt, txt.count)
        nw_listener_set_advertise_descriptor(listener, descriptor)
    }
    nw_listener_start(listener)
    _ = nw_listener_get_port(listener)
    nw_listener_cancel(listener)
}
