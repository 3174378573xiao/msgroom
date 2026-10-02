#include <iostream>
#include <string>
#include <vector>
#include <mutex>
#include <algorithm>
#include "httplib.h"

using namespace httplib;

struct ClientConn
{
    ws::WebSocket* ws_ptr;
    std::string nick;
};

int main()
{
    SetConsoleOutputCP(CP_UTF8);

    Server server;
    std::vector<ClientConn> clients;
    std::mutex mtx;
    int nick_id_counter = 0;

    // 开启心跳检测，丢失2次pong判定掉线
    server.set_websocket_ping_interval(10);
    server.set_websocket_max_missed_pongs(2);

    server.WebSocket("/chat", [&](const Request& req, ws::WebSocket& ws) {
        // 新客户端接入，分配昵称
        std::string nick = "用户" + std::to_string(++nick_id_counter);
        {
            std::lock_guard<std::mutex> lock(mtx);
            clients.push_back({&ws, nick});
        }

        // 系统广播：有人上线
        {
            std::lock_guard<std::mutex> lock(mtx);
            for (auto& c : clients)
            {
                c.ws_ptr->send("[系统] " + nick + " 已进入聊天室");
            }
        }
        std::cout << nick << " 已连接，IP:" << req.remote_addr << "\n";

        std::string msg;
        while (ws.read(msg))
        {
            // 用户发送普通聊天消息，广播所有人
            std::lock_guard<std::mutex> lock(mtx);
            for (auto& c : clients)
            {
                c.ws_ptr->send("[" + nick + "] " + msg);
            }
        }

        // --------客户端断开，清理连接--------
        std::cout << nick << " 断开连接\n";
        {
            std::lock_guard<std::mutex> lock(mtx);
            auto it = std::find_if(clients.begin(), clients.end(), [&](const ClientConn& cc) {
                return cc.ws_ptr == &ws;
            });
            if (it != clients.end())
            {
                std::string leave_nick = it->nick;
                clients.erase(it);

                //广播下线通知
                for (auto& c : clients)
                {
                    c.ws_ptr->send("[系统] " + leave_nick + " 离开聊天室");
                }
            }
        }
    });

    std::cout << "聊天室服务端启动 ws://0.0.0.0:8080/chat\n";
    server.listen("0.0.0.0", 8080);
    return 0;
}
