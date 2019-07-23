/***************************************************************************
* Copyright (c) 2018, Johan Mabille, Sylvain Corlay and Martin Renou       *
*                                                                          *
* Distributed under the terms of the BSD 3-Clause License.                 *
*                                                                          *
* The full license is in the file LICENSE, distributed with this software. *
****************************************************************************/

#include <string>

#include "zmq.hpp"
#include "zmq_addon.hpp"

namespace xeus
{
    class xserver_zmq_split;

    class xshell
    {
    public:

        using listener = std::function<void(zmq::multipart_t&)>;

        xshell(zmq::context_t& context,
               const std::string& transport,
               const std::string& ip,
               const std::string& shell_port,
               const std::string& sdtin_port,
               xserver_zmq_split* server);

        ~xshell();

        std::string get_shell_port() const;
        std::string get_stdin_port() const;

        void run();

        void send_shell(zmq::multipart_t& message);
        void send_stdin(zmq::multipart_t& message);
        void send_internal(zmq::multipart_t& message);
        void publish(zmq::multipart_t& message);
        void abort_queue(const listener& l, long polling_interval);

    private:

        zmq::socket_t m_shell;
        zmq::socket_t m_stdin;
        zmq::socket_t m_publisher_pub;
        zmq::socket_t m_controller;
        xserver_zmq_split* p_server;
    };
}

