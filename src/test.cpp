#include "Channel.h"
#include <cassert>
#include <iostream>
#include <thread>
#include <vector>

void multithreaded_tests(std::unique_ptr<ipc::Receiver<int>> int_receiver, std::unique_ptr<ipc::Receiver<std::vector<int>*>> vector_receiver) {
    assert(int_receiver->recv() == 5);
    auto vec = std::unique_ptr<std::vector<int>>(vector_receiver->recv());
    assert(vec->size() == 3);
}

int main() {
    auto t = ipc::channel<int>();

    auto sender = std::move(t.first);
    auto receiver = std::move(t.second);

    sender->send(4);
    assert(receiver->recv() == 4);

    receiver->set_non_blocking();

    int result;
    assert(receiver->try_recv(&result) == false);
    sender->send(3);
    assert(receiver->try_recv(&result));
    assert(result == 3);

    receiver->set_non_blocking(false);

    auto t2 = ipc::channel<std::vector<int>*>();
    auto vec_sender = std::move(t2.first);
    auto vec_receiver = std::move(t2.second);

    auto guard = std::thread(multithreaded_tests, std::move(receiver), std::move(vec_receiver));

    auto vec = std::unique_ptr<std::vector<int>>(new std::vector<int>());
    vec->push_back(1);
    vec->push_back(2);
    vec->push_back(3);

    vec_sender->send(vec.release());
    sender->send(5);

    guard.join();
    std::cout << "All tests succeeded! \\o/" << std::endl;

    return 0;
}
