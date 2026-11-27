#include "EventLoop.hpp"
#include <gtest/gtest.h>
#include <sys/socket.h> // This is bad

class EventLoopTest : public ::testing::Test {
protected:
  void SetUp() override {
    event_loop = EventLoop();
    temp_socket = socket(PF_SYSTEM, SOCK_DGRAM, 0);
  }

  void TearDown() override { close(temp_socket); }

  int temp_socket;
  EventLoop event_loop;
};

TEST_F(EventLoopTest, PostHandlerWithoutFd) {
  int callback_count = 0;
  ActionHandler *handler_first = new ActionHandler([&callback_count] {
    callback_count++;
    return Event::Finish;
  });
  ActionHandler *handler_second = new ActionHandler([&callback_count] {
    callback_count++;
    return Event::Finish;
  });
  event_loop.post(handler_first);
  event_loop.post(handler_second);
  event_loop.start();

  ASSERT_EQ(callback_count, 2);
}

TEST_F(EventLoopTest, PostHandlerWithFd) {
  int callback_count = 0;
  ActionHandler *handler_first = new ActionHandler([&callback_count] {
    callback_count++;
    return Event::Finish;
  });
  ActionHandler *handler_second = new ActionHandler([&callback_count] {
    callback_count++;
    return Event::Finish;
  });
  event_loop.post(temp_socket, handler_first);
  event_loop.post(temp_socket, handler_second);
  event_loop.start();

  ASSERT_EQ(callback_count, 1);
}

TEST_F(EventLoopTest, StartExecutesHandlerWhenFdIsReadable) {}

TEST_F(EventLoopTest, StartContinuesWhileHandlerAwaits) {}

TEST_F(EventLoopTest, StartReturnsWhenHandlerFinishes) {}

TEST_F(EventLoopTest, StopRemovesHandlers) {}

TEST_F(EventLoopTest, ThrowsOnError) {}
