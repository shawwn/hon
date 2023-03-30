static struct fetcherr _http_errlist[] = {
    { 100, FETCH_OK, "Continue" },
    { 101, FETCH_OK, "Switching Protocols" },
    { 200, FETCH_OK, "OK" },
    { 201, FETCH_OK, "Created" },
    { 202, FETCH_OK, "Accepted" },
    { 203, FETCH_INFO, "Non-Authoritative Information" },
    { 204, FETCH_OK, "No Content" },
    { 205, FETCH_OK, "Reset Content" },
    { 206, FETCH_OK, "Partial Content" },
    { 300, FETCH_MOVED, "Multiple Choices" },
    { 301, FETCH_MOVED, "Moved Permanently" },
    { 302, FETCH_MOVED, "Moved Temporarily" },
    { 303, FETCH_MOVED, "See Other" },
    { 304, FETCH_OK, "Not Modified" },
    { 305, FETCH_INFO, "Use Proxy" },
    { 307, FETCH_MOVED, "Temporary Redirect" },
    { 400, FETCH_PROTO, "Bad Request" },
    { 401, FETCH_AUTH, "Unauthorized" },
    { 402, FETCH_AUTH, "Payment Required" },
    { 403, FETCH_AUTH, "Forbidden" },
    { 404, FETCH_UNAVAIL, "Not Found" },
    { 405, FETCH_PROTO, "Method Not Allowed" },
    { 406, FETCH_PROTO, "Not Acceptable" },
    { 407, FETCH_AUTH, "Proxy Authentication Required" },
    { 408, FETCH_TIMEOUT, "Request Time-out" },
    { 409, FETCH_EXISTS, "Conflict" },
    { 410, FETCH_UNAVAIL, "Gone" },
    { 411, FETCH_PROTO, "Length Required" },
    { 412, FETCH_SERVER, "Precondition Failed" },
    { 413, FETCH_PROTO, "Request Entity Too Large" },
    { 414, FETCH_PROTO, "Request-URI Too Large" },
    { 415, FETCH_PROTO, "Unsupported Media Type" },
    { 416, FETCH_UNAVAIL, "Requested Range Not Satisfiable" },
    { 417, FETCH_SERVER, "Expectation Failed" },
    { 500, FETCH_SERVER, "Internal Server Error" },
    { 501, FETCH_PROTO, "Not Implemented" },
    { 502, FETCH_SERVER, "Bad Gateway" },
    { 503, FETCH_TEMP, "Service Unavailable" },
    { 504, FETCH_TIMEOUT, "Gateway Time-out" },
    { 505, FETCH_PROTO, "HTTP Version not supported" },
    { 999, FETCH_PROTO, "Protocol error" },
    { -1, FETCH_UNKNOWN, "Unknown HTTP error" }
};
