#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <iostream>

namespace ssl = boost::asio::ssl;
namespace http = boost::beast::http;
namespace net = boost::asio;
using tcp = boost::asio::ip::tcp;

constexpr const static int VERSION = 11;

int main()
{
    try {
        net::io_context ioc;
        ssl::context ctx(ssl::context::tlsv12_client);
        tcp::resolver resolver(ioc);
        const tcp::resolver::results_type endpoints = resolver.resolve("dns.google", "https");

        ssl::stream<tcp::socket> stream(ioc, ctx);
        net::connect(stream.next_layer(), endpoints);
        stream.handshake(ssl::stream_base::handshake_type::client);

        http::request<http::string_body> req{http::verb::post, "/dns-query", VERSION};
        req.set(http::field::host, "dns.google");
        req.set(http::field::user_agent, "Boost Beast DoH Client");
        req.set(http::field::content_type, "application/dns-message");
        req.body() = "00 00 01 00 00 01 00 00  00 00 00 00 03 77 77 77 07 65 78 61 6d 70 6c 65 03 63 6f 6d 00 00 01 00 01";
        req.prepare_payload();

        http::write(stream, req);

        boost::beast::flat_buffer buffer;
        http::response<http::dynamic_body> res;
        http::read(stream, buffer, res);

        std::cout << "Response: " << res << std::endl;

        // Process the DNS response here

        stream.shutdown();
    } catch(std::exception& ex) {
        std::cerr << ex.what() << '\n';
    }

    return 0;
}
