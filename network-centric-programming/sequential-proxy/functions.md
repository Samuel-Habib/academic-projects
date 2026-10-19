main: Sets up a listening socket and processes incoming requests.
process_request: Handles parsing and forwarding client requests.
parse_url: Extracts the hostname, path, and port from a given URL.
forward_request: Sends the parsed request to the server and includes necessary headers.
log_request: Logs request and response size to proxy.log.
format_log_entry: Formats the log entry with timestamp and client IP address.
