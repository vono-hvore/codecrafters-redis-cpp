#pragma once
#include <functional>
#include <map>

enum class Event {
  Await,
  Finish,
};

class ActionHandler {
public:
  ActionHandler(std::function<Event()> task);
  ~ActionHandler();
  Event execute();

private:
  std::function<Event()> _task;
};

class EventLoop {
public:
  void post(ActionHandler *);
  void post(int fd, ActionHandler *);
  void start();
  void stop();

private:
  std::map<int, ActionHandler *> _handlers;
};
