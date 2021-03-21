#pragma once

#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>
#include <set>
#include <functional>
#include <fstream>

class Server {
protected:
    websocketpp::server<websocketpp::config::asio> endpoint;
    std::set<websocketpp::connection_hdl, std::owner_less<websocketpp::connection_hdl> > connections;
    std::thread* loop = nullptr;
    std::mutex lock;
    websocketpp::open_handler onOpenCallback;

    inline void run() {
        try {
            endpoint.listen(6723);
            endpoint.start_accept();
            endpoint.run();
        } catch (const std::exception& e) {}
    }

    inline void on_open(websocketpp::connection_hdl conn) {
        if (onOpenCallback) onOpenCallback(conn);
        std::lock_guard<std::mutex> guard(lock);
        connections.insert(conn);
    }

    inline void on_close(websocketpp::connection_hdl conn) {
        std::lock_guard<std::mutex> guard(lock);
        connections.erase(conn);
    }
public:
    inline Server() {
        endpoint.set_error_channels(websocketpp::log::elevel::all);
        endpoint.set_access_channels(websocketpp::log::alevel::all ^ websocketpp::log::alevel::frame_payload);
        endpoint.init_asio();

        endpoint.set_open_handler(std::bind(&Server::on_open, this, websocketpp::lib::placeholders::_1));
        endpoint.set_close_handler(std::bind(&Server::on_close, this, websocketpp::lib::placeholders::_1));
    }

    inline void stop() {
        if (!loop) return;
        if (endpoint.is_listening()) endpoint.stop();
        loop->join();
        loop = nullptr;
    }

    inline void start(websocketpp::open_handler onOpenCallback) {
        this->onOpenCallback = onOpenCallback;
        stop();
        loop = new std::thread(&Server::run, this);
    }

    inline void send(const std::string& data) {
        std::lock_guard<std::mutex> guard(lock);
        for (auto iter = connections.begin(); iter != connections.end(); ++iter) {
            endpoint.send(*iter, data, websocketpp::frame::opcode::text);
        }
    }

    inline void send(websocketpp::connection_hdl conn, const std::string& data) {
        endpoint.send(conn, data, websocketpp::frame::opcode::text);
    }
};