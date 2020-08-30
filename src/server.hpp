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
    void(*openCB)(websocketpp::connection_hdl) = nullptr;

    inline void run() {
        endpoint.listen(6723);
	    endpoint.start_accept();
	    endpoint.run();        
    }

    inline void on_open(websocketpp::connection_hdl conn) {
        if (openCB) openCB(conn);
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
        endpoint.stop();
        loop->join();
        loop = nullptr;
    }

    inline void start(void(*openCB)(websocketpp::connection_hdl)) {
        this->openCB = openCB;
        stop();
        loop = new std::thread(&Server::run, this);
    }

    inline void send(const std::string& data) {
        std::cout << "sending data: " << data << std::endl;
        std::lock_guard<std::mutex> guard(lock);
        for (auto iter = connections.begin(); iter != connections.end(); ++iter) {
            endpoint.send(*iter, data, websocketpp::frame::opcode::text);
        }
    }

    inline void send(websocketpp::connection_hdl conn, const std::string& data) {
        std::cout << "sending data(single): " << data << std::endl;
        endpoint.send(conn, data, websocketpp::frame::opcode::text);
    }
};