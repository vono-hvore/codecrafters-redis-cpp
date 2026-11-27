#include "EventLoop.hpp"
#include <cerrno>
#include <format>
#include <poll.h>
#include <stdexcept>
#include <string.h>
#include <sys/socket.h>
#include <vector>

void start_loop(std::map<int, ActionHandler *> handlers);
void poll_error_handler(int);
int socket_error_handler(int);

ActionHandler::ActionHandler(std::function<Event()> task) { _task = task; }

Event ActionHandler::execute() { return _task(); }

void EventLoop::post(ActionHandler *handler) {
  post(socket_error_handler(socket(PF_SYSTEM, SOCK_DGRAM, 0)), handler);
}

void EventLoop::post(int fd, ActionHandler *handler) {
  _handlers.insert(std::make_pair(fd, handler));
}

void EventLoop::start() { start_loop(_handlers); }

void EventLoop::stop() { _handlers.clear(); }

void start_loop(std::map<int, ActionHandler *> handlers) {
  bool continuation = true;
  while (continuation) {
    std::vector<struct pollfd> array(handlers.size());
    for (auto [fd, _] : handlers)
      array.push_back({
          .fd = fd,
          .events = POLLIN,
      });
    poll_error_handler(poll(&array[0], array.size(), -1));
    for (const auto &watch : array) {
      if (watch.revents & POLLIN)
        switch (handlers[watch.fd]->execute()) {
        case Event::Finish:
          continuation = false;
          break;
        case Event::Await:
          break;
        }
    }
  }
}

void poll_error_handler(int error) {
  switch (error) {
  case 0: {
    std::string msg = "poll returns 0 though no timeout requested";
    throw std::runtime_error(msg);
  }
  case -1: {
    std::string msg = std::format("poll error ({},{})", errno, strerror(errno));
    throw std::runtime_error(msg);
  }
  default:
    break;
  };
}

int socket_error_handler(int error) {
  switch (error) {
  case -1: {
    std::string msg
        = std::format("socket error ({},{})", errno, strerror(errno));
    throw std::runtime_error(msg);
  }
  default:
    return error;
  };
}
