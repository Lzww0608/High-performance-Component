

#include <chrono>
#include <memory>
#include <spdlog/async.h>
#include <spdlog/async_logger.h>
#include <spdlog/common.h>
#include <spdlog/logger.h>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>

class my_formatter_flag : public spdlog::custom_flag_formatter
{
public:
	// Override the format method to add custom text to log messages.
    void format(const spdlog::details::log_msg &, const std::tm &, spdlog::memory_buf_t &dest) override
    {
        std::string some_txt = "mark-flag"; // Custom text to insert
        dest.append(some_txt.data(), some_txt.data() + some_txt.size());
    }
    std::unique_ptr<custom_flag_formatter> clone() const override
    {
        return spdlog::details::make_unique<my_formatter_flag>();
    }
};

int main() {

    spdlog::info("hello world");
	
	// create an async logger
    auto logger = spdlog::stdout_color_mt<spdlog::async_factory>("console");
    logger->info("hello Lzww");
    spdlog::get("console")->info("hello world and Lzww");

	// logger -> sink
    auto sink1 = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    auto sink2 = std::make_shared<spdlog::sinks::basic_file_sink_mt>("lzww.txt");

	// create a formatter
    auto formatter = std::make_unique<spdlog::pattern_formatter>();
    formatter->add_flag<my_formatter_flag>('*').set_pattern("[%n] [%*] [%^%l%$] %v"); // pattern
    sink1->set_formatter(std::move(formatter));

    sink2->set_pattern("[%^%l%$] %v");

    auto logger1 = std::make_shared<spdlog::logger>(std::string("console1"));

    logger1->sinks().push_back(sink1);
    logger1->sinks().push_back(sink2);
    spdlog::register_logger(logger1);

    spdlog::get("console1")->info("hello my Lzww");

    SPDLOG_INFO("hello lzww again");
    SPDLOG_LOGGER_INFO(spdlog::get("console1"), "hello my Lzww !!");


    auto logger_mark = spdlog::create_async<spdlog::sinks::basic_file_sink_mt>("Lzww", "Lzww1.txt");
    logger_mark->info("hello logger_Lzww");

	// 队列长度8292,最大消息数量
	// 线程池中线程数量为8
    spdlog::init_thread_pool(8292, 8);

    std::vector<spdlog::sink_ptr> sinks;
    sinks.push_back(sink1);
    sinks.push_back(sink2);
    auto logger_tp = std::make_shared<spdlog::async_logger>("tp", sinks.begin(), sinks.end(), spdlog::thread_pool(), spdlog::async_overflow_policy::overrun_oldest);

    logger_tp->info("hello thread pool");

    logger_tp->flush();

    logger_tp->flush_on(spdlog::level::err);

    spdlog::flush_every(std::chrono::seconds(5));

    return 0;
}