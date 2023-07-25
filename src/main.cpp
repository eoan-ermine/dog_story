#include "util/logging.hpp"
#include "util/sdk.hpp"

#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/utility/setup/console.hpp>

#include <boost/asio/io_context.hpp>
#include <boost/asio/signal_set.hpp>
#include <boost/log/core.hpp>
#include <iostream>
#include <thread>

#include "json_loader.hpp"
#include "request_handler.hpp"

using namespace std::literals;
namespace net = boost::asio;
namespace logging = boost::log;

namespace {

// Запускает функцию fn на n потоках, включая текущий
template <typename Fn>
void RunWorkers(unsigned num_threads, const Fn &fn) {
    num_threads = std::max(1u, num_threads);
    std::vector<std::jthread> workers;
    workers.reserve(num_threads - 1);
    // Запускаем num_threads-1 рабочих потоков, выполняющих функцию fn
    while (--num_threads) {
        workers.emplace_back(fn);
    }
    fn();
}

} // namespace

int main(int argc, const char *argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: game_server <game-config-json> <static-content-directory>"sv << std::endl;
        return EXIT_FAILURE;
    }

    // 0. Инициализируем логер
    logging::add_console_log(std::clog, logging::keywords::format = &LogFormatter);
    logging::add_common_attributes();

    try {
        // 1. Загружаем карту из файла и построить модель игры
        model::Game game = json_loader::LoadGame(argv[1]);

        // 2. Инициализируем io_context
        const unsigned num_threads = std::thread::hardware_concurrency();
        net::io_context ioc(num_threads);

        // 3. Добавляем асинхронный обработчик сигналов SIGINT и SIGTERM
        net::signal_set signals(ioc, SIGINT, SIGTERM);
        signals.async_wait([&ioc](const boost::system::error_code &ec, int signal_number) {
            if (!ec) {
                ioc.stop();
            }
        });

        // 4. Создаём обработчик HTTP-запросов и связываем его с моделью игры
        request_handler::RequestHandler handler{game, argv[2]};

        // 5. Запустить обработчик HTTP-запросов, делегируя их обработчику запросов
        const auto address = net::ip::make_address("0.0.0.0");
        constexpr net::ip::port_type port = 8080;
        http_server::ServeHttp(ioc, {address, port}, [&handler](auto &&addr, auto &&req, auto &&send) {
            handler(std::forward<decltype(addr)>(addr), std::forward<decltype(req)>(req),
                    std::forward<decltype(send)>(send));
        });

        // 6. Запускаем обработку асинхронных операций
        LogStart(address.to_string(), port);
        RunWorkers(std::max(1u, num_threads), [&ioc] { ioc.run(); });

        // Логирование успешного завершения программы
        LogExit(EXIT_SUCCESS);

        return EXIT_SUCCESS;
    } catch (const std::exception &ex) {
        // Логирование завершения программы с ошибкой
        LogExit(EXIT_FAILURE, ex.what());

        return EXIT_FAILURE;
    }
}
