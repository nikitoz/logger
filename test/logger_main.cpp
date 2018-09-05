#include "logger.h"
#include "netw.h"
#include "netw_replicator.h"

#define MAYBE(x) {auto xx = x;if (xx != 0) { printf("Error %d\n", xx); break; }}

int main() {
    const char* logfolder = "/tmp";
    context_t context("test context", logfolder);
    logger_t logger;
    do {
        MAYBE(context.make_logger("user", 3, &logger));
        appender_t appender;
        MAYBE(logger.make_appender(&appender));
        netw_replicator_t repli("localhost", 8978);
        auto replicate = std::bind(&netw_replicator_t::replicate, &repli,
                                   std::placeholders::_1, std::placeholders::_2,
                                   std::placeholders::_3);
        appender.set_replicator(std::move(replicate));
        MAYBE(appender.append("hello"));
        MAYBE(appender.append("world"));
        reader_t reader;
        MAYBE(logger.make_reader(&reader));
        char data[256] = {0};
        MAYBE(reader.consume_one_str(data, 256));
        MAYBE(strcmp(data, "hello"));
        data[0] = 0;
        MAYBE(reader.consume_one_str(data, 256));
        MAYBE(strcmp(data, "world"));
        fprintf(stdout, "Success\n");
    } while (false);
    remove("/tmp/user");
    return 0;
}
