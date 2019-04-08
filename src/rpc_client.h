
#pragma once

#include <boost/optional.hpp>
#include "common.h"

struct Message {
    u64* payload;
    size_t len;
    bool is_binary;
};

class RPCClient {
public:
    explicit RPCClient();

    ~RPCClient();

    /**
     * Pushes a message to the read queue for the client to process.
     */
    void PushMessage(Message&&);

    /**
     * Removes messages from the write queue and returns the data
     */
    boost::optional<Message> PopPending();

private:
};
