/***************************************************************************
* Copyright (c) 2018, Johan Mabille, Sylvain Corlay and Martin Renou       *
*                                                                          *
* Distributed under the terms of the BSD 3-Clause License.                 *
*                                                                          *
* The full license is in the file LICENSE, distributed with this software. *
****************************************************************************/

#include <thread>
#include <chrono>
#include <iostream>

#include "xeus/xmiddleware.hpp"
#include "xeus/xserver_zmq_split.hpp"
#include "xshell.hpp"

namespace xeus
{
    xshell::xshell(zmq::context_t& context,
                   const std::string& transport,
                   const std::string& ip,
                   const std::string& shell_port,
                   const std::string& stdin_port,
                   xserver_zmq_split* server)
        : m_shell(context, zmq::socket_type::router)
        , m_stdin(context, zmq::socket_type::router)
        , m_publisher_pub(context, zmq::socket_type::pub)
        , m_controller(context, zmq::socket_type::rep)
        , p_server(server)
    {
        init_socket(m_shell, transport, ip, shell_port);
        init_socket(m_stdin, transport, ip, stdin_port);
        m_publisher_pub.setsockopt(ZMQ_LINGER, get_socket_linger());
        m_publisher_pub.connect(get_publisher_end_point());

        m_controller.setsockopt(ZMQ_LINGER, get_socket_linger());
        m_controller.bind(get_controller_end_point("shell"));
    }

    xshell::~xshell()
    {
    }

    std::string xshell::get_shell_port() const
    {
        return get_socket_port(m_shell);
    }

    std::string xshell::get_stdin_port() const
    {
        return get_socket_port(m_stdin);
    }

    void xshell::run()
    {
        zmq::pollitem_t items[] = {
            { m_shell, 0, ZMQ_POLLIN, 0 },
            { m_controller, 0, ZMQ_POLLIN, 0 }
        };

        while (true)
        {
            zmq::poll(&items[0], 2, -1);

            if (items[0].revents & ZMQ_POLLIN)
            {
                zmq::multipart_t wire_msg;
                wire_msg.recv(m_shell);
                p_server->notify_shell_listener(wire_msg);
            }

            if (items[1].revents & ZMQ_POLLIN)
            {
                // stop message
                zmq::multipart_t wire_msg;
                wire_msg.recv(m_controller);
                const zmq::message_t& msg = wire_msg[0];
                if(std::string(msg.data<const char>(), msg.size()) != "stop")
                {
                    p_server->notify_internal_listener(wire_msg);
                }
                else
                {
                    wire_msg.send(m_controller);
                    break;
                }
            }
        }
    }

    void xshell::send_shell(zmq::multipart_t& message)
    {
        message.send(m_shell);
    }

    void xshell::send_stdin(zmq::multipart_t& message)
    {
        message.send(m_stdin);
        zmq::multipart_t wire_msg;
        wire_msg.recv(m_stdin);
        p_server->notify_stdin_listener(wire_msg);
    }

    void xshell::send_internal(zmq::multipart_t& message)
    {
        message.send(m_controller);
    }

    void xshell::publish(zmq::multipart_t& message)
    {
        message.send(m_publisher_pub);
    }

    void xshell::abort_queue(const listener& l, long polling_interval)
    {
        while (true)
        {
            zmq::multipart_t wire_msg;
            bool msg = wire_msg.recv(m_shell, ZMQ_NOBLOCK);
            if (!msg)
            {
                return;
            }

            l(wire_msg);
            std::this_thread::sleep_for(std::chrono::milliseconds(polling_interval));
        }
    }
}

