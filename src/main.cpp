#include "util/sdk.hpp"

#include <boost/asio/io_context.hpp>
#include <boost/asio/signal_set.hpp>
#include <iostream>
#include <thread>

#include "json_loader.hpp"
#include "request_handler.hpp"

using namespace std::literals;
namespace net = boost::asio;

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
    if (argc != 2) {
        std::cerr << "Usage: game_server <game-config-json>"sv << std::endl;
        return EXIT_FAILURE;
    }
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
        request_handler::RequestHandler handler{game};

        // 5. Запустить обработчик HTTP-запросов, делегируя их обработчику запросов
        const auto address = net::ip::make_address("0.0.0.0");
        constexpr net::ip::port_type port = 8080;
        http_server::ServeHttp(ioc, {address, port}, [&handler](auto &&req, auto &&send) {
            handler(std::forward<decltype(req)>(req), std::forward<decltype(send)>(send));
        });

        // Эта надпись сообщает тестам о том, что сервер запущен и готов обрабатывать запросы
        std::cout << "Server has started..."sv << std::endl;

        // 6. Запускаем обработку асинхронных операций
        RunWorkers(std::max(1u, num_threads), [&ioc] { ioc.run(); });

        return EXIT_SUCCESS;
    } catch (const std::exception &ex) {
        std::cerr << ex.what() << std::endl;
        return EXIT_FAILURE;
    }
}
