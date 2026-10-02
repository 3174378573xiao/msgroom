#include <iostream>
#include <string>
#include <sstream>
#include <thread>
#include <atomic>
#include "httplib.h"

using namespace httplib;

// 后台接收消息循环
void recv_loop(ws::WebSocketClient& ws_client, std::atomic<bool>& running)
{
    std::string msg;
    while (running)
    {
        auto res = ws_client.read(msg);
        if (res == ws::ReadResult::Fail)
        {
            std::cout << "\n[连接断开，服务器关闭或者网络异常]\n>>>";
            break;
        }
        std::cout << "\n" << msg << "\n>>>";
    }
}

int main()
{
    SetConsoleOutputCP(CP_UTF8);
    std::string server_ip;
    const int port = 8080;

    std::cout << "请输入服务器IP：";
    std::getline(std::cin, server_ip);

    std::string url = "ws://" + server_ip + ":" + std::to_string(port) + "/chat";
    ws::WebSocketClient ws_client(url);

    auto connect_res = ws_client.connect();
    if (!connect_res)
    {
        std::cout << "连接失败！检查IP、防火墙、服务端是否运行\n";
        system("pause");
        return 0;
    }

    // 开启心跳掉线检测
    ws_client.set_websocket_ping_interval(10);
    ws_client.set_websocket_max_missed_pongs(2);

    std::atomic<bool> running{true};
    std::thread recv_th(recv_loop, std::ref(ws_client), std::ref(running));

    std::cout << "✅ 连接聊天室成功！命令：send 消息内容 | exit 退出\n>>>";

    std::string command;
    while (std::getline(std::cin, command))
    {
        std::istringstream iss(command);
        std::string subcmd;
        iss >> subcmd;

        if (subcmd == "exit")
        {
            running = false;
            ws_client.close(ws::CloseStatus::GoingAway, "客户端主动退出");
            break;
        }
        else if (subcmd == "send")
        {
            iss >> std::ws;
            std::string content;
            std::getline(iss, content);
            if (content.empty())
            {
                std::cout << "消息不能为空！\n>>>";
                continue;
            }
            ws_client.send(content);
        }
        else
        {
            std::cout << "可用命令 send <文本> , exit\n>>>";
        }
    }

    recv_th.join();
    std::cout << "客户端退出\n";
    system("pause");
    return 0;
}
